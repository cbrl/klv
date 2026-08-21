// The byte example is a test vector of the KWIVER test suite.

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <vector>

#include "klv_test_util.hpp"
#include "klv/misb1206.hpp"

using namespace klv;
using Catch::Approx;
using klv_test::hex;
using klv_test::micros;

TEST_CASE("KLV ST 1206 SAR Motion Imagery set re-encodes an example", "[klv][misb1206]") {
	namespace sar = klv::misb1206;
	auto const bytes = hex(
	    "01 02 40 00 02 02 20 00 03 01 01 04 01 00 05 04 40 00 00 00 06 04 04 00 00 00 07 04 03 00 00 00 "
	    "08 04 02 00 00 00 09 02 02 D0 0A 02 04 38 0B 02 03 00 0C 02 07 00 0D 02 0A 00 0E 02 0F 00 "
	    "0F 04 00 00 10 00 10 02 10 00 11 02 00 80 12 04 40 80 00 00 13 02 05 00 14 04 00 80 80 00 "
	    "15 04 00 70 00 00 "
	    "16 1D 02 04 02 02 02 00 00 00 00 49 74 24 00 01 00 02 00 03 00 04 00 05 00 06 00 07 00 08 00 "
	    "17 08 12 34 56 78 00 00 00 00 18 02 07 10 19 02 01 10 1A 02 03 10 1B 02 05 10 1C 01 01"
	);

	auto const rec = decode_set<sar::sar_motion_imagery_ls>(bytes);
	REQUIRE(rec);
	CHECK(rec->get<sar::grazing_angle>() == 64.0);
	CHECK(rec->get<sar::ground_plane_squint_angle>() == -26.0);
	CHECK(rec->get<sar::look_direction>() == sar::look_side::right);
	CHECK(rec->get<sar::image_plane>() == sar::image_plane_type::slant);
	CHECK(rec->get<sar::range_resolution>() == 524'288.0);
	CHECK(rec->get<sar::image_columns>() == uint16_t{1080});
	CHECK(rec->get<sar::aperture_duration>() == uint32_t{4096});
	CHECK(rec->get<sar::minimum_detectable_velocity>() == 0.5);
	CHECK(rec->get<sar::pulse_repetition_frequency_scale_factor>() == 0.0390625);
	CHECK(rec->get<sar::transmit_rf_center_frequency>() == 4'311'744'512.0);
	CHECK(rec->get<sar::transmit_rf_bandwidth>() == 469'762'048.0);
	CHECK(rec->get<sar::reference_frame_precision_time_stamp>() == micros(1'311'768'464'867'721'216));
	CHECK(rec->get<sar::reference_frame_ground_plane_squint_angle>() == -87.875);
	CHECK(rec->get<sar::document_version>() == uint8_t{1});

	auto const& polynomial = rec->get<sar::radar_cross_section_scale_factor_polynomial>();
	REQUIRE(polynomial);
	CHECK(polynomial->sizes == std::vector<uint32_t>{4, 2});
	CHECK(polynomial->elements == std::vector<double>{8192, 16'384, 24'576, 32'768, 40'960, 49'152, 57'344, 65'536});
	CHECK(polynomial->imapb_max == Approx(1'000'000.0));

	CHECK(encode_set(*rec) == bytes);
}
