// ST 0602.5 gives no example. A script made the stream from the keys of section 7, and it checks each key against its
// CRC.

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <initializer_list>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

#include "klv_test_util.hpp"
#include "klv/misb0602.hpp"
#include "klv/packet.hpp"

using namespace klv;
using klv_test::append_packet;
using klv_test::hex;

namespace ann = klv::misb0602;

static_assert(universal_set_definition<ann::annotation_us> && !has_checksum<ann::annotation_us>);
static_assert(item_definition<ann::byte_order_item> && item_definition<ann::active_samples_per_line_item>);
static_assert(ann::parse_media_type("image/svg+xml") == ann::media_type::svg);

namespace {

/// The preface items: Byte Order 0x4D4D, 1080 Active Lines per Frame, and 1920 Active Samples per Line
constexpr auto preface = std::string_view{
    "06 0E 2B 34 01 01 01 01 03 01 02 01 02 00 00 00 02 4D 4D "
    "06 0E 2B 34 01 01 01 01 04 01 03 02 02 00 00 00 02 04 38 "
    "06 0E 2B 34 01 01 01 01 04 01 05 01 02 00 00 00 02 07 80"
};

/// A NEW annotation of object 7 with all 10 items. The value has 274 bytes.
constexpr auto new_annotation = std::string_view{
    "06 0E 2B 34 02 01 01 01 0E 01 03 03 01 00 00 00 82 01 12 "
    "06 0E 2B 34 01 01 01 01 01 03 03 01 00 00 00 00 04 00 00 00 07 "
    "06 0E 2B 34 01 01 01 01 03 02 01 06 03 00 00 00 05 54 72 75 63 6B "  // Truck
    "06 0E 2B 34 01 01 01 07 04 09 02 00 00 00 00 00 0D 69 6D 61 67 65 2F 73 76 67 2B 78 6D 6C "  // image/svg+xml
    "06 0E 2B 34 01 01 01 01 0E 01 02 05 01 00 00 00 3E "  // The SVG of svg_text
    "3C 73 76 67 20 78 6D 6C 6E 73 3D 22 68 74 74 70 3A 2F 2F 77 77 77 2E 77 33 2E 6F 72 67 2F 32 "
    "30 30 30 2F 73 76 67 22 20 77 69 64 74 68 3D 22 38 22 20 68 65 69 67 68 74 3D 22 38 22 2F 3E "
    "06 0E 2B 34 01 01 01 01 07 01 02 03 01 00 00 00 02 00 64 "  // X = 100
    "06 0E 2B 34 01 01 01 01 07 01 02 03 02 00 00 00 02 FF EC "  // Y = -20
    "06 0E 2B 34 01 01 01 01 05 01 01 02 00 00 00 00 01 31 "  // NEW
    "06 0E 2B 34 01 01 01 01 0E 01 02 05 02 00 00 00 09 41 6E 61 6C 79 73 74 20 31 "  // Analyst 1
    "06 0E 2B 34 01 01 01 01 0E 01 02 05 03 00 00 00 04 00 00 00 08 "  // Automated from pixel analysis
    "06 0E 2B 34 01 01 01 01 0E 01 02 05 06 00 00 00 02 81 48"  // Z-Order 200
};

/// A MOVE annotation of object 7. The value has 96 bytes.
constexpr auto move_annotation = std::string_view{
    "06 0E 2B 34 02 01 01 01 0E 01 03 03 01 00 00 00 60 "
    "06 0E 2B 34 01 01 01 01 01 03 03 01 00 00 00 00 04 00 00 00 07 "
    "06 0E 2B 34 01 01 01 01 07 01 02 03 01 00 00 00 02 00 6E "  // X = 110
    "06 0E 2B 34 01 01 01 01 07 01 02 03 02 00 00 00 02 FF F6 "  // Y = -10
    "06 0E 2B 34 01 01 01 01 05 01 01 02 00 00 00 00 01 32 "  // MOVE
    "06 0E 2B 34 01 01 01 01 0E 01 02 05 06 00 00 00 02 81 48"
};

constexpr auto svg_text = std::string_view{R"(<svg xmlns="http://www.w3.org/2000/svg" width="8" height="8"/>)"};

[[nodiscard]]
auto make_new() -> record<ann::annotation_us> {
	auto svg = byte_vector{};
	for (auto const c : svg_text) {
		svg.push_back(static_cast<std::byte>(c));
	}

	auto rec = record<ann::annotation_us>{};
	rec.set<ann::locally_unique_identifier>(uint32_t{7});
	rec.set<ann::description>("Truck");
	rec.set<ann::mime_media_type>("image/svg+xml");
	rec.set<ann::mime_data>(std::move(svg));
	rec.set<ann::x_viewport_position>(int16_t{100});
	rec.set<ann::y_viewport_position>(int16_t{-20});
	rec.set<ann::event_indication>(ann::event::new_event);
	rec.set<ann::modification_history>("Analyst 1");
	rec.set<ann::annotation_source>(ann::source_kind::pixel_analysis);
	rec.set<ann::z_order>(uint32_t{200});
	return rec;
}

/// Decode a stream. Each packet must decode.
[[nodiscard]]
auto decode_all(byte_span bytes) -> std::vector<ann::stream_packet> {
	auto packets = std::vector<ann::stream_packet>{};
	for (auto&& one : ann::decode_stream(bytes)) {
		REQUIRE(one);
		packets.push_back(std::move(*one));
	}
	return packets;
}

/// Findings of the missing items, in the given order
[[nodiscard]]
auto missing(std::initializer_list<ann::required_item> items) -> std::vector<ann::finding> {
	auto findings = std::vector<ann::finding>{};
	for (auto const which : items) {
		findings.push_back({.item = which, .reason = shortfall::missing});
	}
	return findings;
}

}  // namespace

TEST_CASE("KLV ST 0602 annotation stream", "[klv][misb0602]") {
	auto const stream =
	    hex(std::string{preface} + " " + std::string{new_annotation} + " " + std::string{move_annotation});
	auto const packets = decode_all(stream);
	REQUIRE(packets.size() == 5);

	auto const& byte_order = std::get<item<ann::byte_order_item>>(packets[0]);
	auto const& lines = std::get<item<ann::active_lines_per_frame_item>>(packets[1]);
	auto const& samples = std::get<item<ann::active_samples_per_line_item>>(packets[2]);
	auto const& added = std::get<record<ann::annotation_us>>(packets[3]);
	auto const& moved = std::get<record<ann::annotation_us>>(packets[4]);

	CHECK(byte_order.value == ann::big_endian_byte_order);
	CHECK(lines.value == uint16_t{1080});
	CHECK(samples.value == uint16_t{1920});
	CHECK(added == make_new());
	CHECK(added.unknown().empty());
	CHECK(moved.get<ann::event_indication>() == ann::event::move);
	CHECK(moved.get<ann::x_viewport_position>() == int16_t{110});
	CHECK(moved.get<ann::y_viewport_position>() == int16_t{-10});
	CHECK_FALSE(moved.has<ann::mime_data>());

	// Each packet encodes to its original bytes
	auto encoded = byte_vector{};
	REQUIRE(encode_packet(byte_order, encoded));
	REQUIRE(encode_packet(lines, encoded));
	REQUIRE(encode_packet(samples, encoded));
	REQUIRE(encode_packet(added, encoded));
	REQUIRE(encode_packet(moved, encoded));
	CHECK(encoded == stream);

	// Both events hold the items that they require
	CHECK(ann::check(added).empty());
	CHECK(ann::check(moved).empty());

	auto const text = describe(added);
	auto const entry = [&](std::string_view name) {
		auto const found = std::ranges::find(text, name, &field_text::name);
		REQUIRE(found != text.end());
		return *found;
	};
	CHECK(entry("Event Indication").value == std::string{"NEW"});
	CHECK(entry("Annotation Source").value == std::string{"Automated from pixel analysis"});
	CHECK(entry("Y Viewport Position in Pixels").value == std::string{"-20"});
	CHECK(entry("Y Viewport Position in Pixels").units == "pixels");
	CHECK(entry("Z-Order").value == std::string{"200"});
	CHECK(entry("MIME Media Type").id == item_id{ann::mime_media_type::key});
}

TEST_CASE("KLV ST 0602 item values", "[klv][misb0602]") {
	constexpr auto invalid = std::unexpected(field_error::invalid_value);

	SECTION("Event Indication is one digit") {
		constexpr auto format = ann::event_indication::format;
		CHECK(format.decode(hex("35")) == ann::event::status);
		CHECK(format.decode(hex("36")) == invalid);
		CHECK(format.decode(hex("31 32")) == std::unexpected(field_error::bad_length));
		CHECK(encode_value(format, ann::event::delete_event) == hex("34"));
		CHECK(format.to_string(ann::event::delete_event) == "DELETE");
	}

	SECTION("Annotation Source has the values of Table 4") {
		constexpr auto format = ann::annotation_source::format;
		CHECK(format.decode(hex("00 00 00 00")) == ann::source_kind::manual);
		CHECK(format.decode(hex("00 00 00 05")) == invalid);
		CHECK(encode_value(format, ann::source_kind::area_of_interest) == hex("00 00 00 04"));
	}

	SECTION("MIME media types") {
		CHECK(ann::parse_media_type("image/png") == ann::media_type::png);
		CHECK(ann::parse_media_type("IMAGE/X-MS-BMP") == ann::media_type::bmp);
		CHECK(ann::parse_media_type("cgm") == ann::media_type::cgm);
		CHECK(ann::parse_media_type("image/gif") == std::nullopt);
		CHECK(ann::to_string(ann::media_type::jpeg) == "image/jpeg");
	}
}

TEST_CASE("KLV ST 0602 event check", "[klv][misb0602]") {
	using enum ann::required_item;

	auto rec = record<ann::annotation_us>{};
	rec.set<ann::locally_unique_identifier>(uint32_t{7});

	SECTION("a MOVE without a Z-Order") {
		rec.set<ann::event_indication>(ann::event::move);
		rec.set<ann::x_viewport_position>(int16_t{1});
		rec.set<ann::y_viewport_position>(int16_t{2});
		CHECK(ann::check(rec) == missing({z_order}));
	}

	SECTION("a DELETE without a Modification History") {
		rec.set<ann::event_indication>(ann::event::delete_event);
		CHECK(ann::check(rec) == missing({modification_history}));
	}

	SECTION("a STATUS needs the items of a NEW. An empty item does not meet a requirement.") {
		rec = make_new();
		rec.set<ann::event_indication>(ann::event::status);
		rec.set_empty<ann::mime_data>();
		rec.erase<ann::annotation_source>();
		CHECK(ann::check(rec) == missing({mime_data, annotation_source}));
	}

	SECTION("a MODIFY needs no Annotation Source") {
		rec = make_new();
		rec.set<ann::event_indication>(ann::event::modify);
		rec.erase<ann::annotation_source>();
		CHECK(ann::check(rec).empty());
	}

	SECTION("a record without items") {
		CHECK(ann::check(record<ann::annotation_us>{}) == missing({locally_unique_identifier, event_indication}));
	}

	SECTION("an Event Indication without a valid value") {
		rec.assign<ann::event_indication>(std::unexpected(field_error::invalid_value));
		CHECK(ann::check(rec) == missing({event_indication}));
	}

	CHECK(ann::to_string(z_order) == "Z-Order (ST 0602.4-12, -13)");
}

TEST_CASE("KLV ST 0602 keeps unknown items and packets", "[klv][misb0602]") {
	constexpr auto unknown_key = ul("06 0E 2B 34 01 01 01 01 0E 01 02 05 7F 00 00 00");

	auto value = byte_vector{};
	append_packet(value, ann::locally_unique_identifier::key, hex("00 00 00 07"));
	append_packet(value, unknown_key, hex("AB"));
	auto set_bytes = byte_vector{};
	append_packet(set_bytes, ann::annotation_us::key, value);

	auto stream = set_bytes;
	append_packet(stream, unknown_key, hex("CD"));

	auto const packets = decode_all(stream);
	REQUIRE(packets.size() == 2);
	auto const& rec = std::get<record<ann::annotation_us>>(packets[0]);
	REQUIRE(rec.unknown().size() == 1);
	CHECK(rec.unknown()[0] == unknown_field<universal_key>{.id = unknown_key, .value = hex("AB")});
	CHECK(encode_packet(rec) == set_bytes);
	CHECK(std::get<unknown_packet>(packets[1]).key == unknown_key);
}
