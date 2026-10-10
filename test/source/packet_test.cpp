#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <format>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include "klv_test_util.hpp"
#include "klv/misb0601.hpp"
#include "klv/misb0603.hpp"
#include "klv/misb1204.hpp"
#include "klv/packet.hpp"

using namespace klv;
using Catch::Approx;
using klv_test::append_field;
using klv_test::append_packet;
using klv_test::make_bytes;

namespace kv = klv::misb0601;

using uas_record = record<kv::uas_datalink_ls>;

namespace other {

struct value {
	static constexpr uint32_t tag = 1;
	static constexpr std::string_view name = "Value";
	static constexpr codec::integer<uint8_t> format{};
};

/// A second universal set without a checksum, for dispatch tests
struct other_ls {
	static constexpr std::string_view name = "Other";
	static constexpr universal_key key = ul("06 0E 2B 34 02 0B 01 01 0E 01 03 01 7F 00 00 00");
	using fields = field_list<value>;
};

struct keyed_value {
	static constexpr universal_key key = ul("06 0E 2B 34 01 01 01 01 0E 01 03 7F 01 00 00 00");
	static constexpr std::string_view name = "Keyed Value";
	static constexpr codec::integer<uint16_t> format{};
};

struct keyed_checksum {
	static constexpr universal_key key = ul("06 0E 2B 34 01 01 01 01 0E 01 03 7F 02 00 00 00");
	static constexpr std::string_view name = "Keyed Checksum";
	static constexpr codec::integer<uint16_t> format{};
};

/// A universal set (byte 6 of the key is 01) with a CRC-16-CCITT checksum
struct keyed_us {
	static constexpr std::string_view name = "Keyed";
	static constexpr universal_key key = ul("06 0E 2B 34 02 01 01 01 0E 01 03 7F 00 00 00 00");
	using fields = field_list<keyed_value, keyed_checksum>;
	using checksum_field = keyed_checksum;
	static constexpr auto checksum_function = crc16_ccitt;
};

}  // namespace other

static_assert(universal_set_definition<other::keyed_us> && has_checksum<other::keyed_us>);

