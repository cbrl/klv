/**
 * @file klv/bytes/byte_reader.hpp
 * @brief Bounds-checked reader for big-endian integers, universal keys, BER lengths, and BER-OID tags
 */

#pragma once

#include "klv/bytes/big_endian.hpp"
#include "klv/bytes/types.hpp"
#include "klv/bytes/universal_key.hpp"
#include "klv/error.hpp"

#include <algorithm>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <format>
#include <limits>
#include <tuple>

namespace klv {

/// Unsigned integer type that can hold a BER-OID value. `bool` and the character types are not numbers.
template<typename T>
concept ber_oid_integer = std::unsigned_integral<T> && !std::same_as<T, bool> && !std::same_as<T, char>
                       && !std::same_as<T, wchar_t> && !std::same_as<T, char8_t> && !std::same_as<T, char16_t>
                       && !std::same_as<T, char32_t>;

/**
 * @brief Bounds-checked cursor over a byte span
 *
 * Every read checks the request against `remaining()` before it moves, so no size calculation can overflow. The
 * reported offsets include a base offset, so that errors from a nested buffer point into the outer buffer.
 */
class byte_reader {
public:
	constexpr explicit byte_reader(byte_span data, size_t base_offset = 0) noexcept :
	    data_(data),
	    base_offset_(base_offset) {
	}

	/// Number of bytes not read yet
	[[nodiscard]]
	constexpr auto remaining() const noexcept -> size_t {
		return data_.size() - position_;
	}

	/// True when all bytes are read
	[[nodiscard]]
	constexpr auto empty() const noexcept -> bool {
		return remaining() == 0;
	}

	/// Offset of the next byte, including the base offset
	[[nodiscard]]
	constexpr auto offset() const noexcept -> size_t {
		return base_offset_ + position_;
	}

	/// Position of the next byte in the reader's own span
	[[nodiscard]]
	constexpr auto position() const noexcept -> size_t {
		return position_;
	}

	/// Bytes not read yet
	[[nodiscard]]
	constexpr auto rest() const noexcept -> byte_span {
		return data_.subspan(position_);
	}

	/// Read `count` bytes
	[[nodiscard]]
	auto take(size_t count) -> result<byte_span> {
		if (count > remaining()) {
			return detail::make_error(
			    error_code::unexpected_end_of_data,
			    offset(),
			    std::format("Need {} bytes at offset {}, but only {} remain", count, offset(), remaining())
			);
		}

		auto const bytes = data_.subspan(position_, count);
		position_ += count;
		return bytes;
	}

	/// Read one byte
	[[nodiscard]]
	auto read_u8() -> result<uint8_t> {
		return take(1).transform([](byte_span bytes) { return std::to_integer<uint8_t>(bytes.front()); });
	}

	/// Read a 16-byte universal key
	[[nodiscard]]
	auto read_universal_key() -> result<universal_key> {
		return take(std::tuple_size_v<universal_key>).transform([](byte_span bytes) {
			auto key = universal_key{};
			std::ranges::copy(bytes, key.begin());
			return key;
		});
	}

	/**
	 * @brief Read a BER length (short or long form)
	 *
	 * The indefinite form and long forms wider than 8 bytes are errors. The caller still checks the length against
	 * the remaining bytes, with `take`.
	 */
	[[nodiscard]]
	auto read_ber_length() -> result<size_t> {
		auto const start = offset();

		auto const first = read_u8();
		if (!first) {
			return std::unexpected(first.error());
		}

		if (*first < 0x80U) {
			return size_t{*first};
		}

		auto const width = static_cast<size_t>(*first & 0x7FU);
		if (width == 0) {
			return detail::make_error(
			    error_code::invalid_ber_length,
			    start,
			    std::format("BER indefinite length at offset {} is not supported", start)
			);
		}
		if (width > sizeof(uint64_t)) {
			return detail::make_error(
			    error_code::invalid_ber_length,
			    start,
			    std::format("BER length at offset {} has {} length bytes (the maximum is 8)", start, width)
			);
		}

		auto const bytes = take(width);
		if (!bytes) {
			return std::unexpected(bytes.error());
		}

		auto const value = read_big_endian<uint64_t>(*bytes);
		if constexpr (sizeof(size_t) < sizeof(uint64_t)) {
			if (value > std::numeric_limits<size_t>::max()) {
				return detail::make_error(
				    error_code::invalid_ber_length,
				    start,
				    std::format("BER length {} at offset {} is too large", value, start)
				);
			}
		}

		return static_cast<size_t>(value);
	}

	/**
	 * @brief Read a BER-OID integer, for example a local set tag
	 *
	 * Each byte gives 7 bits, most significant group first. Bit 8 is set on every byte except the last. A leading
	 * 0x80 byte and values wider than `T` are errors.
	 */
	template<ber_oid_integer T = uint32_t>
	[[nodiscard]]
	auto read_ber_oid() -> result<T> {
		auto const start = offset();
		auto value = T{0};

		for (auto first = true;; first = false) {
			auto const byte = read_u8();
			if (!byte) {
				return std::unexpected(byte.error());
			}

			if (first && *byte == 0x80U) {
				return detail::make_error(
				    error_code::invalid_ber_oid,
				    start,
				    std::format("BER-OID value at offset {} starts with 0x80", start)
				);
			}
			if (value > (std::numeric_limits<T>::max() >> 7U)) {
				return detail::make_error(
				    error_code::invalid_ber_oid,
				    start,
				    std::format(
				        "BER-OID value at offset {} is wider than {} bits",
				        start,
				        std::numeric_limits<T>::digits
				    )
				);
			}

			value = static_cast<T>((value << 7U) | (*byte & 0x7FU));
			if ((*byte & 0x80U) == 0) {
				return value;
			}
		}
	}

private:
	byte_span data_;
	size_t base_offset_ = 0;
	size_t position_ = 0;
};

}  // namespace klv
