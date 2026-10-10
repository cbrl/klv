// The regions come from the example of MISB ST 1507.2 Figures 1 and 2. The Detector Group has 10 rows and 20 columns.
// Region A (columns 0 to 9) is exposed from top to bottom. Region B (columns 10 to 19) is exposed from right to left.

#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <cstdint>
#include <expected>
#include <optional>
#include <string_view>
#include <vector>

#include "klv_test_util.hpp"
#include "klv/misb1507.hpp"
#include "klv/packet.hpp"

using namespace klv;
using klv_test::hex;
using std::chrono::nanoseconds;

namespace st = klv::misb1507;
namespace tt = klv::misb1603;

namespace {

/// Sensor Timing Local Set packet with a Reference Time Pack, a Window Corners Pack, and the regions A and B
constexpr auto timing_packet = std::string_view{
    "06 0E 2B 34 02 0B 01 01 0E 01 03 02 01 00 00 00 40 "
    "01 0E 17 97 9C FE 3D 85 CD 15 01 01 03 03 01 1A "  // 2023-11-14T22:13:20.123456789Z, ST 1603.3, GPS, slew
    "02 04 01 12 08 01 "  // C1 (1, 18), C2 (8, 1)
    "03 24 "
    "04 10 00 00 00 BD 84 40 00 09 87 04 01 00 CE 10 09 09 "  // Region A
    "04 10 00 13 00 BD 84 40 09 13 87 04 00 12 CE 10 09 0A "  // Region B
    "04 02 28 6E"
};

/// Each detector of a Sub-Region starts 100 ns after the one before it. Each Sub-Region starts 10 us after the one
/// before it. Each exposure is 1 ms.
auto const region_a = st::region{
    .first = {.row = 0, .column = 0},
    .start = nanoseconds{0},
    .end = nanoseconds{1'000'000},
    .sub_region_end = st::detector_start{.location = {.row = 0, .column = 9}, .start = nanoseconds{900}},
    .second_sub_region = st::detector_start{.location = {.row = 1, .column = 0}, .start = nanoseconds{10'000}},
    .last = {.row = 9, .column = 9},
};

auto const region_b = st::region{
    .first = {.row = 0, .column = 19},
    .start = nanoseconds{0},
    .end = nanoseconds{1'000'000},
    .sub_region_end = st::detector_start{.location = {.row = 9, .column = 19}, .start = nanoseconds{900}},
    .second_sub_region = st::detector_start{.location = {.row = 0, .column = 18}, .start = nanoseconds{10'000}},
    .last = {.row = 9, .column = 10},
};

auto const global = st::region{
    .first = {.row = 0, .column = 0},
    .start = nanoseconds{250},
    .end = nanoseconds{2'000},
    .sub_region_end = std::nullopt,
    .second_sub_region = std::nullopt,
    .last = {.row = 9, .column = 19},
};

[[nodiscard]]
auto reference_time() -> nano_timestamp {
	return nano_timestamp{nanoseconds{1'700'000'000'123'456'789}};
}

[[nodiscard]]
auto make_timing() -> record<st::sensor_timing_ls> {
	auto transfer = record<tt::time_transfer_ls>{};
	transfer.set<tt::document_version>(uint8_t{3});
	transfer.set<tt::time_transfer_parameters>({
	    .source = tt::reference_source::synchronized,
	    .correction = tt::correction_method::slew,
	    .method = tt::transfer_method::gps_pps,
	});

	auto list = record<st::region_ls>{};
	st::add_region(list, region_a);
	st::add_region(list, region_b);

	auto timing = record<st::sensor_timing_ls>{};
	timing.set<st::reference_time_pack>({.time = reference_time(), .transfer = transfer});
	timing.set<st::window_corners_pack>({.c1 = {.row = 1, .column = 18}, .c2 = {.row = 8, .column = 1}});
	timing.set<st::region_list>(list);
	return timing;
}

}  // namespace

TEST_CASE("KLV ST 1507 Sensor Timing Local Set packet", "[klv][misb1507]") {
	auto const decoded = decode_packet<st::sensor_timing_ls>(hex(timing_packet));
	REQUIRE(decoded);
	CHECK(decoded->unknown().empty());
	CHECK(decoded->get<st::checksum>() == uint16_t{0x286E});

	auto expected = make_timing();
	expected.set<st::checksum>(uint16_t{0x286E});
	CHECK(*decoded == expected);
	CHECK(encode_packet(make_timing()) == hex(timing_packet));

	// The exposure times of the detectors of Tuple 0, from the Reference Time
	auto const& reference = decoded->get<st::reference_time_pack>();
	auto const& list = decoded->get<st::region_list>();
	REQUIRE(reference);
	REQUIRE(list);
	auto const all = st::regions(*list);
	REQUIRE(all.size() == 2);
	CHECK(reference->time + all[1].end == nano_timestamp{nanoseconds{1'700'000'000'124'456'789}});

	auto corrupt = hex(timing_packet);
	corrupt[20] ^= std::byte{0x01};
	CHECK_FALSE(decode_packet<st::sensor_timing_ls>(corrupt));
}

TEST_CASE("KLV ST 1507 regions of each style", "[klv][misb1507]") {
	auto with_start = region_a;
	with_start.second_sub_region.reset();
	auto with_delay = region_b;
	with_delay.sub_region_end.reset();

	auto list = record<st::region_ls>{};
	st::add_region(list, region_a);
	st::add_region(list, with_delay);
	st::add_region(list, with_start);
	st::add_region(list, global);
	CHECK(list.get<st::global_shutter>() == std::vector{global});
	CHECK(list.get<st::detector_delay>() == std::vector{with_start});
	CHECK(list.get<st::sub_region_delay>() == std::vector{with_delay});
	CHECK(list.get<st::detector_and_sub_region_delay>() == std::vector{region_a});
	CHECK(st::regions(list) == std::vector{global, with_start, with_delay, region_a});

	auto const bytes = encode_set(list);
	REQUIRE(bytes);
	auto const decoded = decode_set<st::region_ls>(*bytes);
	REQUIRE(decoded);
	CHECK(*decoded == list);

	// A global shutter region has Tuple 0 and Tuple 3 only
	CHECK(encode_value(st::global_shutter::format.item, global) == hex("00 00 81 7A 8F 50 09 13"));
	CHECK(st::global_shutter::format.item.decode(hex("00 00 81 7A 8F 50 09 13")) == global);
}

TEST_CASE("KLV ST 1507 region errors", "[klv][misb1507]") {
	constexpr auto format = st::region_codec{};

	// The tuples of the region do not match the tag
	CHECK(encode_value(format, region_a) == std::unexpected(field_error::invalid_value));
	CHECK(encode_value(st::detector_delay::format.item, global) == std::unexpected(field_error::invalid_value));

	auto early = global;
	early.end = nanoseconds{-1};
	CHECK(encode_value(format, early) == std::unexpected(field_error::out_of_range));

	CHECK(format.decode(hex("00 00 81 7A 8F 50 09")) == std::unexpected(field_error::invalid_value));
	CHECK(format.decode(hex("00 00 81 7A 8F 50 09 13 00")) == std::unexpected(field_error::invalid_value));
	CHECK(format.decode(hex("00 00 80 01 02 09 13")) == std::unexpected(field_error::invalid_value));
}

TEST_CASE("KLV ST 1507 time offsets use up to 64 bits", "[klv][misb1507]") {
	constexpr auto format = st::region_codec{};

	auto late = global;
	late.end = nanoseconds{5'000'000'000};
	auto const bytes = encode_value(format, late);
	CHECK(bytes == hex("00 00 81 7A 92 D0 97 E4 00 09 13"));
	CHECK(format.decode(hex("00 00 81 7A 92 D0 97 E4 00 09 13")) == late);

	// 2^63 - 1 decodes. 2^63 is out of range, and a value wider than 64 bits is not valid.
	auto last = global;
	last.end = nanoseconds::max();
	CHECK(format.decode(hex("00 00 81 7A FF FF FF FF FF FF FF FF 7F 09 13")) == last);
	CHECK(
	    format.decode(hex("00 00 81 7A 81 80 80 80 80 80 80 80 80 00 09 13"))
	    == std::unexpected(field_error::out_of_range)
	);
	CHECK(
	    format.decode(hex("00 00 81 7A 82 80 80 80 80 80 80 80 80 80 00 09 13"))
	    == std::unexpected(field_error::invalid_value)
	);
}

TEST_CASE("KLV ST 1507 Window Corners Pack", "[klv][misb1507]") {
	constexpr auto format = st::window_codec{};
	auto const window = st::detector_window{.c1 = {.row = 1079, .column = 1919}, .c2 = {.row = 0, .column = 0}};

	CHECK(encode_value(format, window) == hex("88 37 8E 7F 00 00"));
	CHECK(format.decode(hex("88 37 8E 7F 00 00")) == window);
	CHECK(format.to_string(window) == "{c1: (1079, 1919), c2: (0, 0)}");

	CHECK(format.decode(hex("88 37 8E 7F 00")) == std::unexpected(field_error::invalid_value));
	CHECK(format.decode(hex("88 37 8E 7F 00 00 00")) == std::unexpected(field_error::invalid_value));
}

TEST_CASE("KLV ST 1507 region text", "[klv][misb1507]") {
	CHECK(
	    st::region_codec{.sub_region_end = true, .second_sub_region = true}.to_string(region_b)
	    == "{first: (0, 19), start: 0 ns, end: 1000000 ns, sub_region_end: (9, 19) at 900 ns, "
	       "second_sub_region: (0, 18) at 10000 ns, last: (9, 10)}"
	);
	CHECK(st::region_codec{}.to_string(global) == "{first: (0, 0), start: 250 ns, end: 2000 ns, last: (9, 19)}");
}
