/**
 * @file klv/misb1507/region_ls.hpp
 * @brief MISB ST 1507 Region Local Set: the Region List of a Sensor Timing Local Set
 */

#pragma once

#include "klv/bytes.hpp"
#include "klv/codec.hpp"
#include "klv/misb1507/region.hpp"
#include "klv/set.hpp"

#include <cstdint>
#include <string_view>
#include <utility>
#include <vector>

namespace klv::misb1507 {

// Region Local Set items. Each tag is one style of Exposure Pattern (ST 1507 Table 5). The set holds one item for each
// Region, so it can hold a tag more than once.

/// All detectors of the Region have the same exposure time (Tuple 0 and Tuple 3)
struct global_shutter {
	static constexpr uint32_t tag = 1;
	static constexpr std::string_view name = "Global Shutter Region";
	static constexpr codec::repeated<region_codec> format{};
};

/// Each detector of a Sub-Region has its own Exposure Start Time (Tuple 0, Tuple 1, and Tuple 3)
struct detector_delay {
	static constexpr uint32_t tag = 2;
	static constexpr std::string_view name = "Detector Delay Region";
	static constexpr codec::repeated<region_codec> format{.item = {.sub_region_end = true}};
};

/// Each Sub-Region starts after a delay (Tuple 0, Tuple 2, and Tuple 3)
struct sub_region_delay {
	static constexpr uint32_t tag = 3;
	static constexpr std::string_view name = "Sub-Region Delay Region";
	static constexpr codec::repeated<region_codec> format{.item = {.second_sub_region = true}};
};

/// Each detector of a Sub-Region has its own Exposure Start Time, and each Sub-Region starts after a delay (all
/// tuples)
struct detector_and_sub_region_delay {
	static constexpr uint32_t tag = 4;
	static constexpr std::string_view name = "Detector and Sub-Region Delay Region";
	static constexpr codec::repeated<region_codec> format{.item = {.sub_region_end = true, .second_sub_region = true}};
};

/// MISB ST 1507 Region Local Set: the Region List of a Sensor Timing Local Set
struct region_ls {
	static constexpr std::string_view name = "Region Local Set";
	static constexpr universal_key key = ul("06 0E 2B 34 02 0B 01 01 0E 01 03 02 01 02 00 00");

	using fields = field_list<global_shutter, detector_delay, sub_region_delay, detector_and_sub_region_delay>;
};

/// Add `value` to the Region List. The tuples of `value` select the tag.
inline auto add_region(record<region_ls>& list, region value) -> void {
	if (value.sub_region_end && value.second_sub_region) {
		list.add<detector_and_sub_region_delay>(std::move(value));
	}
	else if (value.sub_region_end) {
		list.add<detector_delay>(std::move(value));
	}
	else if (value.second_sub_region) {
		list.add<sub_region_delay>(std::move(value));
	}
	else {
		list.add<global_shutter>(std::move(value));
	}
}

/// The regions of the Region List, in tag order. An item that did not decode adds no region.
[[nodiscard]]
inline auto regions(record<region_ls> const& list) -> std::vector<region> {
	auto all = std::vector<region>{};
	auto const append = [&](field<std::vector<region>> const& items) {
		if (items) {
			all.insert(all.end(), items->begin(), items->end());
		}
	};
	append(list.get<global_shutter>());
	append(list.get<detector_delay>());
	append(list.get<sub_region_delay>());
	append(list.get<detector_and_sub_region_delay>());
	return all;
}

}  // namespace klv::misb1507
