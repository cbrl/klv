// ST 1504.1 gives no example packet. A separate script calculated the bytes and the CRC of the example packet below.

#include <catch2/catch_test_macros.hpp>

#include <array>
#include <cstdint>
#include <expected>
#include <string_view>
#include <utility>
#include <vector>

#include "klv_test_util.hpp"
#include "klv/misb1504.hpp"
#include "klv/packet.hpp"

using namespace klv;
using klv_test::hex;
using klv_test::micros;

namespace sv = klv::misb1504;

namespace {

/// State Vector Local Set packet with all 15 items. The SDCC-FLP covers the State Vector Pack and the TROC Pack.
constexpr auto orbit_packet = std::string_view{
    "06 0E 2B 34 02 0B 01 01 0E 01 03 01 03 00 00 00 82 01 1B "
    "01 08 00 06 0A 24 18 20 22 40 "  // 2023-11-14T22:13:20.123456Z
    "02 01 01 "  // ST 1504.1
    "05 01 03 "  // Mean Anomaly at Epoch
    "06 08 00 06 0A 24 14 8A B9 00 "  // 2023-11-14T22:12:20Z
    "08 0C 3A 00 00 00 00 00 00 00 B9 00 00 00 "  // Eccentricity vector: 4-byte values
    // Node vector and specific angular momentum: 8-byte values
    "09 18 BF E0 00 00 00 00 00 00 3F EB B6 7A E8 58 4C AA 00 00 00 00 00 00 00 00 "
    "0A 18 42 07 48 76 E8 00 00 00 C2 17 48 76 E8 00 00 00 42 26 1E 70 F6 00 00 00 "
    "0B 08 3F B0 55 32 61 7C 1B DA "  // 0.0638 deg/s
    "0C 08 42 F6 A8 66 5B DA 54 00 "  // 3.986004418e14 m^3/s^2
    "0D 04 4A CE DA 2E "  // 6778135 m
    "0E 08 C1 7C 0A 6A 18 00 00 00 "  // -29402785.5 J/kg
    // State Vector Pack: J2000 ECI, Keplerian elements, 8-byte values
    "04 31 21 41 59 DB 46 40 00 00 00 3F 40 62 4D D2 F1 A9 FC 40 49 D1 EB 85 1E B8 52 "
    "40 3E 00 00 00 00 00 00 40 5E 20 00 00 00 00 00 40 46 A0 00 00 00 00 00 "
    "07 18 00 00 00 00 00 00 00 00 00 00 00 00 39 00 00 00 B7 80 00 00 3D 80 00 00 "  // TROC: 4-byte values
    // SDCC-FLP: 12 rows, Mode 2, sparse, 4-byte IEEE 754 values, 3 correlation coefficients that are not zero
    "0F 48 0C A4 04 80 10 00 00 00 00 00 00 40 "
    "41 20 00 00 3E 80 00 00 3F 00 00 00 3E 00 00 00 3F 40 00 00 3F C0 00 00 "
    "3D 80 00 00 3D 00 00 00 3D 00 00 00 3C 80 00 00 3C 80 00 00 3C 00 00 00 "
    "3F 00 00 00 BE 80 00 00 3F 40 00 00 "
    "03 02 09 6D"
};

[[nodiscard]]
auto make_sdcc() -> klv::sdcc_flp {
	auto members = std::vector<uint32_t>(6, sv::state_vector_pack::tag);
	members.insert(members.end(), 6, sv::state_vector_troc_pack::tag);
	auto rho = std::vector<double>(66, 0.0);
	rho[0] = 0.5;
	rho[11] = -0.25;
	rho[65] = 0.75;
	return klv::sdcc_flp{
	    .size = 12,
	    .members = members,
	    .sigma = {10.0, 0.25, 0.5, 0.125, 0.75, 1.5, 0.0625, 0.03125, 0.03125, 0.015625, 0.015625, 0.0078125},
	    .sigma_raw = {},
	    .rho = rho,
	    .sigma_length = 4,
	    .rho_length = 4,
	    .sparse = true,
	    .long_parse_control = true,
	    .sigma_imapb = false,
	    .rho_imapb = false,
	};
}

[[nodiscard]]
auto make_orbit() -> record<sv::state_vector_ls> {
	auto rec = record<sv::state_vector_ls>{};
	rec.set<sv::precision_time_stamp>(micros(1'700'000'000'123'456));
	rec.set<sv::document_version>(uint32_t{1});
	rec.set<sv::state_vector_pack>({
	    .frame = sv::reference_frame::j2000_eci,
	    .kind = sv::state_vector_kind::keplerian,
	    .elements = {6'778'137.0, 0.0005, 51.64, 30.0, 120.5, 45.25},
	});
	rec.set<sv::anomaly_type>(sv::anomaly_kind::mean_anomaly_at_epoch);
	rec.set<sv::mean_anomaly_epoch>(micros(1'699'999'940'000'000));
	rec.set<sv::state_vector_troc_pack>({0.0, 0.0, 0.0, 0.0001220703125, -0.0000152587890625, 0.0625});
	rec.set<sv::eccentricity_vector_pack>({0.00048828125, 0.0, -0.0001220703125});
	rec.set<sv::node_vector_pack>({-0.5, 0.8660254037844386, 0.0});
	rec.set<sv::specific_angular_momentum_pack>({12'500'000'000.0, -25'000'000'000.0, 47'500'000'000.0});
	rec.set<sv::mean_motion>(0.0638);
	rec.set<sv::mu>(398'600'441'800'000.0);
	rec.set<sv::semilatus_rectum>(6'778'135.0);
	rec.set<sv::specific_energy>(-29'402'785.5);
	rec.set<sv::sdcc_flp>(make_sdcc());
	return rec;
}

/// SDCC-FLP of 6 rows with standard deviations only
[[nodiscard]]
auto six_rows(std::vector<uint32_t> members) -> klv::sdcc_flp {
	return klv::sdcc_flp{
	    .size = 6,
	    .members = std::move(members),
	    .sigma = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0},
	    .sigma_raw = {},
	    .rho = {},
	    .sigma_length = 4,
	    .rho_length = 0,
	    .sparse = false,
	    .long_parse_control = true,
	    .sigma_imapb = false,
	    .rho_imapb = false,
	};
}

/// Tags of the items of a local set value, in order
[[nodiscard]]
auto tags_of(byte_span value) -> std::vector<uint32_t> {
	auto reader = byte_reader{value};
	auto tags = std::vector<uint32_t>{};
	while (!reader.empty()) {
		auto const tag = reader.read_ber_oid();
		auto const length = reader.read_ber_length();
		if (!tag || !length || !reader.take(*length)) {
			return {};
		}
		tags.push_back(*tag);
	}
	return tags;
}

template<typename Codec>
[[nodiscard]]
auto encode(Codec const& codec, typename Codec::value_type const& value) -> field<byte_vector> {
	auto out = byte_vector{};
	auto writer = byte_writer{out};
	return codec.encode(value, writer).transform([&] { return out; });
}

}  // namespace

TEST_CASE("KLV ST 1504 State Vector Local Set packet", "[klv][misb1504]") {
	auto const decoded = decode_packet<sv::state_vector_ls>(hex(orbit_packet));
	REQUIRE(decoded);
	CHECK(decoded->unknown().empty());
	CHECK(decoded->get<sv::crc>() == uint16_t{0x096D});

	auto expected = make_orbit();
	expected.set<sv::crc>(uint16_t{0x096D});
	CHECK(*decoded == expected);
	CHECK(encode_packet(make_orbit()) == hex(orbit_packet));

	CHECK(
	    sv::state_vector_pack::format.to_string(*decoded->get<sv::state_vector_pack>())
	    == "{frame: J2000 ECI, kind: Keplerian Orbital Elements, elements: [6778137, 5e-04, 51.64, 30, 120.5, 45.25]}"
	);

	auto corrupt = hex(orbit_packet);
	corrupt[28] ^= std::byte{0x01};
	CHECK_FALSE(decode_packet<sv::state_vector_ls>(corrupt));
}

TEST_CASE("KLV ST 1504 set inside another set has no CRC", "[klv][misb1504]") {
	auto const bytes = encode_set(make_orbit());
	REQUIRE(bytes);
	CHECK(tags_of(*bytes) == std::vector<uint32_t>{1, 2, 5, 6, 8, 9, 10, 11, 12, 13, 14, 4, 7, 15});

	auto const decoded = decode_set<sv::state_vector_ls>(*bytes);
	REQUIRE(decoded);
	CHECK(*decoded == make_orbit());
}

TEST_CASE("KLV ST 1504 SDCC-FLP covers the packs directly before it", "[klv][misb1504]") {
	auto rec = record<sv::state_vector_ls>{};
	rec.set<sv::state_vector_pack>({
	    .frame = sv::reference_frame::wgs84_ecef,
	    .kind = sv::state_vector_kind::equinoctial,
	    .elements = {7'000'000.0, 0.25, 0.5, 90.0, 0.125, 0.0625},
	});
	rec.set<sv::state_vector_troc_pack>({0.0, 0.0, 0.0, 0.5, 0.0, 0.0});
	rec.set<sv::eccentricity_vector_pack>({0.25, 0.5, 0.0});

	// Without members, 6 rows cover the State Vector Pack
	rec.set<sv::sdcc_flp>(six_rows({}));
	auto bytes = encode_set(rec);
	REQUIRE(bytes);
	CHECK(tags_of(*bytes) == std::vector<uint32_t>{7, 8, 4, 15});
	auto decoded = decode_set<sv::state_vector_ls>(*bytes);
	REQUIRE(decoded);
	CHECK(decoded->get<sv::sdcc_flp>() == six_rows(std::vector<uint32_t>(6, sv::state_vector_pack::tag)));

	// The members select the TROC Pack
	rec.set<sv::sdcc_flp>(six_rows(std::vector<uint32_t>(6, sv::state_vector_troc_pack::tag)));
	bytes = encode_set(rec);
	REQUIRE(bytes);
	CHECK(tags_of(*bytes) == std::vector<uint32_t>{4, 8, 7, 15});
	decoded = decode_set<sv::state_vector_ls>(*bytes);
	REQUIRE(decoded);
	CHECK(*decoded == rec);

	// The members name a pack for each group of 6 rows
	rec.set<sv::sdcc_flp>(six_rows({4, 4, 4, 4, 4, 7}));
	CHECK_FALSE(encode_set(rec));
	rec.set<sv::sdcc_flp>(six_rows(std::vector<uint32_t>(6, sv::eccentricity_vector_pack::tag)));
	CHECK_FALSE(encode_set(rec));

	// 6 rows after an item that is not a covered pack, and 12 rows after one pack
	auto const sdcc = hex("0F 1B 06 80 04 3F 80 00 00 40 00 00 00 40 40 00 00 40 80 00 00 40 A0 00 00 40 C0 00 00");
	auto const eccentricity = hex("08 0C 3E 80 00 00 3F 00 00 00 00 00 00 00");
	auto after_vector = eccentricity;
	after_vector.insert(after_vector.end(), sdcc.begin(), sdcc.end());
	decoded = decode_set<sv::state_vector_ls>(after_vector);
	REQUIRE(decoded);
	CHECK(decoded->get<sv::sdcc_flp>() == std::unexpected(field_error::invalid_value));

	CHECK(sv::sdcc_flp::format.decode(hex("05 80 04 3F 80 00 00 40 00 00 00 40 40 00 00 40 80 00 00 40 A0 00 00"))
	      == std::unexpected(field_error::invalid_value));
	auto const one_pack = std::array<uint32_t, 2>{2, 7};
	auto twelve = six_rows({});
	twelve.size = 12;
	twelve.sigma.insert(twelve.sigma.end(), {7.0, 8.0, 9.0, 10.0, 11.0, 12.0});
	auto const twelve_bytes = encode(sv::sdcc_flp::format, twelve);
	REQUIRE(twelve_bytes);
	CHECK(sv::sdcc_flp::format.decode(*twelve_bytes, one_pack) == std::unexpected(field_error::invalid_value));
}

TEST_CASE("KLV ST 1504 State Vector Pack", "[klv][misb1504]") {
	constexpr auto format = sv::state_vector_codec{};
	auto const elements = hex("3F 80 00 00 3F 00 00 00 40 00 00 00 40 80 00 00 41 00 00 00 41 80 00 00");
	auto const pack = [&](std::string_view status) {
		auto bytes = hex(status);
		bytes.insert(bytes.end(), elements.begin(), elements.end());
		return bytes;
	};
	auto const value = sv::state_vector{
	    .frame = sv::reference_frame::wgs84_ecef,
	    .kind = sv::state_vector_kind::keplerian,
	    .elements = {1.0, 0.5, 2.0, 4.0, 8.0, 16.0},
	};

	CHECK(format.decode(pack("11")) == value);
	CHECK(encode(format, value) == pack("11"));

	// Each element uses 8 bytes when one element needs 8 bytes
	auto precise = value;
	precise.elements[5] = 0.1;
	auto const bytes = encode(format, precise);
	REQUIRE(bytes);
	CHECK(bytes->size() == 49);
	CHECK(format.decode(*bytes) == precise);
	CHECK(encode(sv::state_vector_codec{.length = 8}, value)->size() == 49);

	// A later version can add bytes to the State Vector Status. Decoding ignores them.
	auto experimental = value;
	experimental.frame = sv::reference_frame::experimental;
	experimental.kind = sv::state_vector_kind::experimental;
	CHECK(format.decode(pack("00")) == experimental);
	CHECK(format.decode(pack("91 05")) == value);

	CHECK(format.decode(pack("41")) == std::unexpected(field_error::invalid_value));
	CHECK(format.decode(pack("13")) == std::unexpected(field_error::invalid_value));
	CHECK(format.decode(hex("91")) == std::unexpected(field_error::invalid_value));
	CHECK(format.decode(pack("11 00")) == std::unexpected(field_error::bad_length));
	CHECK(encode(format, sv::state_vector{.frame = sv::reference_frame{4}, .kind = {}, .elements = {}})
	      == std::unexpected(field_error::invalid_value));
}

TEST_CASE("KLV ST 1504 vector packs and scalar items", "[klv][misb1504]") {
	constexpr auto format = sv::node_vector_pack::format;
	auto const unit = std::array{0.5, -0.25, 1.0};

	CHECK(encode(format, unit) == hex("3F 00 00 00 BE 80 00 00 3F 80 00 00"));
	CHECK(format.decode(hex("3F 00 00 00 BE 80 00 00 3F 80 00 00")) == unit);
	CHECK(
	    format.decode(hex("3F E0 00 00 00 00 00 00 BF D0 00 00 00 00 00 00 3F F0 00 00 00 00 00 00")) == unit
	);
	CHECK(format.decode(hex("3F 00 00 00 BE 80 00 00 3F 80 00")) == std::unexpected(field_error::bad_length));
	CHECK(format.to_string(unit) == "[0.5, -0.25, 1]");

	CHECK(sv::anomaly_type::format.decode(hex("00")) == sv::anomaly_kind::experimental);
	CHECK(sv::anomaly_type::format.decode(hex("05")) == std::unexpected(field_error::invalid_value));
	CHECK(sv::anomaly_type::format.to_string(sv::anomaly_kind::true_anomaly) == "True Anomaly");

	// The Mean Anomaly Epoch can use fewer than 8 bytes
	CHECK(sv::mean_anomaly_epoch::format.decode(hex("06 0A 24 14 8A B9 00")) == micros(1'699'999'940'000'000));
}
