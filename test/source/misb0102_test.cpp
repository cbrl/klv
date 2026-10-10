// ST 0102.12 gives no example packet. A script made the Universal Set packet. The script uses the keys of Table 1, and
// it checks each key against its CRC.

#include <catch2/catch_test_macros.hpp>

#include <cstddef>
#include <cstdint>
#include <expected>
#include <optional>
#include <string>
#include <string_view>

#include "klv_test_util.hpp"
#include "klv/misb0102.hpp"
#include "klv/packet.hpp"

using namespace klv;
using klv_test::append_field;
using klv_test::append_packet;
using klv_test::hex;

namespace sec = klv::misb0102;

static_assert(local_set_definition<sec::security_ls> && keyed_set_definition<sec::security_ls>);
static_assert(universal_set_definition<sec::security_us> && !has_checksum<sec::security_us>);
static_assert(sec::security_ls::fields::size == 24);
static_assert(sec::security_us::fields::size == 17);
static_assert(sec::parse_classification("TOP SECRET//") == sec::classification::top_secret);
static_assert(sec::universal_text(sec::object_country_coding::genc_admin_sub) == "GENC AdminSub");

namespace {

/// Security Metadata Universal Set packet with the 17 items of ST 0102 Table 1, in table order
constexpr auto universal_packet = std::string_view{
    "06 0E 2B 34 02 01 01 01 02 08 02 00 00 00 00 00 82 01 B2 "
    "06 0E 2B 34 01 01 01 03 02 08 02 01 00 00 00 00 0E 55 4E 43 4C 41 53 53 49 46 49 45 44 2F 2F "  // UNCLASSIFIED//
    "06 0E 2B 34 01 01 01 03 07 01 20 01 02 07 00 00 0F 47 45 4E 43 20 54 77 6F 20 4C 65 74 74 65 72 "
    "06 0E 2B 34 01 01 01 03 07 01 20 01 02 08 00 00 04 2F 2F 55 53 "
    "06 0E 2B 34 01 01 01 01 0E 01 02 03 02 00 00 00 05 2F 2F 48 43 53 "
    "06 0E 2B 34 01 01 01 03 02 08 02 02 00 00 00 00 04 46 4F 55 4F "
    "06 0E 2B 34 01 01 01 03 07 01 20 01 02 09 00 00 05 55 53 20 47 42 "
    "06 0E 2B 34 01 01 01 03 02 08 02 03 00 00 00 00 09 41 6E 61 6C 79 73 74 20 31 "
    "06 0E 2B 34 01 01 01 03 02 08 02 06 00 00 00 00 08 53 6F 75 72 63 65 20 31 "
    "06 0E 2B 34 01 01 01 03 02 08 02 04 00 00 00 00 06 31 2E 34 28 63 29 "
    "06 0E 2B 34 01 01 01 03 02 08 02 05 00 00 00 00 08 32 30 33 30 31 32 33 31 "
    "06 0E 2B 34 01 01 01 03 02 08 02 08 00 00 00 00 05 43 41 50 43 4F "
    "06 0E 2B 34 01 01 01 03 07 01 20 01 02 06 00 00 11 47 45 4E 43 20 54 68 72 65 65 20 4C 65 74 74 65 72 "
    "06 0E 2B 34 01 01 01 03 07 01 20 01 02 01 01 00 0E 00 55 00 53 00 41 00 3B 00 47 00 42 00 52 "  // UTF-16
    "06 0E 2B 34 01 01 01 03 02 08 02 07 00 00 00 00 09 54 65 73 74 20 6F 6E 6C 79 "
    "06 0E 2B 34 01 01 01 01 0E 01 02 05 04 00 00 00 02 00 0C "
    "06 0E 2B 34 01 01 01 01 0E 01 04 03 03 00 00 00 0A 32 30 31 37 2D 30 36 2D 32 32 "
    "06 0E 2B 34 01 01 01 01 0E 01 04 03 04 00 00 00 0A 32 30 31 37 2D 30 36 2D 32 32"
};

/// The Local Set equivalent of the Universal Set packet
[[nodiscard]]
auto make_local() -> record<sec::security_ls> {
	auto local = record<sec::security_ls>{};
	local.set<sec::security_classification>(sec::classification::unclassified);
	local.set<sec::classifying_country_coding_method>(sec::classifying_country_coding::genc_two_letter);
	local.set<sec::classifying_country>("//US");
	local.set<sec::sci_shi_information>("//HCS");
	local.set<sec::caveats>("FOUO");
	local.set<sec::releasing_instructions>("US GB");
	local.set<sec::classified_by>("Analyst 1");
	local.set<sec::derived_from>("Source 1");
	local.set<sec::classification_reason>("1.4(c)");
	local.set<sec::declassification_date>("20301231");
	local.set<sec::classification_marking_system>("CAPCO");
	local.set<sec::object_country_coding_method>(sec::object_country_coding::genc_three_letter);
	local.set<sec::object_country_codes>("USA;GBR");
	local.set<sec::classification_comments>("Test only");
	local.set<sec::version>(uint16_t{12});
	local.set<sec::classifying_country_coding_method_version_date>("2017-06-22");
	local.set<sec::object_country_coding_method_version_date>("2017-06-22");
	return local;
}

}  // namespace

