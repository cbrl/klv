/**
 * @file klv/codecs/timestamp.hpp
 * @brief MISB precision time stamp codec
 */

#pragma once

#include "klv/bytes/big_endian.hpp"
#include "klv/bytes/byte_writer.hpp"
#include "klv/bytes/types.hpp"
#include "klv/field.hpp"

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <format>
#include <limits>
#include <string>

namespace klv::codec {

/**
 * @brief MISB precision time stamp: 64-bit unsigned count of microseconds since the POSIX epoch
 *
 * Decoding accepts `min_length` to 8 bytes. Encoding always writes 8 bytes.
 *
 * @code
 * static constexpr codec::timestamp format{};                  // Exactly 8 bytes
 * static constexpr codec::timestamp format{.min_length = 1};  // 1 to 8 bytes
 * @endcode
 */
struct timestamp {
	using value_type = klv::timestamp;

	size_t min_length = sizeof(uint64_t);

	[[nodiscard]]
	constexpr auto decode(byte_span in) const -> field<klv::timestamp> {
		if (in.size() < min_length || in.empty() || in.size() > sizeof(uint64_t)) {
			return std::unexpected(field_error::bad_length);
		}

		auto const raw = read_big_endian<uint64_t>(in);
		if (raw > static_cast<uint64_t>(std::numeric_limits<int64_t>::max())) {
			return std::unexpected(field_error::out_of_range);
		}
		return klv::timestamp{std::chrono::microseconds{static_cast<int64_t>(raw)}};
	}

	[[nodiscard]]
	auto encode(klv::timestamp value, byte_writer& out) const -> encode_result {
		auto const count = value.time_since_epoch().count();
		if (count < 0) {
			return std::unexpected(field_error::out_of_range);
		}
		out.put_big_endian(static_cast<uint64_t>(count));
		return {};
	}

	[[nodiscard]]
	auto to_string(klv::timestamp value) const -> std::string {
		return std::format("{:%FT%T}Z", value);
	}
};

}  // namespace klv::codec
