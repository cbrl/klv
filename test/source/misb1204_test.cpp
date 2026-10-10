// The examples come from MISB ST 1204.3 (Table 7 and Appendix E).

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <array>
#include <cstdint>
#include <expected>
#include <format>
#include <optional>
#include <string>
#include <string_view>

#include "klv_test_util.hpp"
#include "klv/misb1204.hpp"
#include "klv/packet.hpp"

using namespace klv;
using klv_test::hex;

namespace miis = klv::misb1204;

namespace {

constexpr auto format = miis::core_id_codec{};

/// A standalone Core Identifier KLV item and its text format
struct example {
	std::string_view klv;
	std::string_view text;
};

constexpr auto examples = std::array{
    // Table 7: a physical Sensor ID and a virtual Platform ID
    example{
        "060E2B3401010101 0E01040503000000 220170F592F02373 364AF8AA9162C00F 2EB2DA16B7434100 0841A0BE365B5AB9 "
        "6A3645",
        "0170:F592-F023-7336-4AF8-AA91-62C0-0F2E-B2DA/16B7-4341-0008-41A0-BE36-5B5A-B96A-3645:D3",
    },
    // E1: Sensor ID, Platform ID, and Window ID
    example{
        "060E2B3401010101 0E01040503000000 320154C7D1625398 A241C2BA6E90F8FC C73914E047AB3E81 BE41ED966409B02F "
        "445FAB5E71B0DC20 FE4920821626D64F 61D863",
        "0154:C7D1-6253-98A2-41C2-BA6E-90F8-FCC7-3914/E047-AB3E-81BE-41ED-9664-09B0-2F44-5FAB/"
        "5E71-B0DC-20FE-4920-8216-26D6-4F61-D863:C8",
    },
    // E2: Sensor ID and Platform ID
    example{
        "060E2B3401010101 0E01040503000000 22015008CE252ED0 FA49E3B1A065E61D 5720DCEAF18A27A0 864019A586EAAF97 "
        "157BFA",
        "0150:08CE-252E-D0FA-49E3-B1A0-65E6-1D57-20DC/EAF1-8A27-A086-4019-A586-EAAF-9715-7BFA:DA",
    },
    // E3: a physical Sensor ID and a managed Platform ID
    example{
        "060E2B3401010101 0E01040503000000 220168F354666ED5 524C0D9168A7454C EBA073840A4799BB C04BD597A756F640 "
        "92AF7B",
        "0168:F354-666E-D552-4C0D-9168-A745-4CEB-A073/840A-4799-BBC0-4BD5-97A7-56F6-4092-AF7B:AA",
    },
    // E4: a physical Sensor ID and a physical Platform ID
    example{
        "060E2B3401010101 0E01040503000000 220178865EFD9CEF 8A41C38244B885AF CC40BFED8A9AB872 E2416599797E5AF5 "
        "4A5B9A",
        "0178:865E-FD9C-EF8A-41C3-8244-B885-AFCC-40BF/ED8A-9AB8-72E2-4165-9979-7E5A-F54A-5B9A:25",
    },
    // E5: Sensor ID and Window ID
    example{
        "060E2B3401010101 0E01040503000000 220144340F463BAE C24F8CBD4592AEDE 80E1C6F0AF86733A 17424CB0603EE4A8 "
        "6B38F9",
        "0144:340F-463B-AEC2-4F8C-BD45-92AE-DE80-E1C6/F0AF-8673-3A17-424C-B060-3EE4-A86B-38F9:D9",
    },
    // E6: Sensor ID
    example{
        "060E2B3401010101 0E01040503000000 120140BC76CFEF0B EE41A49618EB4B01 0D2F08",
        "0140:BC76-CFEF-0BEE-41A4-9618-EB4B-010D-2F08:B6",
    },
    // E7: Platform ID and Window ID
    example{
        "060E2B3401010101 0E01040503000000 2201143D502DB64A 9344C3B56E94AD7C 4EE47645E28FAFC2 D34A6E815B5FE6B0 "
        "A96ABD",
        "0114:3D50-2DB6-4A93-44C3-B56E-94AD-7C4E-E476/45E2-8FAF-C2D3-4A6E-815B-5FE6-B0A9-6ABD:F1",
    },
    // E8: Platform ID
    example{
        "060E2B3401010101 0E01040503000000 1201101AB8231E17 E84748A133CE9389 A7A060",
        "0110:1AB8-231E-17E8-4748-A133-CE93-89A7-A060:25",
    },
    // E9: Window ID
    example{
        "060E2B3401010101 0E01040503000000 120104C2A7D72496 F747DBA23DA29730 075876",
        "0104:C2A7-D724-96F7-47DB-A23D-A297-3007-5876:55",
    },
    // E10: Minor ID
    example{
        "060E2B3401010101 0E01040503000000 12010203DD9DEEFB 48477B8204B0506F 6B2A33",
        "0102:03DD-9DEE-FB48-477B-8204-B050-6F6B-2A33:25",
    },
};

/// A UUID from the 32 hex digits of its text
[[nodiscard]]
auto uuid_of(std::string_view text) -> klv::uuid {
	auto const bytes = hex(text);
	auto id = klv::uuid{};
	REQUIRE(bytes.size() == id.size());
	std::ranges::copy(bytes, id.begin());
	return id;
}

}  // namespace

TEST_CASE("KLV ST 1204 examples decode, encode, and convert to text", "[klv][misb1204]") {
	for (auto const& item : examples) {
		INFO(item.text);
		auto const bytes = hex(item.klv);

		auto const decoded = decode_packet<miis::core_id_item>(bytes);
		REQUIRE(decoded);
		CHECK(format.to_string(decoded->value) == item.text);
		CHECK(miis::parse_core_id(item.text) == decoded->value);
		CHECK(encode_packet(*decoded) == bytes);
	}
}

