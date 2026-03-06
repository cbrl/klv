/**
 * @file klv/bytes/checksum.hpp
 * @brief MISB ST 0601 16-bit packet checksum
 */

#pragma once

#include "klv/bytes/types.hpp"

#include <cstddef>
#include <cstdint>

namespace klv {

/**
 * @brief MISB ST 0601 16-bit checksum
 *
 * A running 16-bit sum of big-endian byte pairs. An odd final byte counts as a high byte.
 *
 * @param data Packet bytes from the first byte of the key through the length byte of the checksum field
 */
[[nodiscard]]
constexpr auto packet_checksum(byte_span data) noexcept -> uint16_t {
	auto sum = uint32_t{0};

	for (auto i = size_t{0}; i < data.size(); ++i) {
		auto const value = std::to_integer<uint32_t>(data[i]);
		sum += (i % 2 == 0) ? (value << 8U) : value;
	}

	return static_cast<uint16_t>(sum & 0xFFFFU);
}

}  // namespace klv