TEST_CASE("KLV ST 0102 Universal Set packet", "[klv][misb0102]") {
	auto const bytes = hex(universal_packet);
	auto const universal = decode_packet<sec::security_us>(bytes);
	REQUIRE(universal);
	CHECK(universal->unknown().empty());
	CHECK(universal->get<sec::security_classification_text>() == std::string{"UNCLASSIFIED//"});
	CHECK(universal->get<sec::object_country_codes>() == std::string{"USA;GBR"});
	CHECK(universal->get<sec::version>() == uint16_t{12});
	CHECK(encode_packet(*universal) == bytes);

	auto const text = describe(*universal);
	REQUIRE(text.size() == 17);
	CHECK(text[0].id == item_id{sec::security_classification_text::key});
	CHECK(text[0].name == "Security Classification");
	CHECK(text[0].value == std::string{"UNCLASSIFIED//"});
	CHECK(text[12].name == "Object Country Codes");
	CHECK(text[12].value == std::string{"USA;GBR"});

	// ST 0102 section 6.8: the conversion to the Local Set and back
	auto const local = sec::to_local(*universal);
	CHECK(local == make_local());
	CHECK(sec::to_universal(local) == *universal);

	// The Local Set writes the Object Country Codes as UTF-16
	auto codes = record<sec::security_ls>{};
	codes.set<sec::object_country_codes>("USA;GBR");
	CHECK(encode_set(codes) == hex("0D 0E 00 55 00 53 00 41 00 3B 00 47 00 42 00 52"));
}

TEST_CASE("KLV ST 0102 Universal Set keeps unknown items", "[klv][misb0102]") {
	constexpr auto unknown_key = ul("06 0E 2B 34 01 01 01 03 02 08 02 7F 00 00 00 00");

	auto value = byte_vector{};
	append_packet(value, unknown_key, hex("AB CD"));
	append_packet(value, sec::security_classification_text::key, hex("53 45 43 52 45 54 2F 2F"));  // SECRET//
	auto bytes = byte_vector{};
	append_packet(bytes, sec::security_us::key, value);

	auto const universal = decode_packet<sec::security_us>(bytes);
	REQUIRE(universal);
	CHECK(universal->get<sec::security_classification_text>() == std::string{"SECRET//"});
	REQUIRE(universal->unknown().size() == 1);
	CHECK(universal->unknown()[0] == unknown_field<universal_key>{.id = unknown_key, .value = hex("AB CD")});

	// Encoding writes the known items first, then the unknown items
	auto expected_value = byte_vector{};
	append_packet(expected_value, sec::security_classification_text::key, hex("53 45 43 52 45 54 2F 2F"));
	append_packet(expected_value, unknown_key, hex("AB CD"));
	auto expected = byte_vector{};
	append_packet(expected, sec::security_us::key, expected_value);
	CHECK(encode_packet(*universal) == expected);

	// The conversion does not copy the unknown items
	CHECK(sec::to_local(*universal).unknown().empty());
}

