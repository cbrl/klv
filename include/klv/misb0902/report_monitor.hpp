/**
 * @file klv/misb0902/report_monitor.hpp
 * @brief MISB ST 0902 check of the 30-second rule over a stream (requirement ST 0902.8-05)
 */

#pragma once

#include "klv/bytes/types.hpp"
#include "klv/misb0601/uas_datalink_ls.hpp"
#include "klv/misb0902/check.hpp"
#include "klv/misb0902/items.hpp"
#include "klv/set/record.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <cstddef>
#include <optional>
#include <vector>

namespace klv::misb0902 {

/**
 * @brief Checks requirement ST 0902.8-05 over a stream: a report of each item at least once every 30 seconds
 *
 * Add the records in stream order. The Precision Time Stamp (tag 2) of a record gives its time. The interval of each
 * requirement starts at the first record. The monitor keeps the latest time, so an earlier time stamp does not move
 * the time back.
 * @code
 * auto monitor = klv::misb0902::report_monitor{};
 * for (auto const& uas : records) {
 *     monitor.add(uas);
 *     for (auto const late : monitor.overdue()) {
 *         // to_string(late)
 *     }
 * }
 * @endcode
 */
class report_monitor {
public:
	/// Longest interval that ST 0902 allows between two reports of an item
	static constexpr auto max_interval = std::chrono::seconds{30};

	/// Note the requirements that `rec` meets. A record without a Precision Time Stamp changes nothing and gives false.
	auto add(record<misb0601::uas_datalink_ls> const& rec) -> bool {
		auto const& time = rec.get<misb0601::precision_time_stamp>();
		if (!time) {
			return false;
		}
		if (!now_) {
			last_.fill(*time);
		}
		now_ = std::max(now_.value_or(*time), *time);
		for (auto index = size_t{0}; index < item_count; ++index) {
			if (reports(rec, static_cast<item>(index))) {
				last_[index] = std::max(last_[index], *time);
			}
		}
		return true;
	}

	/// The requirements with no report in the 30 seconds before the latest time stamp, in table order
	[[nodiscard]]
	auto overdue() const -> std::vector<item> {
		auto late = std::vector<item>{};
		if (!now_) {
			return late;
		}
		for (auto index = size_t{0}; index < item_count; ++index) {
			if (*now_ - last_[index] > max_interval) {
				late.push_back(static_cast<item>(index));
			}
		}
		return late;
	}

private:
	std::array<timestamp, item_count> last_{};
	std::optional<timestamp> now_;
};

}  // namespace klv::misb0902