namespace {

constexpr auto sample_time = timestamp{std::chrono::microseconds{1'224'807'209'913'000}};

[[nodiscard]]
auto make_sample() -> uas_record {
	auto rec = uas_record{};
	rec.set<kv::precision_time_stamp>(sample_time);
	rec.set<kv::mission_id>("Mission 12");
	rec.set<kv::platform_heading_angle>(159.974);
	rec.set<kv::platform_pitch_angle>(-0.43);
	rec.set<kv::sensor_latitude>(60.176822966978335);
	rec.set<kv::sensor_longitude>(128.42675904204452);
	rec.set<kv::uas_datalink_ls_version_number>(17);
	return rec;
}

[[nodiscard]]
auto encode_sample() -> byte_vector {
	return encode_packet(make_sample()).value();
}

/// Recompute the checksum in the last two bytes of a packet
auto fix_checksum(byte_vector& packet) -> void {
	auto const checksum = packet_checksum(byte_span{packet}.first(packet.size() - 2));
	packet[packet.size() - 2] = static_cast<std::byte>(checksum >> 8U);
	packet[packet.size() - 1] = static_cast<std::byte>(checksum & 0xFFU);
}

}  // namespace

TEST_CASE("KLV packet encodes and decodes a UAS Datalink LS", "[klv]") {
	auto const bytes = encode_sample();

	auto const decoded = decode_packet<kv::uas_datalink_ls>(bytes);
	REQUIRE(decoded);
	CHECK(decoded->get<kv::precision_time_stamp>() == sample_time);
	CHECK(decoded->get<kv::mission_id>() == std::string{"Mission 12"});
	CHECK(decoded->get<kv::platform_heading_angle>().value() == Approx(159.974).margin(360.0 / 65'535.0));
	CHECK(decoded->get<kv::platform_pitch_angle>().value() == Approx(-0.43).margin(40.0 / 65'534.0));
	CHECK(decoded->get<kv::sensor_latitude>().value() == Approx(60.176822966978335).margin(1e-7));
	CHECK(decoded->get<kv::uas_datalink_ls_version_number>() == uint8_t{17});

	// The decoded values encode to the same bytes
	CHECK(encode_packet(*decoded) == bytes);
}

TEST_CASE("KLV packet puts the time stamp first and a valid checksum last", "[klv]") {
	auto const bytes = encode_sample();
	auto const key = kv::uas_datalink_ls::key;

	REQUIRE(bytes.size() > 22);
	CHECK(std::equal(key.begin(), key.end(), bytes.begin()));

	auto reader = byte_reader{bytes};
	REQUIRE(reader.take(16));
	auto const length = reader.read_ber_length();
	REQUIRE(length);
	CHECK(*length == reader.remaining());
	CHECK(reader.read_ber_oid() == kv::precision_time_stamp::tag);

	auto const tail = byte_span{bytes}.last(4);
	CHECK(tail[0] == std::byte{0x01});
	CHECK(tail[1] == std::byte{0x02});

	auto const checksum = packet_checksum(byte_span{bytes}.first(bytes.size() - 2));
	CHECK(read_big_endian<uint16_t>(tail.last(2)) == checksum);

	auto const decoded = decode_packet<kv::uas_datalink_ls>(bytes);
	REQUIRE(decoded);
	CHECK(decoded->get<kv::checksum>() == checksum);
}

TEST_CASE("KLV packet encoder ignores a user checksum value", "[klv]") {
	auto rec = make_sample();
	rec.set<kv::checksum>(0x1234);
	CHECK(encode_packet(rec) == encode_sample());
}

TEST_CASE("KLV packet with a long value uses the long length form", "[klv]") {
	auto rec = make_sample();
	rec.set<kv::mission_id>(std::string(127, 'M'));
	rec.set<kv::platform_tail_number>(std::string(127, 'T'));

	auto const bytes = encode_packet(rec);
	REQUIRE(bytes);
	CHECK((*bytes)[16] == std::byte{0x82});

	auto const decoded = decode_packet<kv::uas_datalink_ls>(*bytes);
	REQUIRE(decoded);
	CHECK(decoded->get<kv::mission_id>() == std::string(127, 'M'));
	CHECK(decoded->get<kv::platform_tail_number>() == std::string(127, 'T'));
}

TEST_CASE("KLV packet checksum verification", "[klv]") {
	SECTION("a changed byte fails verification") {
		auto bytes = encode_sample();
		bytes[30] ^= std::byte{0x01};

		auto const decoded = decode_packet<kv::uas_datalink_ls>(bytes);
		REQUIRE_FALSE(decoded);
		CHECK(decoded.error().code == error_code::checksum_mismatch);

		CHECK(decode_packet<kv::uas_datalink_ls>(bytes, {.verify_checksum = false}));
	}

	SECTION("a packet without a checksum fails verification") {
		auto value = byte_vector{};
		append_field(value, 3, make_bytes({'M'}));

		auto bytes = byte_vector{};
		append_packet(bytes, kv::uas_datalink_ls::key, value);

		auto const decoded = decode_packet<kv::uas_datalink_ls>(bytes);
		REQUIRE_FALSE(decoded);
		CHECK(decoded.error().code == error_code::missing_checksum);

		auto const unchecked = decode_packet<kv::uas_datalink_ls>(bytes, {.verify_checksum = false});
		REQUIRE(unchecked);
		CHECK(unchecked->get<kv::mission_id>() == std::string{"M"});
	}

	SECTION("a checksum that is not the last field fails verification") {
		auto value = byte_vector{};
		append_field(value, 1, make_bytes({0x00, 0x00}));
		append_field(value, 3, make_bytes({'M'}));

		auto bytes = byte_vector{};
		append_packet(bytes, kv::uas_datalink_ls::key, value);

		auto const decoded = decode_packet<kv::uas_datalink_ls>(bytes);
		REQUIRE_FALSE(decoded);
		CHECK(decoded.error().code == error_code::missing_checksum);
	}
}

TEST_CASE("KLV decode_packet checks the key and the packet size", "[klv]") {
	SECTION("the version byte of the key is ignored") {
		auto bytes = encode_sample();
		bytes[7] = std::byte{0x02};
		fix_checksum(bytes);
		CHECK(decode_packet<kv::uas_datalink_ls>(bytes));
	}

	SECTION("another key is an error") {
		auto bytes = encode_sample();
		bytes[12] = std::byte{0x7F};
		fix_checksum(bytes);

		auto const decoded = decode_packet<kv::uas_datalink_ls>(bytes);
		REQUIRE_FALSE(decoded);
		CHECK(decoded.error().code == error_code::unknown_key);
	}

	SECTION("bytes after the packet are an error") {
		auto bytes = encode_sample();
		bytes.push_back(std::byte{0x00});

		auto const decoded = decode_packet<kv::uas_datalink_ls>(bytes);
		REQUIRE_FALSE(decoded);
		CHECK(decoded.error().code == error_code::trailing_data);
	}

	SECTION("a truncated packet is an error") {
		auto bytes = encode_sample();
		bytes.pop_back();

		auto const decoded = decode_packet<kv::uas_datalink_ls>(bytes);
		REQUIRE_FALSE(decoded);
		CHECK(decoded.error().code == error_code::unexpected_end_of_data);
	}
}

TEST_CASE("KLV decode_packets dispatches by key and resynchronizes after errors", "[klv]") {
	using packets = packet<kv::uas_datalink_ls, other::other_ls>;

	auto other_record = record<other::other_ls>{};
	other_record.set<other::value>(5);

	auto buffer = make_bytes({0xDE, 0xAD});  // garbage before the first packet
	auto const first = encode_sample();
	buffer.insert(buffer.end(), first.begin(), first.end());

	// A packet whose length runs past the end of the buffer
	buffer.insert(buffer.end(), kv::uas_datalink_ls::key.begin(), kv::uas_datalink_ls::key.end());
	buffer.insert(buffer.end(), {std::byte{0x82}, std::byte{0xFF}, std::byte{0xFF}, std::byte{0x01}});

	REQUIRE(encode_packet(other_record, buffer));

	auto unknown_key = kv::uas_datalink_ls::key;
	unknown_key[12] = std::byte{0x55};
	append_packet(buffer, unknown_key, make_bytes({0x01, 0x01, 0x09}));

	auto results = std::vector<result<packets>>{};
	for (auto&& item : decode_packets<kv::uas_datalink_ls, other::other_ls>(buffer)) {
		results.push_back(std::move(item));
	}

	REQUIRE(results.size() == 5);

	REQUIRE_FALSE(results[0]);
	CHECK(results[0].error().code == error_code::unknown_key);
	CHECK(results[0].error().offset == 0);

	REQUIRE(results[1]);
	auto const* uas = std::get_if<uas_record>(&*results[1]);
	REQUIRE(uas != nullptr);
	CHECK(uas->get<kv::mission_id>() == std::string{"Mission 12"});

	REQUIRE_FALSE(results[2]);
	CHECK(results[2].error().code == error_code::unexpected_end_of_data);

	REQUIRE(results[3]);
	auto const* other_decoded = std::get_if<record<other::other_ls>>(&*results[3]);
	REQUIRE(other_decoded != nullptr);
	CHECK(*other_decoded == other_record);

	REQUIRE(results[4]);
	auto const* unknown = std::get_if<unknown_packet>(&*results[4]);
	REQUIRE(unknown != nullptr);
	CHECK(unknown->key == unknown_key);
	CHECK(unknown->value == make_bytes({0x01, 0x01, 0x09}));
}

TEST_CASE("KLV decode_packets reports a set decode error and continues", "[klv]") {
	auto bytes = encode_sample();
	bytes[30] ^= std::byte{0x01};  // checksum mismatch
	auto const good = encode_sample();
	bytes.insert(bytes.end(), good.begin(), good.end());

	auto ok = 0;
	auto errors = 0;
	for (auto&& item : decode_packets<kv::uas_datalink_ls>(bytes)) {
		if (item) {
			++ok;
		}
		else {
			++errors;
			CHECK(item.error().code == error_code::checksum_mismatch);
		}
	}

	CHECK(ok == 1);
	CHECK(errors == 1);
}

namespace {

/// Universal set packet with a Keyed Value of 0x1234 and a CRC-16-CCITT checksum
[[nodiscard]]
auto keyed_packet(universal_key checksum_key = other::keyed_checksum::key) -> byte_vector {
	auto value = byte_vector{};
	append_packet(value, other::keyed_value::key, make_bytes({0x12, 0x34}));
	append_packet(value, checksum_key, make_bytes({0x00, 0x00}));

	auto bytes = byte_vector{};
	append_packet(bytes, other::keyed_us::key, value);
	auto const crc = crc16_ccitt(byte_span{bytes}.first(bytes.size() - 2));
	bytes[bytes.size() - 2] = static_cast<std::byte>(crc >> 8U);
	bytes[bytes.size() - 1] = static_cast<std::byte>(crc & 0xFFU);
	return bytes;
}

}  // namespace

TEST_CASE("KLV universal set packet with a checksum", "[klv]") {
	auto rec = record<other::keyed_us>{};
	rec.set<other::keyed_value>(0x1234);
	CHECK(encode_packet(rec) == keyed_packet());

	auto const decoded = decode_packet<other::keyed_us>(keyed_packet());
	REQUIRE(decoded);
	CHECK(decoded->get<other::keyed_value>() == uint16_t{0x1234});
	CHECK(decoded->unknown().empty());

	SECTION("a checksum key with another version byte is the checksum field") {
		auto checksum_key = other::keyed_checksum::key;
		checksum_key[7] = std::byte{0x0E};
		CHECK(decode_packet<other::keyed_us>(keyed_packet(checksum_key)));
	}

	SECTION("a changed byte fails verification") {
		auto bytes = keyed_packet();
		bytes[35] ^= std::byte{0x01};  // the low byte of the Keyed Value
		auto const changed = decode_packet<other::keyed_us>(bytes);
		REQUIRE_FALSE(changed);
		CHECK(changed.error().code == error_code::checksum_mismatch);
	}

	SECTION("a packet without the checksum item fails verification") {
		auto value = byte_vector{};
		append_packet(value, other::keyed_value::key, make_bytes({0x12, 0x34}));
		auto bytes = byte_vector{};
		append_packet(bytes, other::keyed_us::key, value);

		auto const missing = decode_packet<other::keyed_us>(bytes);
		REQUIRE_FALSE(missing);
		CHECK(missing.error().code == error_code::missing_checksum);
	}
}

TEST_CASE("KLV decode_packets mixes local set and universal set packets", "[klv]") {
	auto other_record = record<other::other_ls>{};
	other_record.set<other::value>(5);

	auto buffer = encode_sample();
	auto const keyed = keyed_packet();
	buffer.insert(buffer.end(), keyed.begin(), keyed.end());
	REQUIRE(encode_packet(other_record, buffer));

	auto results = std::vector<result<packet<kv::uas_datalink_ls, other::keyed_us, other::other_ls>>>{};
	for (auto&& item : decode_packets<kv::uas_datalink_ls, other::keyed_us, other::other_ls>(buffer)) {
		results.push_back(std::move(item));
	}

	REQUIRE(results.size() == 3);
	REQUIRE(results[0]);
	CHECK(std::holds_alternative<uas_record>(*results[0]));
	REQUIRE(results[1]);
	auto const* universal = std::get_if<record<other::keyed_us>>(&*results[1]);
	REQUIRE(universal != nullptr);
	CHECK(universal->get<other::keyed_value>() == uint16_t{0x1234});
	REQUIRE(results[2]);
	CHECK(std::get_if<record<other::other_ls>>(&*results[2]) != nullptr);
}

namespace {

using precision_item = item<misb0603::precision_time_stamp_item>;

/// Core Identifier of MISB ST 1204 Table 7: a physical Sensor ID and a virtual Platform ID
constexpr auto core_id_value = std::string_view{
    "01 70 F5 92 F0 23 73 36 4A F8 AA 91 62 C0 0F 2E B2 DA 16 B7 43 41 00 08 41 A0 BE 36 5B 5A B9 6A 36 45"
};

}  // namespace

static_assert(item_definition<misb1204::core_id_item> && packet_definition<misb1204::core_id_item>);
static_assert(!item_definition<kv::uas_datalink_ls> && packet_definition<kv::uas_datalink_ls>);
static_assert(!item_definition<kv::mission_id> && !packet_definition<kv::mission_id>);

TEST_CASE("KLV decode_packets mixes sets and standalone items", "[klv]") {
	auto buffer = encode_sample();
	append_packet(buffer, misb1204::core_id_item::key, klv_test::hex(core_id_value));
	REQUIRE(encode_packet(precision_item{sample_time}, buffer));

	auto unknown_key = misb1204::core_id_item::key;
	unknown_key[12] = std::byte{0x7E};
	append_packet(buffer, unknown_key, make_bytes({0x01}));

	// A Precision Time Stamp item with 7 bytes does not decode
	auto const bad_offset = buffer.size();
	append_packet(buffer, misb0603::precision_time_stamp_item::key, make_bytes({0, 1, 2, 3, 4, 5, 6}));

	using packets = packet<kv::uas_datalink_ls, misb1204::core_id_item, precision_item::definition>;
	auto results = std::vector<result<packets>>{};
	for (auto&& one : decode_packets<kv::uas_datalink_ls, misb1204::core_id_item, precision_item::definition>(buffer)) {
		results.push_back(std::move(one));
	}

	REQUIRE(results.size() == 5);
	REQUIRE(results[0]);
	CHECK(std::holds_alternative<uas_record>(*results[0]));

	REQUIRE(results[1]);
	auto const* core = std::get_if<item<misb1204::core_id_item>>(&*results[1]);
	REQUIRE(core != nullptr);
	CHECK(
	    misb1204::core_id_item::format.to_string(core->value)
	    == "0170:F592-F023-7336-4AF8-AA91-62C0-0F2E-B2DA/16B7-4341-0008-41A0-BE36-5B5A-B96A-3645:D3"
	);

	REQUIRE(results[2]);
	CHECK(std::get<precision_item>(*results[2]).value == sample_time);

	REQUIRE(results[3]);
	CHECK(std::get<unknown_packet>(*results[3]).key == unknown_key);

	REQUIRE_FALSE(results[4]);
	CHECK(results[4].error().code == error_code::invalid_value);
	CHECK(results[4].error().offset == bad_offset);
	CHECK(results[4].error().message == std::format("Precision Time Stamp at offset {}: bad length", bad_offset));
}

TEST_CASE("KLV standalone item decoding and encoding", "[klv]") {
	auto const bytes = encode_packet(precision_item{sample_time});
	REQUIRE(bytes);
	auto expected = byte_vector{};
	append_packet(expected, misb0603::precision_time_stamp_item::key, klv_test::hex("00 04 59 F4 A6 AA 4A A8"));
	CHECK(*bytes == expected);
	CHECK(decode_packet<misb0603::precision_time_stamp_item>(*bytes) == precision_item{sample_time});

	SECTION("an empty item is an error") {
		auto empty = byte_vector{};
		append_packet(empty, misb0603::precision_time_stamp_item::key, {});
		auto const decoded = decode_packet<misb0603::precision_time_stamp_item>(empty);
		REQUIRE_FALSE(decoded);
		CHECK(decoded.error().code == error_code::invalid_value);
		CHECK(decoded.error().message == "Precision Time Stamp at offset 0: empty");
	}

	SECTION("another key is an error") {
		auto const decoded = decode_packet<misb1204::core_id_item>(*bytes);
		REQUIRE_FALSE(decoded);
		CHECK(decoded.error().code == error_code::unknown_key);
	}

	SECTION("a value that does not encode keeps the buffer") {
		auto out = make_bytes({0xEE});
		auto const bad = item<misb0603::time_status_item>{{.locked = true, .jump = misb0603::time_jump{3}}};
		auto const status = encode_packet(bad, out);
		REQUIRE_FALSE(status);
		CHECK(status.error().code == error_code::invalid_value);
		CHECK(status.error().message == "Cannot encode item 'Time Status': invalid value");
		CHECK(out == make_bytes({0xEE}));
	}
}
