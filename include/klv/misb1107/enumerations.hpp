/**
 * @file klv/misb1107/enumerations.hpp
 * @brief MISB ST 1107 enumerations
 */

#pragma once

#include <cstdint>
#include <string_view>

namespace klv::misb1107 {

/// Method that gives the slant range (tag 38, MISB ST 0801 section 6.6.1)
enum class range_pedigree : uint8_t {
	other = 0,
	measured = 1,
	computed = 2,
};

[[nodiscard]]
constexpr auto to_string(range_pedigree value) -> std::string_view {
	using enum range_pedigree;
	switch (value) {
		case other:
			return "Other";
		case measured:
			return "Measured";
		case computed:
			return "Computed";
	}
	return {};
}

}  // namespace klv::misb1107
