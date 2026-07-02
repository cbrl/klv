/**
 * @file klv/misb0806/enumerations.hpp
 * @brief MISB ST 0806 enumerations
 */

#pragma once

#include <cstdint>
#include <string_view>

namespace klv::misb0806 {

/// Kind of a point or area of interest (POI LS tag 5, AOI LS tag 6)
enum class interest_type : uint8_t {
	friendly = 1,
	hostile = 2,
	target = 3,
	unknown = 4,
};

[[nodiscard]]
constexpr auto to_string(interest_type value) -> std::string_view {
	using enum interest_type;
	switch (value) {
		case friendly:
			return "Friendly";
		case hostile:
			return "Hostile";
		case target:
			return "Target";
		case unknown:
			return "Unknown";
	}
	return {};
}

/// Data type of a user-defined data item (User Defined LS tag 1)
enum class user_data_type : uint8_t {
	string = 0,
	signed_integer = 1,
	unsigned_integer = 2,
	experimental = 3,
};

[[nodiscard]]
constexpr auto to_string(user_data_type value) -> std::string_view {
	using enum user_data_type;
	switch (value) {
		case string:
			return "String";
		case signed_integer:
			return "Signed Integer";
		case unsigned_integer:
			return "Unsigned Integer";
		case experimental:
			return "Experimental";
	}
	return {};
}

}  // namespace klv::misb0806
