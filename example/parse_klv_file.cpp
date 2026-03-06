/**
 * @file parse_klv_file.cpp
 * @brief Example: split a KLV file into packets and print each field
 */

#include "read_file.hpp"

#include <klv/klv.hpp>

#include <print>
#include <variant>

namespace {

namespace kv = klv::misb0601;

auto print_record(klv::record<kv::uas_datalink_ls> const& uas) -> void {
	std::println("{}", kv::uas_datalink_ls::name);
	for (auto const& field : klv::describe(uas)) {
		if (field.value) {
			std::println("  [{}] {}: {} {}", field.tag, field.name, *field.value, field.units);
		}
		else {
			std::println("  [{}] {}: <{}>", field.tag, field.name, klv::to_string(field.value.error()));
		}
	}
}

}  // namespace

auto main(int argc, char* argv[]) -> int {
	if (argc < 2) {
		std::println(stderr, "Usage: {} <klv_file>", argv[0]);
		return 1;
	}

	auto const data = read_file(argv[1]);
	if (!data) {
		std::println(stderr, "Error: Could not open file: {}", argv[1]);
		return 1;
	}

	auto packet_count = 0;
	for (auto const& packet : klv::decode_packets<kv::uas_datalink_ls>(*data)) {
		if (!packet) {
			std::println(stderr, "Error at offset {}: {}", packet.error().offset, packet.error().message);
			continue;
		}

		++packet_count;
		std::println("Packet {}", packet_count);

		if (auto const* uas = std::get_if<klv::record<kv::uas_datalink_ls>>(&*packet)) {
			print_record(*uas);
		}
		else if (auto const* unknown = std::get_if<klv::unknown_packet>(&*packet)) {
			std::println("Unknown key {} ({} value bytes)", klv::to_hex(unknown->key), unknown->value.size());
		}
		std::println("");
	}

	std::println("Total packets: {}", packet_count);
	return 0;
}
