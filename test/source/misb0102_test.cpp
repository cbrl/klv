#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <string>

#include "klv_test_util.hpp"
#include "klv/misb0102.hpp"

using namespace klv;
using klv_test::append_field;
using klv_test::hex;

namespace sec = klv::misb0102;

static_assert(local_set_definition<sec::security_ls>);
static_assert(!universal_set_definition<sec::security_ls>);
static_assert(sec::security_ls::fields::size == 24);

TEST_CASE("KLV ST 0102 decodes the stream identifiers and version dates", "[klv][misb0102]") {
	auto security = byte_vector{};
	append_field(security, 1, hex("01"));
	append_field(security, 19, hex("E0"));
	append_field(security, 20, hex("00 01"));
	append_field(security, 21, hex("00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F"));
	append_field(security, 22, hex("00 0C"));
	append_field(security, 23, hex("32 30 31 39 2D 30 31 2D 30 31"));  // "2019-01-01"
	append_field(security, 24, hex("32 30 32 30 2D 30 36 2D 33 30"));  // "2020-06-30"

	auto const rec = decode_set<sec::security_ls>(security);
	REQUIRE(rec);
	CHECK(rec->get<sec::stream_id>() == uint8_t{0xE0});
	CHECK(rec->get<sec::transport_stream_id>() == uint16_t{1});
	CHECK(rec->get<sec::item_designator_id>() == hex("00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F"));
	CHECK(rec->get<sec::classifying_country_coding_method_version_date>() == std::string{"2019-01-01"});
	CHECK(rec->get<sec::object_country_coding_method_version_date>() == std::string{"2020-06-30"});
	CHECK(rec->unknown().empty());
}
