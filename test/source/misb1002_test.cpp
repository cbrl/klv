// The Range Image example is a test vector of the KWIVER test suite.

#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <vector>

#include "klv_test_util.hpp"
#include "klv/misb1002.hpp"
#include "klv/misb1202.hpp"
#include "klv/packet.hpp"

using namespace klv;
using klv_test::hex;
using klv_test::micros;

static_assert(has_checksum<misb1002::range_image_ls>);

TEST_CASE("KLV ST 1002 Range Image set decodes and re-encodes an example", "[klv][misb1002]") {
	auto const bytes = hex(
	    "01 08 00 00 00 00 00 00 12 34 0B 01 02 0C 01 48 0D 04 43 80 00 00 0E 04 40 80 00 00 0F 04 43 A0 00 00 "
	    "10 04 43 70 00 00 11 01 04 12 01 01 13 03 0A 01 01 "
	    "14 25 01 02 01 00 15 02 02 02 04 01 42 C8 00 00 42 D2 00 00 42 BE 00 00 42 C8 00 00 00 04 3F 80 00 00 "
	    "04 40 00 00 00"
	);

	auto const rec = decode_set<misb1002::range_image_ls>(bytes);
	REQUIRE(rec);
	CHECK(rec->get<misb1002::precision_time_stamp>() == micros(0x1234));
	CHECK(rec->get<misb1002::document_version>() == uint32_t{2});
	CHECK(
	    rec->get<misb1002::range_image_enumerations_pack>()
	    == misb1002::range_image_enumerations{
	        .compression = misb1002::compression_method::none,
	        .data_type = misb1002::range_data_type::depth_range_image,
	        .source = misb1002::range_image_source::range_sensor,
	    }
	);
	CHECK(rec->get<misb1002::single_point_range_measurement>() == 256.0);
	CHECK(rec->get<misb1002::single_point_range_measurement_row>() == 320.0);
	CHECK(rec->get<misb1002::number_of_sections_in_x>() == uint32_t{4});

	auto const& transform = rec->get<misb1002::generalized_transformation_local_set>();
	REQUIRE(transform);
	CHECK(transform->get<misb1202::document_version>() == uint8_t{1});

	auto const& sections = rec->get<misb1002::section_data_pack>();
	REQUIRE(sections);
	REQUIRE(sections->size() == 1);
	auto const& section = sections->front();
	CHECK(section.section_x == 2);
	CHECK(section.section_y == 0);
	CHECK(section.measurements.sizes == std::vector<uint32_t>{2, 2});
	CHECK(section.measurements.elements == std::vector<double>{100.0, 105.0, 95.0, 100.0});
	CHECK_FALSE(section.uncertainty);
	CHECK(section.plane_x_scale == 1.0);
	CHECK(section.plane_y_scale == 2.0);
	CHECK_FALSE(section.plane_constant);

	CHECK(encode_set(*rec) == bytes);
}

TEST_CASE("KLV ST 1002 stand-alone packet carries a CRC-16-CCITT", "[klv][misb1002]") {
	auto const bytes = hex("06 0E 2B 34 02 0B 01 01 0E 01 03 03 0C 00 00 00 07 0B 01 02 15 02 32 3E");
	auto rec = record<misb1002::range_image_ls>{};
	rec.set<misb1002::document_version>(2);
	CHECK(encode_packet(rec) == bytes);

	auto const decoded = decode_packet<misb1002::range_image_ls>(bytes);
	REQUIRE(decoded);
	CHECK(decoded->get<misb1002::checksum>() == uint16_t{0x323E});
}
