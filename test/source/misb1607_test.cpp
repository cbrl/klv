#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <concepts>
#include <cstddef>
#include <string>
#include <vector>

#include "klv_test_util.hpp"
#include "klv/misb0601.hpp"
#include "klv/misb1607.hpp"

using namespace klv;
using klv_test::hex;
using klv_test::micros;

namespace kv = klv::misb0601;

static_assert(std::same_as<value_t<kv::segment_local_set>, std::vector<record<kv::uas_datalink_ls>>>);
static_assert(std::same_as<value_t<kv::amend_local_set>, std::vector<record<kv::uas_datalink_ls>>>);

TEST_CASE("KLV ST 0601 decodes ST 1607 Segment and Amend sets", "[klv][misb0601][misb1607]") {
	auto const bytes = hex(
	    "02 08 00 00 00 00 00 00 12 34 41 01 11 03 05 41 4C 50 48 41 "
	    "65 07 03 05 42 52 41 56 4F "
	    "64 07 46 05 4C 41 52 52 59 "
	    "64 0C 46 06 4A 4F 48 4E 4E 59 3E 02 04 57"
	);

	auto const rec = decode_set<kv::uas_datalink_ls>(bytes);
	REQUIRE(rec);
	CHECK(rec->get<kv::mission_id>() == std::string{"ALPHA"});

	auto const& amends = rec->get<kv::amend_local_set>();
	REQUIRE(amends);
	REQUIRE(amends->size() == 1);
	CHECK(amends->front().get<kv::mission_id>() == std::string{"BRAVO"});

	auto const& segments = rec->get<kv::segment_local_set>();
	REQUIRE(segments);
	REQUIRE(segments->size() == 2);
	CHECK(segments->front().get<kv::alternate_platform_name>() == std::string{"LARRY"});
	CHECK((*segments)[1].get<kv::alternate_platform_name>() == std::string{"JOHNNY"});
	CHECK((*segments)[1].get<kv::laser_prf_code>() == uint16_t{1111});

	SECTION("merge applies a child set to its parent") {
		auto const second = misb1607::merge(*rec, (*segments)[1]);
		CHECK(second.get<kv::mission_id>() == std::string{"ALPHA"});
		CHECK(second.get<kv::alternate_platform_name>() == std::string{"JOHNNY"});
		CHECK(second.get<kv::laser_prf_code>() == uint16_t{1111});

		auto const amended = misb1607::merge(*rec, amends->front());
		CHECK(amended.get<kv::mission_id>() == std::string{"BRAVO"});
		CHECK(amended.get<kv::precision_time_stamp>() == micros(0x1234));

		// The child sets of the parent are the merged child and its siblings. They do not apply to the result.
		CHECK(second.get<kv::segment_local_set>().error() == field_error::absent);
		CHECK(second.get<kv::amend_local_set>().error() == field_error::absent);
		CHECK(amended.get<kv::segment_local_set>().error() == field_error::absent);
		CHECK(amended.get<kv::amend_local_set>().error() == field_error::absent);
	}

	SECTION("describe lists the fields of each child set") {
		auto names = std::vector<std::string>{};
		for (auto const& entry : describe(*rec)) {
			names.push_back(entry.name);
		}
		CHECK(std::ranges::contains(names, "Segment Local Set[1].Laser PRF Code"));
		CHECK(std::ranges::contains(names, "Amend Local Set[0].Mission ID"));
	}

	SECTION("a child set can hold other child sets") {
		auto nested = byte_vector{};
		klv_test::append_field(nested, 101, hex("03 01 41"));
		auto outer = byte_vector{};
		klv_test::append_field(outer, 100, nested);

		auto const with_nested = decode_set<kv::uas_datalink_ls>(outer);
		REQUIRE(with_nested);
		auto const& child = with_nested->get<kv::segment_local_set>();
		REQUIRE(child);
		CHECK(child->front().unknown().empty());
		auto const& amend = child->front().get<kv::amend_local_set>();
		REQUIRE(amend);
		CHECK(amend->front().get<kv::mission_id>() == std::string{"A"});
		CHECK(encode_set(*with_nested) == outer);

		// The merged segment keeps its own Amend LS, and the next merge applies it
		auto const segment = misb1607::merge(*with_nested, child->front());
		CHECK(segment.get<kv::segment_local_set>().error() == field_error::absent);
		REQUIRE(segment.get<kv::amend_local_set>());
		CHECK(segment.get<kv::amend_local_set>()->size() == 1);

		auto const amended = misb1607::merge(segment, segment.get<kv::amend_local_set>()->front());
		CHECK(amended.get<kv::mission_id>() == std::string{"A"});
		CHECK(amended.get<kv::amend_local_set>().error() == field_error::absent);
	}
}

TEST_CASE("KLV ST 0601 child sets stop at the nesting limit", "[klv][misb0601][misb1607]") {
	// Each level is a Segment LS that holds the next level
	auto bytes = hex("03 01 41");
	for (auto level = size_t{0}; level < max_nesting_depth + 2; ++level) {
		auto outer = byte_vector{};
		klv_test::append_field(outer, 100, bytes);
		bytes = outer;
	}

	auto const rec = decode_set<kv::uas_datalink_ls>(bytes);
	REQUIRE(rec);

	auto const* level = &*rec;
	auto depth = size_t{0};
	while (level->has<kv::segment_local_set>()) {
		level = &level->get<kv::segment_local_set>()->front();
		++depth;
	}
	CHECK(depth == max_nesting_depth);
	CHECK(level->get<kv::segment_local_set>().error() == field_error::invalid_value);
}

TEST_CASE("KLV ST 0601 Amend LS clears a parent value with a zero-length item", "[klv][misb0601][misb1607]") {
	// Mission ID ALPHA, then an Amend LS with a zero-length Mission ID and a new Platform Tail Number
	auto const bytes = hex("03 05 41 4C 50 48 41 65 05 03 00 04 01 42");

	auto const rec = decode_set<kv::uas_datalink_ls>(bytes);
	REQUIRE(rec);
	auto const& amend = rec->get<kv::amend_local_set>();
	REQUIRE(amend);
	CHECK(amend->front().get<kv::mission_id>().error() == field_error::empty);
	CHECK(encode_set(*rec) == bytes);

	auto const amended = misb1607::merge(*rec, amend->front());
	CHECK(amended.get<kv::mission_id>().error() == field_error::empty);
	CHECK(amended.get<kv::platform_tail_number>() == std::string{"B"});
}

TEST_CASE("KLV ST 0601 Amend LS field that does not decode keeps the parent value", "[klv][misb0601][misb1607]") {
	// Sensor Latitude 0x40000000, then an Amend LS with a 2-byte Sensor Latitude (the format needs 4 bytes)
	auto const bytes = hex("0D 04 40 00 00 00 65 04 0D 02 00 00");

	auto const rec = decode_set<kv::uas_datalink_ls>(bytes);
	REQUIRE(rec);
	auto const& amend = rec->get<kv::amend_local_set>();
	REQUIRE(amend);
	CHECK(amend->front().get<kv::sensor_latitude>().error() == field_error::bad_length);

	auto const amended = misb1607::merge(*rec, amend->front());
	REQUIRE(amended.get<kv::sensor_latitude>());
	CHECK(*amended.get<kv::sensor_latitude>() == Catch::Approx(45.0));
}
