/**
 * @file klv/codecs/text.hpp
 * @brief Display helpers for codecs
 */

#pragma once

#include <format>
#include <ranges>
#include <string>
#include <utility>

namespace klv::detail {

/// Display text of a list, for example "[1, 2, 3]". `text` gives the display text of one element.
template<std::ranges::input_range R, typename F>
[[nodiscard]]
auto list_text(R&& values, F&& text) -> std::string {
	auto result = std::string{"["};
	auto first = true;
	for (auto&& value : values) {
		if (!first) {
			result += ", ";
		}
		result += text(value);
		first = false;
	}
	return result + "]";
}

/// Display text of a list. `std::format` gives the display text of each element.
template<std::ranges::input_range R>
[[nodiscard]]
auto list_text(R&& values) -> std::string {
	return list_text(std::forward<R>(values), [](auto const& value) { return std::format("{}", value); });
}

}  // namespace klv::detail
