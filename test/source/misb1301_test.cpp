// ST 1301.2 gives no example packet. The Core Identifier comes from MISB ST 1204.3 Table 7.

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <cstdint>
#include <expected>
#include <optional>
#include <string>
#include <string_view>

#include "klv_test_util.hpp"
#include "klv/misb1301.hpp"
#include "klv/packet.hpp"

using namespace klv;
using klv_test::hex;

namespace aug = klv::misb1301;
namespace miis = klv::misb1204;

namespace {

/// The Core Identifier of ST 1204 Table 7: a physical Sensor ID and a virtual Platform ID
constexpr auto core_id_bytes = std::string_view{
    "01 70 F5 92 F0 23 73 36 4A F8 AA 91 62 C0 0F 2E B2 DA 16 B7 43 41 00 08 41 A0 BE 36 5B 5A B9 6A 36 45"
};

/// Augmentation Identifiers Local Set packet with the Version and the Core Identifier. The value has 39 bytes.
constexpr auto augmentation_packet = std::string_view{
    "06 0E 2B 34 02 0B 01 01 0E 01 03 05 03 00 00 00 27 "
    "03 01 02 "  // ST 1301.2
    "04 22 "
    "01 70 F5 92 F0 23 73 36 4A F8 AA 91 62 C0 0F 2E B2 DA 16 B7 43 41 00 08 41 A0 BE 36 5B 5A B9 6A 36 45"
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

[[nodiscard]]
auto table_7_id() -> miis::core_id {
	return miis::core_id{
	    .sensor = miis::device_id{.type = miis::id_type::physical, .id = uuid_of("F592F02373364AF8AA9162C00F2EB2DA")},
	    .platform =
	        miis::device_id{.type = miis::id_type::virtual_id, .id = uuid_of("16B74341000841A0BE365B5AB96A3645")},
	    .window = std::nullopt,
	    .minor = std::nullopt,
	};
}

[[nodiscard]]
auto make_augmentation() -> record<aug::augmentation_ls> {
	auto rec = record<aug::augmentation_ls>{};
	rec.set<aug::document_version>(uint32_t{2});
	rec.set<aug::core_identifier>(table_7_id());
	return rec;
}

}  // namespace

TEST_CASE("KLV ST 1301 Augmentation Identifiers packet", "[klv][misb1301]") {
	auto const decoded = decode_packet<aug::augmentation_ls>(hex(augmentation_packet));
	REQUIRE(decoded);
	CHECK(decoded->unknown().empty());
	CHECK(*decoded == make_augmentation());
	CHECK(encode_packet(make_augmentation()) == hex(augmentation_packet));

	auto const text = describe(*decoded);
	auto const entry = [&](std::string_view name) {
		auto const found = std::ranges::find(text, name, &field_text::name);
		REQUIRE(found != text.end());
		return *found;
	};
	CHECK(entry("Version").value == std::string{"2"});
	CHECK(
	    entry("Core Identifier").value
	    == std::string{"0170:F592-F023-7336-4AF8-AA91-62C0-0F2E-B2DA/16B7-4341-0008-41A0-BE36-5B5A-B96A-3645:D3"}
	);
}

TEST_CASE("KLV ST 1301 keeps the augmentation items", "[klv][misb1301]") {
	// The order of ST 1301 Figure 1: the Core Identifier, the Version, then an augmentation item with tag 8. The set
	// also holds the deprecated tag 1.
	auto const bytes = hex(
	    std::string{"06 0E 2B 34 02 0B 01 01 0E 01 03 05 03 00 00 00 30 04 22 "} + std::string{core_id_bytes}
	    + " 03 01 02 08 02 AB CD 01 03 00 01 02"
	);
	auto const decoded = decode_packet<aug::augmentation_ls>(bytes);
	REQUIRE(decoded);
	CHECK(decoded->get<aug::document_version>() == uint32_t{2});
	CHECK(decoded->get<aug::core_identifier>() == table_7_id());
	REQUIRE(decoded->unknown().size() == 2);
	CHECK(decoded->unknown()[0] == unknown_field{.tag = 8, .value = hex("AB CD")});
	CHECK(decoded->unknown()[1] == unknown_field{.tag = 1, .value = hex("00 01 02")});

	// Encoding writes the fields in tag order, then the unknown items in their decoded order
	CHECK(
	    encode_packet(*decoded)
	    == hex(
	        std::string{"06 0E 2B 34 02 0B 01 01 0E 01 03 05 03 00 00 00 30 03 01 02 04 22 "}
	        + std::string{core_id_bytes} + " 08 02 AB CD 01 03 00 01 02"
	    )
	);
}

TEST_CASE("KLV ST 1301 item errors", "[klv][misb1301]") {
	// A usage byte of zero is not a valid Core Identifier. A Version with an extra byte is not a valid BER-OID.
	auto const bytes = hex("06 0E 2B 34 02 0B 01 01 0E 01 03 05 03 00 00 00 08 03 02 02 00 04 02 01 00");
	auto const decoded = decode_packet<aug::augmentation_ls>(bytes);
	REQUIRE(decoded);
	CHECK(decoded->get<aug::document_version>() == std::unexpected(field_error::invalid_value));
	CHECK(decoded->get<aug::core_identifier>() == std::unexpected(field_error::invalid_value));

	// A version of more than 127 uses more than one byte
	CHECK(aug::document_version::format.decode(hex("81 2D")) == uint32_t{173});

	// A Core Identifier with no ID does not encode
	auto rec = record<aug::augmentation_ls>{};
	rec.set<aug::core_identifier>(miis::core_id{});
	CHECK_FALSE(encode_packet(rec));
}
