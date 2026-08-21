#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <string>
#include <vector>

#include "klv_test_util.hpp"
#include "klv/misb1601.hpp"

using namespace klv;
using Catch::Approx;
using klv_test::hex;

TEST_CASE("KLV ST 1601 Geo-Registration set re-encodes an example", "[klv][misb1601]") {
	namespace geo = klv::misb1601;
	auto const bytes = hex(
	    "01 01 01 02 04 41 4C 47 4F 03 03 31 2E 30 "
	    "04 15 02 04 04 01 01 85 80 61 45 1F 5B 7A 81 59 52 34 1B 7D B0 CC D2 "
	    "05 25 02 02 04 04 01 42 03 EF C8 42 03 EF CA 42 03 EF CD 42 03 EF D0 42 40 55 E5 42 40 55 E7 42 40 55 EA "
	    "42 40 55 ED "
	    "06 08 74 65 73 74 2E 69 6D 67 07 10 AA BB CC DD EE FF 00 11 22 33 44 55 66 77 88 99 "
	    "08 18 01 04 03 02 C4 61 00 00 46 94 70 00 09 60 00 09 61 00 09 60 00 09 5F 00 "
	    "09 35 02 06 04 02 01 01 50 02 50 03 50 04 50 01 60 02 60 03 60 04 60 05 00 06 00 07 00 08 00 "
	    "0A 70 0B 70 0C 70 0D 70 0A 80 0B 80 0C 80 0D 80 15 00 16 00 17 00 18 00 "
	    "0A 1D 02 06 04 01 01 00 04 10 14 10 14 00 04 20 24 50 54 30 34 40 44 40 44 30 34 50 54 20 24"
	);

	auto const rec = decode_set<geo::geo_registration_ls>(bytes);
	REQUIRE(rec);
	CHECK(rec->get<geo::document_version>() == uint32_t{1});
	CHECK(rec->get<geo::algorithm_name>() == std::string{"ALGO"});
	CHECK(rec->get<geo::second_image_name>() == std::string{"test.img"});

	auto const& pixels = rec->get<geo::correspondence_points_row_column>();
	REQUIRE(pixels);
	CHECK(pixels->sizes == std::vector<uint32_t>{4, 4});
	CHECK(
	    pixels->elements
	    == std::vector<uint64_t>{133, 128, 97, 69, 31, 91, 122, 129, 89, 82, 52, 27, 125, 176, 204, 210}
	);

	auto const& points = rec->get<geo::correspondence_points_latitude_longitude>();
	REQUIRE(points);
	CHECK(points->elements.front() == Approx(32.98416));
	CHECK(points->elements.back() == Approx(48.08391));

	auto const& elevation = rec->get<geo::correspondence_points_elevation>();
	REQUIRE(elevation);
	CHECK(elevation->elements == std::vector<double>{1500.0, 1501.0, 1500.0, 1499.0});
	CHECK(elevation->imapb_min == Approx(-900.0));

	auto const& pixel_sdcc = rec->get<geo::correspondence_points_row_column_sdcc>();
	REQUIRE(pixel_sdcc);
	CHECK(pixel_sdcc->sizes == std::vector<uint32_t>{6, 4});
	CHECK(pixel_sdcc->elements[0] == Approx(1.3125));
	CHECK(pixel_sdcc->elements[8] == Approx(-0.921875));
	CHECK(pixel_sdcc->elements[23] == Approx(-0.625));

	auto const& geographic_sdcc = rec->get<geo::correspondence_points_latitude_longitude_elevation_sdcc>();
	REQUIRE(geographic_sdcc);
	CHECK(
	    geographic_sdcc->elements
	    == std::vector<double>{0,   32,  128, 160, 128,  160,    0,     32,    -0.5,  -0.4375, 0.25,  0.3125,
	                           384, 416, 512, 544, 0, 0.0625, -0.25, -0.1875, 0.25, 0.3125, -0.5, -0.4375}
	);

	CHECK(encode_set(*rec) == bytes);
}

TEST_CASE("KLV ST 1601 SDCC arrays need a valid shape", "[klv][misb1601]") {
	constexpr auto format = misb1601::correspondence_points_row_column_sdcc::format;

	// Five rows instead of six
	CHECK(format.decode(hex("02 05 01 01 01 00 00 00 00 00")) == std::unexpected(field_error::invalid_value));

	auto out = byte_vector{};
	auto writer = byte_writer{out};
	auto const one_dimension = mdarray<double>{.sizes = {6}, .elements = {0, 0, 0, 0, 0, 0}, .element_size = 1};
	CHECK(format.encode(one_dimension, writer) == std::unexpected(field_error::invalid_value));
}
