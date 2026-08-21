// The byte example is a test vector of the KWIVER test suite.

#include <catch2/catch_test_macros.hpp>

#include <cstdint>

#include "klv_test_util.hpp"
#include "klv/misb1602.hpp"

using namespace klv;
using klv_test::hex;
using klv_test::micros;

TEST_CASE("KLV ST 1602 Composite Imaging set re-encodes an example", "[klv][misb1602]") {
	namespace composite = klv::misb1602;
	auto const bytes = hex(
	    "01 08 01 02 03 04 00 00 00 00 02 01 01 03 02 02 D0 04 02 04 38 05 02 01 68 06 02 01 E0 07 01 40 08 01 30 "
	    "09 01 78 0A 02 01 2C 0B 02 00 80 0C 01 40 0D 02 01 E0 0E 02 02 D0 0F 01 C0 10 01 80 11 01 00 12 01 80"
	);

	auto const rec = decode_set<composite::composite_imaging_ls>(bytes);
	REQUIRE(rec);
	CHECK(rec->get<composite::precision_time_stamp>() == micros(0x0102'0304'0000'0000));
	CHECK(rec->get<composite::source_image_rows>() == uint32_t{720});
	CHECK(rec->get<composite::source_image_aoi_position_x>() == int32_t{64});
	CHECK(rec->get<composite::sub_image_position_x>() == int32_t{128});
	CHECK(rec->get<composite::active_sub_image_offset_x>() == int32_t{-64});
	CHECK(rec->get<composite::active_sub_image_offset_y>() == int32_t{-128});
	CHECK(rec->get<composite::z_order>() == uint8_t{128});
	CHECK(encode_set(*rec) == bytes);
}
