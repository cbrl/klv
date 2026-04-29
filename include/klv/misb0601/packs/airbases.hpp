/**
 * @file klv/misb0601/packs/airbases.hpp
 * @brief MISB ST 0601 Airbase Locations (tag 130)
 */

#pragma once

#include "klv/bytes.hpp"
#include "klv/codec.hpp"
#include "klv/misb0601/packs/location.hpp"
#include "klv/misb0601/packs/detail.hpp"

#include <expected>
#include <format>
#include <optional>
#include <string>
#include <utility>

namespace klv::misb0601 {

/// Take-off and recovery locations. An unknown location has no value.
struct airbases {
	std::optional<location> take_off;
	std::optional<location> recovery;

	friend auto operator==(airbases const&, airbases const&) -> bool = default;
};

/**
 * @brief Airbase Locations: two length-prefixed location packs (take-off, then recovery)
 *
 * A zero length marks an unknown location. When the recovery element is omitted, the recovery location is the
 * take-off location. Encoding omits the recovery element when the two locations are equal.
 */
struct airbases_codec {
	using value_type = airbases;

	[[nodiscard]]
	auto decode(byte_span in) const -> field<airbases> {
		auto reader = byte_reader{in};
		auto result = airbases{};

		auto const take_off = read_location(reader);
		if (!take_off) {
			return std::unexpected(take_off.error());
		}
		result.take_off = *take_off;

		if (reader.empty()) {
			result.recovery = result.take_off;
			return result;
		}

		auto const recovery = read_location(reader);
		if (!recovery) {
			return std::unexpected(recovery.error());
		}
		result.recovery = *recovery;
		return detail::finish(reader, std::move(result));
	}

	[[nodiscard]]
	auto encode(airbases const& value, byte_writer& out) const -> encode_result {
		return put_location(value.take_off, out).and_then([&] {
			return value.recovery == value.take_off ? encode_result{} : put_location(value.recovery, out);
		});
	}

	[[nodiscard]]
	auto to_string(airbases const& value) const -> std::string {
		auto const text = [](std::optional<location> const& place) {
			return place ? location_codec{}.to_string(*place) : std::string{"unknown"};
		};
		return std::format("{{take-off: {}, recovery: {}}}", text(value.take_off), text(value.recovery));
	}

private:
	[[nodiscard]]
	static auto read_location(byte_reader& reader) -> field<std::optional<location>> {
		auto const bytes = detail::read_prefixed(reader);
		if (!bytes) {
			return std::unexpected(bytes.error());
		}
		if (bytes->empty()) {
			return std::optional<location>{};
		}
		return location_codec{}.decode(*bytes).transform([](location place) { return std::optional{place}; });
	}

	[[nodiscard]]
	static auto put_location(std::optional<location> const& place, byte_writer& out) -> encode_result {
		if (!place) {
			out.put_ber_length(0);
			return {};
		}
		return out.put_length_prefixed([&] { return location_codec{}.encode(*place, out); });
	}
};

}  // namespace klv::misb0601
