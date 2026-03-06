/**
 * @file create_klv_data.cpp
 * @brief Example: build a MISB ST 0601 record, write it to a file, and decode it again
 */

#include <klv/klv.hpp>

#include <chrono>
#include <fstream>
#include <print>
#include <string>

namespace {

namespace kv = klv::misb0601;
namespace sec = klv::misb0102;

auto make_record() -> klv::record<kv::uas_datalink_ls> {
	auto security = klv::record<sec::security_ls>{};
	security.set<sec::security_classification>(sec::classification::unclassified);
	security.set<sec::classifying_country_coding_method>(sec::classifying_country_coding::iso_3166_two_letter);
	security.set<sec::classifying_country>("//US");

	auto uas = klv::record<kv::uas_datalink_ls>{};
	uas.set<kv::precision_time_stamp>(
	    std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::system_clock::now())
	);
	uas.set<kv::mission_id>("Demo Mission");
	uas.set<kv::platform_heading_angle>(90.0);
	uas.set<kv::platform_pitch_angle>(2.5);
	uas.set_error_indicator<kv::platform_roll_angle>();  // roll unknown
	uas.set<kv::sensor_latitude>(40.7128);
	uas.set<kv::sensor_longitude>(-74.0060);
	uas.set<kv::security_local_set>(security);
	uas.set<kv::uas_datalink_ls_version_number>(17);
	return uas;
}

}  // namespace

auto main() -> int {
	auto const uas = make_record();

	// The encoder writes the fields in definition order and adds the checksum as the last field
	auto const bytes = klv::encode_packet(uas);
	if (!bytes) {
		std::println(stderr, "Encode failed: {}", bytes.error().message);
		return 1;
	}
	std::println("Encoded {} bytes: {}", bytes->size(), klv::to_hex(*bytes));

	auto const filename = "example_output.klv";
	auto file = std::ofstream(filename, std::ios::binary);
	file.write(reinterpret_cast<char const*>(bytes->data()), static_cast<std::streamsize>(bytes->size()));
	std::println("Wrote {}", filename);

	auto const decoded = klv::decode_packet<kv::uas_datalink_ls>(*bytes);
	if (!decoded) {
		std::println(stderr, "Decode failed: {}", decoded.error().message);
		return 1;
	}

	for (auto const& field : klv::describe(*decoded)) {
		if (field.value) {
			std::println("{}: {} {}", field.name, *field.value, field.units);
		}
		else {
			std::println("{}: <{}>", field.name, klv::to_string(field.value.error()));
		}
	}
	return 0;
}
