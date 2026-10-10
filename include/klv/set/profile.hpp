/**
 * @file klv/set/profile.hpp
 * @brief Support for the checks of profile standards
 *
 * A profile standard, such as MISB ST 0902 or ST 1403, lists the items that a set must report. A requirement can
 * accept one of several items, such as tag 6 or tag 90 of MISB ST 0601. The check of a profile gives one `finding`
 * for each requirement that a record does not meet.
 */

#pragma once

#include "klv/field.hpp"
#include "klv/set/definition.hpp"
#include "klv/set/record.hpp"

#include <cstdint>
#include <string_view>

namespace klv {

/**
 * @brief True when `rec` reports the field `E`, or the field `Inner...` in the nested set of `E`
 *
 * A field with a value or with the error indicator of its format is a report. An empty field (length of zero) is not a
 * report. A field that did not decode is not a report.
 * @code
 * klv::reports<misb0601::mission_id>(uas);
 * klv::reports<misb0601::security_local_set, misb0102::security_classification>(uas);
 * @endcode
 */
template<typename E, typename... Inner, set_definition S>
requires member_of<E, S> && (sizeof...(Inner) == 0 || detail::is_record<value_t<E>>::value)
[[nodiscard]]
auto reports(record<S> const& rec) noexcept -> bool {
	auto const& value = rec.template get<E>();
	if constexpr (sizeof...(Inner) == 0) {
		return value || value.error() == field_error::error_indicator;
	}
	else {
		return value && reports<Inner...>(*value);
	}
}

/// Why a record does not meet one requirement of a profile
enum class shortfall : uint8_t {
	missing,  ///< The record reports no item of the requirement
	conflict,  ///< The record reports two items of the requirement that exclude each other
};

[[nodiscard]]
constexpr auto to_string(shortfall value) noexcept -> std::string_view {
	switch (value) {
		case shortfall::missing:
			return "missing";
		case shortfall::conflict:
			return "conflict";
	}
	return {};
}

/// One requirement of a profile that a record does not meet
template<typename Item>
struct finding {
	Item item{};
	shortfall reason = shortfall::missing;

	friend auto operator==(finding const&, finding const&) -> bool = default;
};

}  // namespace klv
