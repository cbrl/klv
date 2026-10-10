#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <string>

#include "klv_test_util.hpp"
#include "klv/misb0806.hpp"
#include "klv/packet.hpp"

using namespace klv;
using Catch::Approx;
using klv_test::hex;
using klv_test::micros;

namespace rvt = klv::misb0806;

static_assert(keyed_set_definition<rvt::rvt_ls> && has_checksum<rvt::rvt_ls>);

TEST_CASE("KLV ST 0806 RVT set decodes and re-encodes an example", "[klv][misb0806]") {
	auto const bytes = hex(
	    "02 08 00 00 00 00 00 00 01 00 03 02 01 F4 04 02 01 90 05 01 AB 06 02 00 1E 07 04 00 00 00 14 08 01 04 "
	    "09 04 00 00 08 00 0A 05 4D 50 45 47 32 "
	    "0B 09 01 01 01 02 04 54 45 53 54 "
	    "0C 40 01 02 00 02 02 04 00 00 00 00 03 04 AB 00 AB 00 04 02 12 34 05 01 03 06 04 54 45 53 54 "
	    "07 05 69 63 6F 6E 37 08 02 23 35 09 10 69 74 27 73 31 36 63 68 61 72 61 63 74 65 72 73 "
	    "0A 04 54 65 73 74 "
	    "0D 3A 01 02 00 03 02 04 12 01 34 00 03 04 12 02 34 00 04 04 12 03 34 00 05 04 12 04 34 00 06 01 01 "
	    "07 01 31 08 01 32 09 10 30 31 32 33 34 35 36 37 38 39 41 42 43 44 45 46 0A 01 34 "
	    "0E 01 05 0F 03 41 42 43 10 03 00 04 00 11 03 00 08 00 12 01 06 13 03 58 59 5A 14 03 00 00 64 "
	    "15 03 00 00 C8"
	);

	auto const rec = decode_set<rvt::rvt_ls>(bytes);
	REQUIRE(rec);
	CHECK(rec->unknown().empty());
	CHECK(rec->get<rvt::precision_time_stamp>() == micros(256));
	CHECK(rec->get<rvt::platform_true_airspeed>() == uint16_t{500});
	CHECK(rec->get<rvt::telemetry_accuracy_indicator>() == hex("AB"));
	CHECK(rec->get<rvt::video_data_rate>() == uint32_t{2048});
	CHECK(rec->get<rvt::digital_video_file_format>() == std::string{"MPEG2"});
	CHECK(rec->get<rvt::mgrs_latitude_band_and_grid_square>() == std::string{"ABC"});
	CHECK(rec->get<rvt::mgrs_easting>() == uint32_t{1024});
	CHECK(rec->get<rvt::frame_center_mgrs_northing>() == uint32_t{200});

	auto const& user = rec->get<rvt::user_defined_local_set>();
	REQUIRE(user);
	REQUIRE(user->size() == 1);
	CHECK(
	    user->front().get<rvt::user_data_type_id>()
	    == rvt::user_data_id{.type = rvt::user_data_type::string, .id = 1}
	);
	CHECK(user->front().get<rvt::user_data>() == hex("54 45 53 54"));

	auto const& poi = rec->get<rvt::poi_local_set>();
	REQUIRE(poi);
	REQUIRE(poi->size() == 1);
	auto const& point = poi->front();
	CHECK(point.get<rvt::poi_number>() == uint16_t{2});
	CHECK(*point.get<rvt::poi_latitude>() == Approx(0.0));
	CHECK(*point.get<rvt::poi_longitude>() == Approx(-119.527580793727));
	CHECK(*point.get<rvt::poi_altitude>() == Approx(515.030136568246));
	CHECK(point.get<rvt::poi_type>() == rvt::interest_type::target);
	CHECK(point.get<rvt::poi_label>() == std::string{"it's16characters"});

	auto const& aoi = rec->get<rvt::aoi_local_set>();
	REQUIRE(aoi);
	REQUIRE(aoi->size() == 1);
	CHECK(*aoi->front().get<rvt::aoi_corner_latitude_point_1>() == Approx(12.6595544874014));
	CHECK(*aoi->front().get<rvt::aoi_corner_longitude_point_3>() == Approx(25.335588466998));
	CHECK(aoi->front().get<rvt::aoi_type>() == rvt::interest_type::friendly);

	CHECK(encode_set(*rec) == bytes);
}

TEST_CASE("KLV ST 0806 stand-alone packet carries a CRC-32/MPEG-2", "[klv][misb0806]") {
	auto const bytes = hex(
	    "06 0E 2B 34 02 0B 01 01 0E 01 03 01 02 00 00 00 0B 0F 03 41 42 43 01 04 D6 85 F2 F6"
	);
	auto rec = record<rvt::rvt_ls>{};
	rec.set<rvt::mgrs_latitude_band_and_grid_square>("ABC");
	CHECK(encode_packet(rec) == bytes);

	auto const decoded = decode_packet<rvt::rvt_ls>(bytes);
	REQUIRE(decoded);
	CHECK(decoded->get<rvt::checksum>() == uint32_t{0xD685'F2F6});

	auto damaged = bytes;
	damaged[20] = std::byte{0x58};
	auto const failed = decode_packet<rvt::rvt_ls>(damaged);
	REQUIRE_FALSE(failed);
	CHECK(failed.error().code == error_code::checksum_mismatch);
}
