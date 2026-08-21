// The byte example is a test vector of the KWIVER test suite.

#include <catch2/catch_test_macros.hpp>

#include <vector>

#include "klv_test_util.hpp"
#include "klv/misb1202.hpp"

using namespace klv;
using klv_test::hex;

TEST_CASE("KLV ST 1202 Generalized Transformation set re-encodes an example", "[klv][misb1202]") {
	auto const bytes = hex(
	    "01 04 3F 80 00 00 02 04 40 00 00 00 03 04 40 40 00 00 04 04 40 80 00 00 05 04 40 A0 00 00 "
	    "06 04 40 C0 00 00 07 04 40 E0 00 00 08 04 41 00 00 00 "
	    "09 22 08 40 3F 80 00 00 40 00 00 00 40 40 00 00 40 80 00 00 40 A0 00 00 40 C0 00 00 40 E0 00 00 41 00 00 00 "
	    "0A 01 02 0B 01 04"
	);

	auto const rec = decode_set<misb1202::generalized_transformation_ls>(bytes);
	REQUIRE(rec);
	CHECK(rec->get<misb1202::x_numerator_x_factor>() == 1.0F);
	CHECK(rec->get<misb1202::denominator_y_factor>() == 8.0F);
	CHECK(rec->get<misb1202::transformation_type>() == misb1202::transformation_kind::optical);
	auto const& sdcc = rec->get<misb1202::sdcc_flp>();
	REQUIRE(sdcc);
	REQUIRE(sdcc->size() == 1);
	CHECK(sdcc->front().sigma == std::vector{1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0});
	CHECK(encode_set(*rec) == bytes);
}
