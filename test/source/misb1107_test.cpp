// The first example is a test vector of the KWIVER test suite.

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <expected>
#include <string>
#include <string_view>
#include <vector>

#include "klv_test_util.hpp"
#include "klv/misb1107.hpp"
#include "klv/packet.hpp"

using namespace klv;
using Catch::Approx;
using klv_test::hex;
using klv_test::micros;

namespace mg = klv::misb1107;

static_assert(keyed_set_definition<mg::metric_geopositioning_ls>);
static_assert(has_checksum<mg::metric_geopositioning_ls>);
static_assert(has_covering_field<mg::metric_geopositioning_ls>);

namespace {

/// Metric Geopositioning Local Set without its checksum
constexpr auto set_bytes = std::string_view{
    "01 03 0A 0B 00 02 03 0A 0C 00 03 03 0A 0D 00 04 03 0A 0B 00 05 03 0A 0C 00 06 03 0A 0D 00 "
    "07 02 42 00 08 02 42 00 09 02 42 00 0A 02 43 00 0B 02 43 00 0C 02 43 00 "
    "0D 02 44 00 0E 02 45 00 0F 02 46 00 10 01 70 11 01 71 12 01 72 13 02 30 00 14 02 31 00 15 01 20 "
    "16 04 3F 80 00 00 17 04 40 00 00 00 18 04 40 40 00 00 19 04 40 80 00 00 1A 04 40 A0 00 00 "
    "1B 04 40 C0 00 00 1C 04 40 E0 00 00 1D 04 41 00 00 00 1E 04 41 10 00 00 1F 04 41 20 00 00 "
    "20 00 21 00 22 02 02 D0 23 02 04 38 24 01 40 25 01 60 26 01 01 "
    "27 04 41 30 00 00 28 04 41 40 00 00 29 04 41 50 00 00 2A 04 41 60 00 00 "
    "2B 08 00 01 02 03 04 05 06 07 2C 01 04 2E 01 25 2F 02 01 00"
};

}  // namespace

