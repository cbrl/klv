/**
 * @file print_metadata.cpp
 * @brief Example: decode MISB ST 0601 packets and print selected typed values
 */

#include "read_file.hpp"

#include <klv/klv.hpp>

#include <print>
#include <variant>

auto main(int argc, char* argv[]) -> int {
	namespace kv = klv::misb0601;

	if (argc < 2) {
		std::println(stderr, "Usage: {} <klv_file>", argv[0]);
		return 1;
	}

	auto const data = read_file(argv[1]);
	if (!data) {
		std::println(stderr, "Error: Could not open file: {}", argv[1]);
		return 1;
	}

	for (auto const& packet : klv::decode_packets<kv::uas_datalink_ls>(*data)) {
		if (!packet) {
			std::println(stderr, "Parse error: {}", packet.error().message);
			continue;
		}

		auto const* uas = std::get_if<klv::record<kv::uas_datalink_ls>>(&*packet);
		if (uas == nullptr) {
			continue;
		}

		if (auto const& time = uas->get<kv::precision_time_stamp>()) {
			std::println("Timestamp: {:%F %T} UTC", *time);
		}
		if (auto const& latitude = uas->get<kv::sensor_latitude>()) {
			std::println("Sensor Latitude: {:.6f} degrees", *latitude);
		}
		if (auto const& longitude = uas->get<kv::sensor_longitude>()) {
			std::println("Sensor Longitude: {:.6f} degrees", *longitude);
		}
		if (auto const& heading = uas->get<kv::platform_heading_angle>()) {
			std::println("Platform Heading: {:.2f} degrees", *heading);
		}

		// A field that holds the error value is not the same as a missing field
		if (auto const& roll = uas->get<kv::platform_roll_angle>()) {
			std::println("Platform Roll: {:.2f} degrees", *roll);
		}
		else {
			std::println("Platform Roll: {}", klv::to_string(roll.error()));
		}

		std::println("");
	}

	return 0;
}