TEST_CASE("KLV ST 0102 conversion of the text items", "[klv][misb0102]") {
	SECTION("the Universal Set text") {
		CHECK(sec::universal_text(sec::classification::confidential) == "CONFIDENTIAL//");
		CHECK(sec::universal_text(sec::classifying_country_coding::stanag_1059_mixed) == "STANAG 1059 Mixed");
		CHECK(sec::universal_text(sec::classifying_country_coding::omitted_value_08).empty());
		CHECK(sec::universal_text(sec::object_country_coding::omitted_value_0a).empty());
		CHECK(sec::universal_text(static_cast<sec::classification>(9)).empty());
	}

	SECTION("parsing ignores the case of ASCII letters") {
		CHECK(sec::parse_classification("secret//") == sec::classification::secret);
		CHECK(sec::parse_classification("SECRET") == std::nullopt);
		CHECK(
		    sec::parse_classifying_country_coding("ISO 3166 MIXED") == sec::classifying_country_coding::iso_3166_mixed
		);
		CHECK(sec::parse_classifying_country_coding("Omitted Value") == std::nullopt);
		CHECK(sec::parse_object_country_coding("GENC AdminSub") == sec::object_country_coding::genc_admin_sub);
		CHECK(sec::parse_object_country_coding("GENC Mixed") == std::nullopt);
	}

	SECTION("a text without a value gives invalid_value") {
		auto universal = record<sec::security_us>{};
		universal.set<sec::security_classification_text>("SECRET");
		universal.set<sec::object_country_coding_method_text>("genc two letter");
		universal.set_empty<sec::classifying_country_coding_method_text>();

		auto const local = sec::to_local(universal);
		CHECK(local.get<sec::security_classification>() == std::unexpected(field_error::invalid_value));
		CHECK(local.get<sec::classifying_country_coding_method>() == std::unexpected(field_error::empty));
		CHECK(local.get<sec::object_country_coding_method>() == sec::object_country_coding::genc_two_letter);
		CHECK_FALSE(local.has<sec::caveats>());
	}

	SECTION("an Omitted Value gives invalid_value") {
		auto local = record<sec::security_ls>{};
		local.set<sec::classifying_country_coding_method>(sec::classifying_country_coding::omitted_value_09);
		local.set<sec::stream_id>(uint8_t{0xE0});

		auto const universal = sec::to_universal(local);
		CHECK(
		    universal.get<sec::classifying_country_coding_method_text>()
		    == std::unexpected(field_error::invalid_value)
		);
		CHECK(universal.get<sec::security_classification_text>() == std::unexpected(field_error::absent));
		CHECK(sec::to_universal(record<sec::security_ls>{}) == record<sec::security_us>{});
	}
}

TEST_CASE("KLV ST 0102 Local Set packet", "[klv][misb0102]") {
	auto const bytes = hex(
	    "06 0E 2B 34 02 03 01 01 0E 01 03 03 02 00 00 00 16 "
	    "01 01 01 02 01 0D 03 04 2F 2F 55 53 0D 04 00 55 00 53 16 02 00 0C"
	);
	auto const local = decode_packet<sec::security_ls>(bytes);
	REQUIRE(local);
	CHECK(local->get<sec::security_classification>() == sec::classification::unclassified);
	CHECK(local->get<sec::classifying_country_coding_method>() == sec::classifying_country_coding::genc_two_letter);
	CHECK(local->get<sec::classifying_country>() == std::string{"//US"});
	CHECK(local->get<sec::object_country_codes>() == std::string{"US"});
	CHECK(local->get<sec::version>() == uint16_t{12});
	CHECK(encode_packet(*local) == bytes);

	// Both kinds of set in one buffer
	auto buffer = hex(universal_packet);
	buffer.insert(buffer.end(), bytes.begin(), bytes.end());
	auto count = 0;
	for (auto&& one : decode_packets<sec::security_us, sec::security_ls>(buffer)) {
		REQUIRE(one);
		CHECK(one->index() == static_cast<size_t>(count));
		++count;
	}
	CHECK(count == 2);
}

TEST_CASE("KLV ST 0102 Object Country Codes accept UTF-16 and 8-bit text", "[klv][misb0102]") {
	auto security = byte_vector{};
	SECTION("UTF-16 with a byte order mark") {
		append_field(security, 13, hex("FF FE 55 00 53 00 41 00"));
	}
	SECTION("8-bit text of an older producer") {
		append_field(security, 13, hex("55 53 41"));
	}
	auto const rec = decode_set<sec::security_ls>(security);
	REQUIRE(rec);
	CHECK(rec->get<sec::object_country_codes>() == std::string{"USA"});
}

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
