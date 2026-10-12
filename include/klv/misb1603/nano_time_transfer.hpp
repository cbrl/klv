/**
 * @file klv/misb1603/nano_time_transfer.hpp
 * @brief MISB ST 1603 Nano Time Transfer Pack
 */

#pragma once

#include "klv/bytes.hpp"
#include "klv/codec.hpp"
#include "klv/misb1603/time_transfer_ls.hpp"
#include "klv/set.hpp"

#include <cstdint>
#include <expected>
#include <format>
#include <string>
#include <string_view>
#include <utility>

namespace klv::misb1603 {

/// A Nano Precision Time Stamp and the Time Transfer Local Set that applies to it
struct nano_time_transfer {
	nano_timestamp time{};
	record<time_transfer_ls> transfer;  ///< Empty when the pack holds only the time stamp

	friend auto operator==(nano_time_transfer const&, nano_time_transfer const&) -> bool = default;
};

/**
 * @brief Nano Time Transfer Pack: an 8-byte Nano Precision Time Stamp, then the value of a Time Transfer Local Set
 *
 * The pack is a truncation pack, so the set is optional. A pack of 8 bytes decodes with an empty set. Encoding an
 * empty set writes only the time stamp.
 */
struct nano_time_transfer_codec {
	using value_type = nano_time_transfer;

	static constexpr auto time_length = sizeof(uint64_t);

	[[nodiscard]]
	auto decode(byte_span in) const -> field<nano_time_transfer> {
		if (in.size() < time_length) {
			return std::unexpected(field_error::bad_length);
		}
		auto const time = codec::nano_timestamp{}.decode(in.first(time_length));
		if (!time) {
			return std::unexpected(time.error());
		}
		auto transfer = codec::nested<time_transfer_ls>{}.decode(in.subspan(time_length));
		if (!transfer) {
			return std::unexpected(transfer.error());
		}
		return nano_time_transfer{.time = *time, .transfer = std::move(*transfer)};
	}

	[[nodiscard]]
	auto encode(nano_time_transfer const& value, byte_writer& out) const -> encode_result {
		if (auto const time = codec::nano_timestamp{}.encode(value.time, out); !time) {
			return time;
		}
		return codec::nested<time_transfer_ls>{}.encode(value.transfer, out);
	}

	[[nodiscard]]
	auto to_string(nano_time_transfer const& value) const -> std::string {
		return std::format(
		    "{{time: {}, transfer: {{{}}}}}",
		    codec::nano_timestamp{}.to_string(value.time),
		    codec::nested<time_transfer_ls>{}.to_string(value.transfer)
		);
	}
};

/// Standalone Nano Time Transfer Pack
struct nano_time_transfer_item {
	static constexpr universal_key key = ul("06 0E 2B 34 02 05 01 01 0E 01 03 02 09 00 00 00");
	static constexpr std::string_view name = "Nano Time Transfer Pack";
	static constexpr nano_time_transfer_codec format{};
};

}  // namespace klv::misb1603