TEST_CASE("KLV ST 1107 set decodes and re-encodes an example", "[klv][misb1107]") {
	auto const bytes = hex(set_bytes);
	auto const rec = decode_set<mg::metric_geopositioning_ls>(bytes);
	REQUIRE(rec);
	CHECK(rec->unknown().empty());

	// Each IMAPB value keeps its byte length
	CHECK(rec->get<mg::sensor_ecef_position_x>() == imapb_value{.value = -831'506'944.0, .length = 3});
	CHECK(rec->get<mg::sensor_ecef_position_y>() == imapb_value{.value = -831'441'408.0, .length = 3});
	CHECK(rec->get<mg::sensor_ecef_position_z>() == imapb_value{.value = -831'375'872.0, .length = 3});
	CHECK(rec->get<mg::sensor_ecef_velocity_x>() == imapb_value{.value = -19'858.0, .length = 3});
	CHECK(rec->get<mg::sensor_ecef_velocity_z>() == imapb_value{.value = -19'854.0, .length = 3});
	CHECK(rec->get<mg::sensor_absolute_azimuth>() == imapb_value{.value = 1.03125, .length = 2});
	CHECK(rec->get<mg::sensor_absolute_pitch>() == imapb_value{.value = 0.03125, .length = 2});
	CHECK(rec->get<mg::sensor_absolute_roll_rate>() == imapb_value{.value = 0.046875, .length = 2});
	CHECK(rec->get<mg::boresight_offset_delta_x>() == imapb_value{.value = 244.0, .length = 2});
	CHECK(rec->get<mg::boresight_offset_delta_z>() == imapb_value{.value = 260.0, .length = 2});
	CHECK(rec->get<mg::boresight_delta_angle_1>() == imapb_value{.value = 0.1875, .length = 1});
	CHECK(rec->get<mg::boresight_delta_angle_2>() == imapb_value{.value = 0.19140625, .length = 1});
	CHECK(rec->get<mg::focal_plane_principal_point_offset_y>() == imapb_value{.value = -1.0, .length = 2});
	CHECK(rec->get<mg::focal_plane_principal_point_offset_x>() == imapb_value{.value = -0.5, .length = 2});
	CHECK(rec->get<mg::sensor_calibrated_focal_length>() == imapb_value{.value = 4096.0, .length = 1});
	CHECK(rec->get<mg::sensor_calibrated_focal_length_extended>() == imapb_value{.value = 1024.0, .length = 2});

	CHECK(rec->get<mg::radial_distortion_constant>() == 1.0);
	CHECK(rec->get<mg::third_tangential_decentering_parameter>() == 7.0);
	CHECK(rec->get<mg::slant_range>() == 10.0);
	CHECK(rec->get<mg::radial_distortion_valid_range>() == 14.0);

	// Zero-length items
	CHECK(rec->get<mg::sdcc_flp>().error() == field_error::empty);
	CHECK(rec->get<mg::generalized_transformation_local_set>().error() == field_error::empty);

	CHECK(rec->get<mg::image_rows>() == uint16_t{720});
	CHECK(rec->get<mg::image_columns>() == uint16_t{1080});
	CHECK(rec->get<mg::pixel_size_x>()->value == Approx(0.0626));
	CHECK(rec->get<mg::pixel_size_y>()->value == Approx(0.09385));
	CHECK(rec->get<mg::slant_range_pedigree>() == mg::range_pedigree::measured);
	CHECK(rec->get<mg::precision_time_stamp>() == micros(0x0001'0203'0405'0607));
	CHECK(rec->get<mg::document_version>() == uint32_t{4});
	CHECK(rec->get<mg::leap_seconds>() == int32_t{37});

	CHECK(encode_set(*rec) == bytes);
}

TEST_CASE("KLV ST 1107 stand-alone packet carries a CRC-16-CCITT", "[klv][misb1107]") {
	auto value = hex(set_bytes);
	auto const crc = hex("2D 02 A7 5A");
	value.insert(value.end(), crc.begin(), crc.end());
	auto packet = byte_vector{};
	klv_test::append_packet(packet, mg::metric_geopositioning_ls::key, value);

	auto const decoded = decode_packet<mg::metric_geopositioning_ls>(packet);
	REQUIRE(decoded);
	CHECK(decoded->get<mg::checksum>() == uint16_t{0xA75A});
	CHECK(encode_packet(*decoded) == packet);

	auto damaged = packet;
	damaged[22] = std::byte{0x0C};
	auto const failed = decode_packet<mg::metric_geopositioning_ls>(damaged);
	REQUIRE_FALSE(failed);
	CHECK(failed.error().code == error_code::checksum_mismatch);
}

TEST_CASE("KLV ST 1107 SDCC-FLP standard deviations use the ranges of Table 1", "[klv][misb1107]") {
	auto rec = record<mg::metric_geopositioning_ls>{};
	rec.set<mg::sensor_ecef_position_x>(imapb_value{.value = 1.0e6, .length = 4});
	rec.set<mg::sensor_ecef_position_y>(imapb_value{.value = -2.0e6, .length = 4});
	rec.set<mg::sensor_calibrated_focal_length_extended>(imapb_value{.value = 50.0, .length = 3});
	rec.set<mg::document_version>(4);
	rec.add<mg::sdcc_flp>(sdcc_flp{
	    .size = 3,
	    .members =
	        {mg::sensor_ecef_position_x::tag,
	         mg::sensor_ecef_position_y::tag,
	         mg::sensor_calibrated_focal_length_extended::tag},
	    .sigma = {1.5, 2.0, 300.0},
	    .sigma_length = 2,
	    .rho_length = 0,
	    .sigma_imapb = true,
	});

	auto const bytes = encode_packet(rec);
	REQUIRE(bytes);
	auto decoded = decode_packet<mg::metric_geopositioning_ls>(*bytes);
	REQUIRE(decoded);
	decoded->erase<mg::checksum>();
	CHECK(*decoded == rec);

	auto const& sdcc = decoded->get<mg::sdcc_flp>();
	REQUIRE(sdcc);
	REQUIRE(sdcc->size() == 1);
	CHECK(sdcc->front().sigma == std::vector{1.5, 2.0, 300.0});
	CHECK(sdcc->front().sigma_raw.empty());

	SECTION("the focal length uses IMAPB(0, 350)") {
		// 300 is 0x4B00 in 2 bytes of IMAPB(0, 350), and 0x2580 in 2 bytes of IMAPB(0, 650)
		auto out = byte_vector{};
		auto writer = byte_writer{out};
		auto const focal = sdcc_flp{
		    .size = 1,
		    .members = {mg::sensor_calibrated_focal_length::tag},
		    .sigma = {300.0},
		    .sigma_length = 2,
		    .rho_length = 0,
		    .sigma_imapb = true,
		};
		REQUIRE(mg::sdcc_flp::format.item.encode(focal, writer));
		CHECK(byte_vector(out.end() - 2, out.end()) == hex("4B 00"));
	}

	SECTION("items 22 to 30 have no IMAPB range") {
		CHECK_FALSE(mg::sigma_range(mg::radial_distortion_constant::tag));
		CHECK_FALSE(mg::sigma_range(mg::skewness_affine_parameter::tag));
		CHECK(mg::sigma_range(mg::slant_range::tag)->max == Approx(650.0));

		auto distortion = record<mg::metric_geopositioning_ls>{};
		distortion.set<mg::radial_distortion_constant>(1.0);
		distortion.add<mg::sdcc_flp>(sdcc_flp{
		    .size = 1,
		    .members = {mg::radial_distortion_constant::tag},
		    .sigma_raw = {5},
		    .sigma_length = 2,
		    .rho_length = 0,
		    .sigma_imapb = true,
		});
		auto const raw = encode_set(distortion);
		REQUIRE(raw);
		auto const read = decode_set<mg::metric_geopositioning_ls>(*raw);
		REQUIRE(read);
		CHECK(read->get<mg::sdcc_flp>()->front().sigma.empty());
		CHECK(read->get<mg::sdcc_flp>()->front().sigma_raw == std::vector<uint64_t>{5});
	}
}

TEST_CASE("KLV ST 1107 float items accept binary16", "[klv][misb1107]") {
	auto const rec = decode_set<mg::metric_geopositioning_ls>(hex("16 02 3C 00 1F 02 C0 00"));
	REQUIRE(rec);
	CHECK(rec->get<mg::radial_distortion_constant>() == 1.0);
	CHECK(rec->get<mg::slant_range>() == -2.0);

	// Encoding writes binary32
	CHECK(encode_set(*rec) == hex("16 04 3F 80 00 00 1F 04 C0 00 00 00"));
}

TEST_CASE("KLV ST 1107 IMAPB items", "[klv][misb1107]") {
	SECTION("a special value is the error indicator") {
		auto const rec = decode_set<mg::metric_geopositioning_ls>(hex("01 03 D0 00 00"));
		REQUIRE(rec);
		CHECK(rec->get<mg::sensor_ecef_position_x>().error() == field_error::error_indicator);
		CHECK(encode_set(*rec) == hex("01 04 D0 00 00 00"));
	}

	SECTION("the producer chooses the length") {
		auto rec = record<mg::metric_geopositioning_ls>{};
		rec.set<mg::sensor_ecef_position_z>(imapb_value{.value = 4'500'000.125, .length = 8});
		auto const bytes = encode_set(rec);
		REQUIRE(bytes);
		CHECK(bytes->size() == 10);
		auto const decoded = decode_set<mg::metric_geopositioning_ls>(*bytes);
		REQUIRE(decoded);
		CHECK(decoded->get<mg::sensor_ecef_position_z>() == imapb_value{.value = 4'500'000.125, .length = 8});

		auto const text = describe(*decoded);
		REQUIRE(text.size() == 1);
		CHECK(text.front().value == std::string{"4500000.125000"});
		CHECK(text.front().units == "m");

		rec.set<mg::sensor_ecef_position_z>(imapb_value{.value = 1.0, .length = 0});
		CHECK_FALSE(encode_set(rec));
	}
}
