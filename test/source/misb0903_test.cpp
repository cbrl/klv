// Most byte examples are test vectors of the KWIVER test suite.

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <string>
#include <vector>

#include "klv_test_util.hpp"
#include "klv/misb0903.hpp"
#include "klv/packet.hpp"

using namespace klv;
using Catch::Approx;
using klv_test::hex;
using klv_test::micros;

namespace vmti = klv::misb0903;

static_assert(universal_set_definition<vmti::vmti_ls> && has_checksum<vmti::vmti_ls>);

TEST_CASE("KLV ST 0903 VMTI set decodes an example", "[klv][misb0903]") {
	auto const vtarget_1234 =
	    "68 89 52 02 03 06 40 00 03 03 06 40 00 04 01 1B 05 01 50 06 02 0A CD 07 01 32 08 03 DA A5 20 09 02 33 54 "
	    "0A 03 3A 66 67 0B 03 3E 66 67 0C 02 2A 94 0D 03 3A 66 67 0E 03 3A 66 67 0F 03 3A 66 67 10 03 3A 66 67 "
	    "11 00 12 00 13 02 03 68 14 02 04 71 15 02 02 03 16 01 03 17 01 04 "
	    "65 00 66 00 67 00 68 00 69 00 6A 00 6B 00 ";
	auto const bytes = hex(
	    std::string{
	        "02 08 00 03 82 44 30 F6 CE 40 03 0E 44 53 54 4F 5F 41 44 53 53 5F 56 4D 54 49 04 01 05 05 01 1C "
	        "06 01 0E 07 03 01 30 B0 08 02 07 80 09 02 04 38 0A 07 45 4F 20 4E 6F 73 65 0B 02 06 40 0C 02 05 00 "
	        "0D 00 "
	        "65 70 06 01 01 03 06 40 00 "
	    }
	    + vtarget_1234
	    + "66 2C 2B 01 01 09 02 14 6B 36 5F 79 6F 6C 6F 5F 39 30 30 30 5F 74 72 61 63 6B 65 72 03 04 32 2E 36 61 "
	      "04 07 6B 61 6C 6D 61 6E 6E 05 01 0A "
	      "67 1F 1E 01 01 11 02 01 0C 03 03 49 52 49 04 06 65 6E 74 69 74 79 05 02 76 32 06 05 6C 61 62 65 6C"
	);

	auto const rec = decode_set<vmti::vmti_ls>(bytes);
	REQUIRE(rec);
	CHECK(rec->unknown().empty());
	CHECK(rec->get<vmti::precision_time_stamp>() == micros(987'654'321'000'000));
	CHECK(rec->get<vmti::vmti_system_name>() == std::string{"DSTO_ADSS_VMTI"});
	CHECK(rec->get<vmti::vmti_ls_version>() == uint16_t{5});
	CHECK(rec->get<vmti::total_number_of_targets_detected>() == uint32_t{28});
	CHECK(rec->get<vmti::number_of_targets_reported>() == uint32_t{14});
	CHECK(rec->get<vmti::frame_number>() == uint32_t{78'000});
	CHECK(rec->get<vmti::frame_width>() == uint32_t{1920});
	CHECK(rec->get<vmti::frame_height>() == uint32_t{1080});
	CHECK(rec->get<vmti::vmti_source_sensor>() == std::string{"EO Nose"});
	CHECK(rec->get<vmti::vmti_horizontal_field_of_view>() == 12.5);
	CHECK(rec->get<vmti::vmti_vertical_field_of_view>() == 10.0);
	CHECK(rec->get<vmti::miis_id>().error() == field_error::empty);

	auto const& targets = rec->get<vmti::vtarget_series>();
	REQUIRE(targets);
	REQUIRE(targets->size() == 2);
	CHECK((*targets)[0].id == 1);
	CHECK((*targets)[0].set.get<vmti::target_centroid>() == uint64_t{409'600});

	auto const& target = (*targets)[1];
	CHECK(target.id == 1234);
	CHECK(target.set.get<vmti::bounding_box_top_left_pixel>() == uint64_t{409'600});
	CHECK(target.set.get<vmti::target_priority>() == uint8_t{27});
	CHECK(target.set.get<vmti::target_confidence_level>() == uint8_t{80});
	CHECK(target.set.get<vmti::target_history>() == uint16_t{2765});
	CHECK(target.set.get<vmti::percentage_of_target_pixels>() == uint8_t{50});
	CHECK(target.set.get<vmti::target_color>() == uint32_t{0xDA'A520});
	CHECK(target.set.get<vmti::target_intensity>() == uint32_t{13'140});
	CHECK(*target.set.get<vmti::target_location_offset_latitude>() == Approx(10.0));
	CHECK(*target.set.get<vmti::target_location_offset_longitude>() == Approx(12.0));
	CHECK(target.set.get<vmti::target_height_above_ellipsoid>() == 10'000.0);
	CHECK(*target.set.get<vmti::bounding_box_bottom_right_longitude_offset>() == Approx(10.0));
	CHECK(target.set.get<vmti::geospatial_contour_series>().error() == field_error::empty);
	CHECK(target.set.get<vmti::centroid_pixel_row>() == uint32_t{872});
	CHECK(target.set.get<vmti::centroid_pixel_column>() == uint32_t{1137});
	CHECK(target.set.get<vmti::fpa_index_pack>() == vmti::fpa_index{.row = 2, .column = 3});
	CHECK(target.set.get<vmti::target_algorithm_id>() == uint32_t{3});
	CHECK(target.set.get<vmti::target_detection_status>() == vmti::detection_status::active_coasting);

	// Zero-length items are empty, for each format
	CHECK(target.set.get<vmti::target_location>().error() == field_error::empty);
	CHECK(target.set.get<vmti::vtracker_local_set>().error() == field_error::empty);
	CHECK(target.set.get<vmti::vchip_series>().error() == field_error::empty);

	auto const& algorithms = rec->get<vmti::algorithm_series>();
	REQUIRE(algorithms);
	REQUIRE(algorithms->size() == 1);
	CHECK(algorithms->front().get<vmti::algorithm_id>() == uint32_t{9});
	CHECK(algorithms->front().get<vmti::algorithm_name>() == std::string{"k6_yolo_9000_tracker"});
	CHECK(algorithms->front().get<vmti::algorithm_class>() == std::string{"kalmann"});
	CHECK(algorithms->front().get<vmti::algorithm_frame_count>() == uint64_t{10});

	auto const& ontologies = rec->get<vmti::ontology_series>();
	REQUIRE(ontologies);
	REQUIRE(ontologies->size() == 1);
	CHECK(ontologies->front().get<vmti::ontology_id>() == uint64_t{17});
	CHECK(ontologies->front().get<vmti::ontology_parent_id>() == uint64_t{12});
	CHECK(ontologies->front().get<vmti::ontology_label>() == std::string{"label"});

	CHECK(encode_set(*rec) == bytes);
}

TEST_CASE("KLV ST 0903 packs and sets decode examples", "[klv][misb0903]") {
	SECTION("location pack with sigma and rho") {
		auto const bytes = hex("01 02 03 04 10 20 30 40 0A 0B 01 02 03 04 05 06 10 20 30 40 50 60");
		auto const value = vmti::location_codec{}.decode(bytes);
		REQUIRE(value);
		CHECK(value->latitude == Approx(-87.984282970428467));
		CHECK(value->longitude == Approx(-115.49705505371094));
		CHECK(value->height == Approx(1671.0));
		CHECK(value->sigma == vmti::standard_deviations{.east = 8.0625, .north = 24.125, .up = 40.1875});
		CHECK(
		    value->rho
		    == vmti::correlations{.east_north = -0.748046875, .east_up = -0.24609375, .north_up = 0.255859375}
		);

		auto out = byte_vector{};
		auto writer = byte_writer{out};
		REQUIRE(vmti::location_codec{}.encode(*value, writer));
		CHECK(out == bytes);

		auto prefixed = hex("16");
		prefixed.insert(prefixed.end(), bytes.begin(), bytes.end());
		auto const series = codec::series<vmti::location_codec>{}.decode(prefixed);
		REQUIRE(series);
		CHECK(series->size() == 1);
	}

	SECTION("location pack needs sigma before rho") {
		auto out = byte_vector{};
		auto writer = byte_writer{out};
		auto const value = vmti::location{.rho = vmti::correlations{}};
		CHECK(vmti::location_codec{}.encode(value, writer) == std::unexpected(field_error::invalid_value));
	}

	SECTION("VMask") {
		auto const bytes = hex("01 09 02 39 AA 02 39 BF 02 3B 0B 02 0C 03 01 4A 02 03 01 59 04 03 01 6A 02");
		auto const rec = decode_set<vmti::vmask_ls>(bytes);
		REQUIRE(rec);
		CHECK(rec->get<vmti::pixel_contour>() == std::vector<uint64_t>{14'762, 14'783, 15'115});
		auto const runs = std::vector<vmti::pixel_run>{
		    {.index = 74, .length = 2},
		    {.index = 89, .length = 4},
		    {.index = 106, .length = 2},
		};
		CHECK(rec->get<vmti::bitmask_series>() == runs);
		CHECK(encode_set(*rec) == bytes);
	}

	SECTION("VObject and VFeature with 1-byte IMAPB confidence") {
		auto const object = decode_set<vmti::vobject_ls>(hex("01 03 55 52 49 02 05 63 6C 61 73 73 03 01 07 04 01 20"));
		REQUIRE(object);
		CHECK(object->get<vmti::vobject_ontology>() == std::string{"URI"});
		CHECK(object->get<vmti::vobject_ontology_class>() == std::string{"class"});
		CHECK(object->get<vmti::vobject_ontology_id>() == uint32_t{7});
		CHECK(object->get<vmti::vobject_confidence>() == 32.0);

		auto const feature = decode_set<vmti::vfeature_ls>(hex("01 01 41 02 01 42 03 01 09 04 01 63"));
		REQUIRE(feature);
		CHECK(feature->get<vmti::vfeature_ontology_id>() == uint64_t{9});
		CHECK(feature->get<vmti::vfeature_confidence>() == 99.0);
	}

	SECTION("VTracker") {
		auto const bytes = hex(
		    "01 10 F8 1D 4F AE 7D EC 11 D0 A7 65 00 A0 C9 1E 6B F6 02 01 02 03 08 00 03 82 44 30 F6 CE 40 "
		    "04 08 00 03 82 44 30 F6 CE 40 05 00 06 04 74 65 73 74 07 01 32 08 01 1B 09 00 "
		    "0A 06 12 34 23 34 45 45 0B 06 42 34 43 34 45 45 0C 01 03"
		);
		auto const rec = decode_set<vmti::vtracker_ls>(bytes);
		REQUIRE(rec);
		CHECK(codec::uuid{}.to_string(*rec->get<vmti::track_id>()) == "f81d4fae-7dec-11d0-a765-00a0c91e6bf6");
		CHECK(rec->get<vmti::tracker_detection_status>() == vmti::detection_status::dropped);
		CHECK(rec->get<vmti::first_observation_time>() == micros(987'654'321'000'000));
		CHECK(rec->get<vmti::tracker_algorithm>() == std::string{"test"});
		CHECK(rec->get<vmti::number_of_track_points>() == uint16_t{27});
		CHECK(rec->get<vmti::tracker_velocity>() == vmti::motion{.east = -608.75, .north = -336.75, .up = 208.3125});
		CHECK(rec->get<vmti::tracker_acceleration>() == vmti::motion{.east = 159.25, .north = 175.25, .up = 208.3125});
		CHECK(rec->get<vmti::tracker_algorithm_id>() == uint32_t{3});
		CHECK(encode_set(*rec) == bytes);
	}

	SECTION("VChip") {
		auto const bytes = hex("01 04 6A 70 65 67 02 03 49 52 49 03 04 01 02 03 04");
		auto const rec = decode_set<vmti::vchip_ls>(bytes);
		REQUIRE(rec);
		CHECK(rec->get<vmti::chip_image_type>() == std::string{"jpeg"});
		CHECK(rec->get<vmti::chip_embedded_image>() == hex("01 02 03 04"));
		CHECK(encode_set(*rec) == bytes);
	}
}

TEST_CASE("KLV ST 0903 stand-alone packet carries the ST 0601 checksum", "[klv][misb0903]") {
	auto target = record<vmti::vtarget_ls>{};
	target.set<vmti::target_centroid>(409'600);

	auto rec = record<vmti::vmti_ls>{};
	rec.set<vmti::precision_time_stamp>(micros(1'000));
	rec.set<vmti::vtarget_series>({vmti::vtarget{.id = 3, .set = target}});

	auto bytes = encode_packet(rec);
	REQUIRE(bytes);
	auto const decoded = decode_packet<vmti::vmti_ls>(*bytes);
	REQUIRE(decoded);
	CHECK(decoded->get<vmti::vtarget_series>() == rec.get<vmti::vtarget_series>());

	(*bytes)[bytes->size() - 5] ^= std::byte{0x01};
	CHECK(decode_packet<vmti::vmti_ls>(*bytes).error().code == error_code::checksum_mismatch);
}