TEST_CASE("KLV ST 1204 decodes the ID types and the IDs", "[klv][misb1204]") {
	auto const foundational =
	    format.decode(hex("0170" "F592F02373364AF8AA9162C00F2EB2DA" "16B74341000841A0BE365B5AB96A3645"));
	REQUIRE(foundational);
	CHECK(
	    foundational->sensor
	    == miis::device_id{.type = miis::id_type::physical, .id = uuid_of("F592F02373364AF8AA9162C00F2EB2DA")}
	);
	CHECK(
	    foundational->platform
	    == miis::device_id{.type = miis::id_type::virtual_id, .id = uuid_of("16B74341000841A0BE365B5AB96A3645")}
	);
	CHECK_FALSE(foundational->window);
	CHECK_FALSE(foundational->minor);

	auto const managed = miis::parse_core_id(examples[3].text);
	REQUIRE(managed);
	REQUIRE(managed->platform);
	CHECK(managed->platform->type == miis::id_type::managed);
	CHECK(miis::to_string(managed->platform->type) == "Managed");

	auto const minor = miis::parse_core_id(examples[10].text);
	REQUIRE(minor);
	CHECK(*minor == miis::core_id{.minor = uuid_of("03DD9DEEFB48477B8204B0506F6B2A33")});
}

TEST_CASE("KLV ST 1204 check value", "[klv][misb1204]") {
	// Appendix B, Table 15
	STATIC_CHECK(miis::check_value("031FA3") == 0x79);
	CHECK(miis::check_value("031fa3") == 0x79);

	// Separators do not count
	CHECK(miis::check_value("0140:BC76-CFEF-0BEE-41A4-9618-EB4B-010D-2F08") == 0xB6);
}

TEST_CASE("KLV ST 1204 rejects malformed binary values", "[klv][misb1204]") {
	auto const id = std::string{"F592F02373364AF8AA9162C00F2EB2DA"};

	// Reserved bit 7 or bit 0 set, a usage byte of zero, and a version other than 1
	CHECK(format.decode(hex("01C0" + id)) == std::unexpected(field_error::invalid_value));
	CHECK(format.decode(hex("0141" + id)) == std::unexpected(field_error::invalid_value));
	CHECK(format.decode(hex("0100")) == std::unexpected(field_error::invalid_value));
	CHECK(format.decode(hex("0240" + id)) == std::unexpected(field_error::invalid_value));

	// A Minor ID together with a Sensor ID
	CHECK(format.decode(hex("0142" + id + id)) == std::unexpected(field_error::invalid_value));

	// The length does not match the usage byte
	CHECK(format.decode(hex("0170" + id)) == std::unexpected(field_error::bad_length));
	CHECK(format.decode(hex("0140" + id + "00")) == std::unexpected(field_error::bad_length));
	CHECK(format.decode(hex("01")) == std::unexpected(field_error::bad_length));
}

TEST_CASE("KLV ST 1204 encoding needs a valid Core Identifier", "[klv][misb1204]") {
	auto const id = uuid_of("F592F02373364AF8AA9162C00F2EB2DA");
	auto const sensor = miis::device_id{.type = miis::id_type::physical, .id = id};

	CHECK(encode_value(format, miis::core_id{}) == std::unexpected(field_error::invalid_value));
	CHECK(
	    encode_value(format, miis::core_id{.sensor = sensor, .minor = id})
	    == std::unexpected(field_error::invalid_value)
	);
	CHECK(
	    encode_value(format, miis::core_id{.sensor = miis::device_id{.type = miis::id_type{0}, .id = id}})
	    == std::unexpected(field_error::invalid_value)
	);
	CHECK(encode_value(format, miis::core_id{.window = id}) == hex("0104" "F592F02373364AF8AA9162C00F2EB2DA"));
}

TEST_CASE("KLV ST 1204 text format", "[klv][misb1204]") {
	// Lower case hex digits are valid
	auto const lower = std::string{"0140:bc76-cfef-0bee-41a4-9618-eb4b-010d-2f08:b6"};
	auto const value = miis::parse_core_id(lower);
	REQUIRE(value);
	CHECK(format.to_string(*value) == "0140:BC76-CFEF-0BEE-41A4-9618-EB4B-010D-2F08:B6");

	// A wrong check value
	CHECK_FALSE(miis::parse_core_id("0140:BC76-CFEF-0BEE-41A4-9618-EB4B-010D-2F08:B7"));

	// Text in another form
	CHECK_FALSE(miis::parse_core_id("0140:BC76-CFEF-0BEE-41A4-9618-EB4B-010D-2F08"));
	CHECK_FALSE(miis::parse_core_id("0140:BC76CFEF-0BEE-41A4-9618-EB4B-010D-2F08:B6"));
	CHECK_FALSE(miis::parse_core_id("0140:BC76-CFEF-0BEE-41A4-9618-EB4B-010D-2F08/:B6"));
	CHECK_FALSE(miis::parse_core_id("140:BC76-CFEF-0BEE-41A4-9618-EB4B-010D-2F08:B6"));

	// The usage byte names two IDs, but the text holds one. The check value is correct.
	auto const one_id = std::string{"0170:F592-F023-7336-4AF8-AA91-62C0-0F2E-B2DA"};
	CHECK_FALSE(miis::parse_core_id(std::format("{}:{:02X}", one_id, miis::check_value(one_id))));
}
