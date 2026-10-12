/**
 * @file klv/misb1204/check_value.hpp
 * @brief MISB ST 1204 check value of the text format (ST 1204 Appendix B)
 */

#pragma once

#include "klv/bytes.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>

namespace klv::misb1204 {

namespace detail {

/// pMap([a, b, c, d]) = [a ^ b, c, d, a]. Bit a is the most significant bit. (ST 1204 Appendix B.1)
[[nodiscard]]
constexpr auto p_map(unsigned value) -> unsigned {
	auto const a = (value >> 3U) & 1U;
	auto const b = (value >> 2U) & 1U;
	auto const c = (value >> 1U) & 1U;
	auto const d = value & 1U;
	return ((a ^ b) << 3U) | (c << 2U) | (d << 1U) | a;
}

/// qMap([a, b, c, d]) = [d, a ^ d, b, c] (ST 1204 Appendix B.1)
[[nodiscard]]
constexpr auto q_map(unsigned value) -> unsigned {
	auto const a = (value >> 3U) & 1U;
	auto const b = (value >> 2U) & 1U;
	auto const c = (value >> 1U) & 1U;
	auto const d = value & 1U;
	return (d << 3U) | ((a ^ d) << 2U) | (b << 1U) | c;
}

using permutation_table = std::array<std::array<uint8_t, 16>, 15>;

/// Row 0 holds the hex values 0 to 15. Each next row applies `map` to the row before it.
template<typename Map>
[[nodiscard]]
consteval auto make_table(Map map) -> permutation_table {
	auto table = permutation_table{};
	for (auto j = size_t{0}; j < 16; ++j) {
		auto value = static_cast<unsigned>(j);
		for (auto i = size_t{0}; i < table.size(); ++i) {
			table[i][j] = static_cast<uint8_t>(value);
			value = map(value);
		}
	}
	return table;
}

inline constexpr auto p_table = make_table(p_map);
inline constexpr auto q_table = make_table(q_map);

}  // namespace detail

/**
 * @brief Check value of the hex digits in `text` (ST 1204 Appendix B)
 *
 * Characters that are not hex digits (for example the separators ':', '-', and '/') do not count.
 */
[[nodiscard]]
constexpr auto check_value(std::string_view text) -> uint8_t {
	auto p_check = 0U;
	auto q_check = 0U;
	auto index = size_t{0};
	for (auto const c : text) {
		auto const digit = klv::detail::hex_digit(c);
		if (digit < 0) {
			continue;
		}
		++index;
		p_check ^= detail::p_table[index % 15][static_cast<size_t>(digit)];
		q_check ^= detail::q_table[index % 15][static_cast<size_t>(digit)];
	}
	return static_cast<uint8_t>((p_check << 4U) | q_check);
}

}  // namespace klv::misb1204
