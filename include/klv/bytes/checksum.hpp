/**
 * @file klv/bytes/checksum.hpp
 * @brief Packet checksums: the MISB ST 0601 16-bit sum, CRC-16-CCITT (MISB ST 1002), and CRC-32/MPEG-2 (MISB ST 0806)
 *
 * Each function takes the packet bytes from the first key byte through the length byte of the checksum field.
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

/**
 * @brief CRC-16-CCITT checksum of MISB ST 1002
 *
 * Polynomial 0x1021, initial value 0xFFFF, and 16 zero bits after the data (CRC-16/AUG-CCITT). The function does not
 * add the zero bits. It starts at 0x1D0F, which gives the same result.
 */
[[nodiscard]]
constexpr auto crc16_ccitt(byte_span data) noexcept -> uint16_t {
	auto crc = uint16_t{0x1D0F};

	for (auto const byte : data) {
		crc = static_cast<uint16_t>(crc ^ (std::to_integer<uint16_t>(byte) << 8U));
		for (auto bit = 0; bit < 8; ++bit) {
			auto const high = (crc & 0x8000U) != 0;
			crc = static_cast<uint16_t>(crc << 1U);
			if (high) {
				crc = static_cast<uint16_t>(crc ^ 0x1021U);
			}
		}
	}

	return crc;
}

/**
 * @brief CRC-32/MPEG-2 checksum of MISB ST 0806
 *
 * Polynomial 0x04C11DB7, initial value 0xFFFFFFFF, no bit reflection, and no final XOR.
 */
[[nodiscard]]
constexpr auto crc32_mpeg2(byte_span data) noexcept -> uint32_t {
	auto crc = uint32_t{0xFFFF'FFFF};

	for (auto const byte : data) {
		crc ^= std::to_integer<uint32_t>(byte) << 24U;
		for (auto bit = 0; bit < 8; ++bit) {
			auto const high = (crc & 0x8000'0000U) != 0;
			crc <<= 1U;
			if (high) {
				crc ^= 0x04C1'1DB7U;
			}
		}
	}

	return crc;
}

}  // namespace klv
