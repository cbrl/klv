/**
 * @file klv/bytes/byte_writer.hpp
 * @brief Writer for big-endian integers, BER lengths, BER-OID tags, and length-prefixed values
 */

#pragma once

#include "klv/bytes/big_endian.hpp"
#include "klv/bytes/types.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <utility>

namespace klv {

/**
 * @brief Appends KLV wire data to a byte vector
 */
class byte_writer {
public:
	explicit byte_writer(byte_vector& out) noexcept : out_(&out) {
	}

	/// Number of bytes in the output buffer
	[[nodiscard]]
	auto size() const noexcept -> size_t {
		return out_->size();
	}

	/// The output buffer
	[[nodiscard]]
	auto buffer() const noexcept -> byte_vector& {
		return *out_;
	}

	/// Remove all bytes after `size`. Use this to discard a partial write.
	auto truncate(size_t size) -> void {
		out_->resize(size);
	}

	auto put_u8(uint8_t value) -> void {
		out_->push_back(static_cast<std::byte>(value));
	}

	auto put_bytes(byte_span bytes) -> void {
		out_->insert(out_->end(), bytes.begin(), bytes.end());
	}

	/// Write the low `count` bytes of `value`, most significant first
	template<wire_integer T>
	auto put_big_endian(T value, size_t count = sizeof(T)) -> void {
		using U = std::make_unsigned_t<T>;
		auto const bits = static_cast<U>(value);

		for (auto i = count; i > 0; --i) {
			auto const shift = (i - 1) * 8;
			auto const byte = shift < sizeof(U) * 8 ? static_cast<uint8_t>((bits >> shift) & 0xFFU) : 0;
			put_u8(static_cast<uint8_t>(byte));
		}
	}

	/// Write a BER length in the shortest form
	auto put_ber_length(size_t length) -> void {
		if (length < 0x80) {
			put_u8(static_cast<uint8_t>(length));
			return;
		}

		auto const width = byte_width(length);
		put_u8(static_cast<uint8_t>(0x80U | width));
		put_big_endian(static_cast<uint64_t>(length), width);
	}

	/// Write a BER-OID integer (local set tag)
	auto put_ber_oid(uint32_t value) -> void {
		auto groups = std::array<uint8_t, 5>{};
		auto count = size_t{0};

		do {
			groups[count++] = static_cast<uint8_t>(value & 0x7FU);
			value >>= 7U;
		} while (value != 0);

		for (auto i = count; i > 0; --i) {
			auto const more = i > 1 ? 0x80U : 0x00U;
			put_u8(static_cast<uint8_t>(groups[i - 1] | more));
		}
	}

	/**
	 * @brief Write a BER length followed by the value that `write_value` writes
	 *
	 * The function first reserves one length byte, which is enough for values shorter than 128 bytes. A longer value
	 * moves the value bytes once to make room for the long form. On failure, the output returns to its old size.
	 *
	 * @param write_value Callable that writes the value and returns a `std::expected<void, E>`
	 */
	template<typename F>
	auto put_length_prefixed(F&& write_value) -> std::invoke_result_t<F> {
		auto const length_position = out_->size();
		out_->push_back(std::byte{0});

		auto status = std::forward<F>(write_value)();
		if (!status) {
			out_->resize(length_position);
			return status;
		}

		auto const length = out_->size() - length_position - 1;
		if (length < 0x80) {
			(*out_)[length_position] = static_cast<std::byte>(length);
			return status;
		}

		auto const width = byte_width(length);
		(*out_)[length_position] = static_cast<std::byte>(0x80U | width);

		auto length_bytes = std::array<std::byte, sizeof(uint64_t)>{};
		for (auto i = size_t{0}; i < width; ++i) {
			length_bytes[i] =
			    static_cast<std::byte>((static_cast<uint64_t>(length) >> ((width - 1 - i) * 8)) & 0xFFU);
		}

		auto const insert_at = out_->begin() + static_cast<std::ptrdiff_t>(length_position + 1);
		out_->insert(insert_at, length_bytes.begin(), length_bytes.begin() + static_cast<std::ptrdiff_t>(width));
		return status;
	}

private:
	/// Number of bytes needed for `value`, at least 1
	[[nodiscard]]
	static auto byte_width(size_t value) noexcept -> size_t {
		auto width = size_t{1};
		while (width < sizeof(size_t) && (value >> (width * 8)) != 0) {
			++width;
		}
		return width;
	}

	byte_vector* out_;
};

}  // namespace klv
