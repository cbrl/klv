// The byte examples are test vectors of the KWIVER test suite.

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <expected>
#include <string>
#include <string_view>
#include <vector>

#include "klv_test_util.hpp"
#include "klv/misb1108.hpp"
#include "klv/packet.hpp"

using namespace klv;
using klv_test::hex;
using klv_test::micros;

namespace iq = klv::misb1108;

static_assert(universal_set_definition<iq::interpretability_quality_ls>);
static_assert(has_checksum<iq::interpretability_quality_ls>);
static_assert(local_set_definition<iq::metric_ls> && !universal_set_definition<iq::metric_ls>);

namespace {

/// Interpretability and Quality Local Set without its checksum
constexpr auto set_bytes = std::string_view{
    "01 01 05 "
    "02 0C 00 05 CA 79 F2 FB E0 00 00 6A CF C0 "
    "03 06 00 00 85 50 8A 00 "
    "04 2F 01 06 56 4E 49 49 52 53 02 03 33 2E 30 03 05 4B 57 1E 43 56 04 05 41 30 2B 41 31 "
    "05 08 00 05 CA 79 F2 FB E0 00 06 08 40 1C 7E 6B 74 DD 1B D3 "
    "05 01 02 06 01 04 07 03 35 2E 32 08 04 41 C9 99 9A 09 02 04 00 0A 01 03"
};

template<typename Codec>
[[nodiscard]]
auto encode(Codec const& codec, typename Codec::value_type const& value) -> field<byte_vector> {
	auto out = byte_vector{};
	auto writer = byte_writer{out};
	return codec.encode(value, writer).transform([&] { return out; });
}

}  // namespace

TEST_CASE("KLV ST 1108 set decodes and re-encodes an example", "[klv][misb1108]") {
	auto const bytes = hex(set_bytes);
	auto const rec = decode_set<iq::interpretability_quality_ls>(bytes);
	REQUIRE(rec);
	CHECK(rec->unknown().empty());

	CHECK(rec->get<iq::assessment_point>() == iq::assessment_location::library_archive);
	auto const& period = rec->get<iq::metric_period_pack>();
	REQUIRE(period);
	CHECK(period->start == micros(1'630'000'000'000'000));
	CHECK(period->offset == std::chrono::microseconds{7'000'000});
	CHECK(period->end() == micros(1'630'000'007'000'000));
	CHECK(rec->get<iq::window_corners_pack>() == iq::window_corners{.c2_row = 720, .c2_column = 1280});

	auto const& metrics = rec->get<iq::metric_local_set>();
	REQUIRE(metrics);
	REQUIRE(metrics->size() == 1);
	auto const& metric = metrics->front();
	CHECK(metric.unknown().empty());
	CHECK(metric.get<iq::metric_name>() == std::string{"VNIIRS"});
	CHECK(metric.get<iq::metric_version>() == std::string{"3.0"});
	CHECK(metric.get<iq::metric_implementer>() == iq::implementer{.organization = "KW", .subgroup = "CV"});
	CHECK(metric.get<iq::metric_parameters>() == std::string{"A0+A1"});
	CHECK(metric.get<iq::metric_time>() == micros(1'630'000'000'000'000));
	CHECK(metric.get<iq::metric_value>() == 7.12345678901234);
	CHECK(metric.get<iq::metric_units>().error() == field_error::absent);

	CHECK(rec->get<iq::compression_type>() == iq::compression_method::h264);
	CHECK(rec->get<iq::compression_profile>() == iq::compression_profile_kind::high);
	CHECK(rec->get<iq::compression_level>() == std::string{"5.2"});
	CHECK(rec->get<iq::compression_ratio>() == double{25.2F});
	CHECK(rec->get<iq::stream_bitrate>() == uint16_t{1024});
	CHECK(rec->get<iq::document_version>() == uint8_t{3});

	// The ratio is exact in binary32, so it keeps its 4-byte encoding
	CHECK(encode_set(*rec) == bytes);
}

TEST_CASE("KLV ST 1108 stand-alone packet carries a CRC-16-CCITT", "[klv][misb1108]") {
	auto const packet = hex(std::string{"06 0E 2B 34 02 03 01 01 0E 01 03 03 1C 00 00 00 66 "} + std::string{set_bytes}
	                        + " 0B 02 4A B4");

	auto const decoded = decode_packet<iq::interpretability_quality_ls>(packet);
	REQUIRE(decoded);
	CHECK(decoded->get<iq::checksum>() == uint16_t{0x4AB4});
	CHECK(encode_packet(*decoded) == packet);

	auto damaged = packet;
	damaged[19] = std::byte{0x04};
	auto const failed = decode_packet<iq::interpretability_quality_ls>(damaged);
	REQUIRE_FALSE(failed);
	CHECK(failed.error().code == error_code::checksum_mismatch);
}

TEST_CASE("KLV ST 1108 set holds one Metric Local Set for each metric", "[klv][misb1108]") {
	// A metric of a human observer
	auto const gsd = decode_set<iq::metric_ls>(hex(
	    "01 03 47 53 44 02 05 48 75 6D 61 6E 03 05 4B 57 1E 43 56 04 03 78 3D 37 "
	    "05 08 00 00 00 00 61 27 D3 80 06 08 3F F3 C0 C9 53 9B 88 87"
	));
	REQUIRE(gsd);
	CHECK(gsd->get<iq::metric_name>() == std::string{"GSD"});
	CHECK(gsd->get<iq::metric_version>() == std::string{"Human"});
	CHECK(gsd->get<iq::metric_parameters>() == std::string{"x=7"});
	CHECK(gsd->get<iq::metric_time>() == micros(1'630'000'000));
	CHECK(gsd->get<iq::metric_value>() == 1.234567);

	auto with_units = *gsd;
	with_units.set<iq::metric_units>("m");

	auto vniirs = record<iq::metric_ls>{};
	vniirs.set<iq::metric_name>("VNIIRS");
	vniirs.set<iq::metric_version>("2.1");
	vniirs.set<iq::metric_implementer>(iq::implementer{.organization = "Lab", .subgroup = "Video"});
	vniirs.set<iq::metric_time>(micros(1'630'000'001'000'000));
	vniirs.set<iq::metric_value>(6.5);

	auto rec = record<iq::interpretability_quality_ls>{};
	rec.set<iq::assessment_point>(iq::assessment_location::gcs_received);
	rec.set<iq::metric_period_pack>(
	    iq::metric_period{.start = micros(1'630'000'000'000'000), .offset = std::chrono::microseconds{33'367}}
	);
	rec.add<iq::metric_local_set>(vniirs);
	rec.add<iq::metric_local_set>(with_units);
	rec.set<iq::compression_type>(iq::compression_method::uncompressed);
	rec.set<iq::compression_profile>(iq::compression_profile_kind::not_applicable);
	rec.set<iq::compression_level>("N/A");
	rec.set<iq::compression_ratio>(1.0);
	rec.set<iq::stream_bitrate>(uint16_t{2200});
	rec.set<iq::document_version>(uint8_t{4});

	auto const bytes = encode_packet(rec);
	REQUIRE(bytes);
	auto decoded = decode_packet<iq::interpretability_quality_ls>(*bytes);
	REQUIRE(decoded);
	decoded->erase<iq::checksum>();
	CHECK(*decoded == rec);

	auto const text = describe(*decoded);
	auto const value_of = [&](std::string_view name) {
		auto const found = std::ranges::find(text, name, &field_text::name);
		REQUIRE(found != text.end());
		return found->value;
	};
	CHECK(value_of("Assessment Point") == std::string{"GCS Received"});
	CHECK(value_of("Metric Period Pack") == std::string{"{start: 2021-08-26T17:46:40.000000Z, offset: 33367 us}"});
	CHECK(
	    value_of("Metric Local Set[0].Metric Implementer")
	    == std::string{"{organization: \"Lab\", subgroup: \"Video\"}"}
	);
	CHECK(value_of("Metric Local Set[1].Metric Units") == std::string{"m"});
	CHECK(value_of("Compression Profile") == std::string{"N/A (uncompressed)"});
}

TEST_CASE("KLV ST 1108 Metric Implementer needs the record separator only to encode a subgroup", "[klv][misb1108]") {
	constexpr auto format = iq::metric_implementer::format;

	// ST 1108 requires the separator, but some producers leave it out
	auto const without = format.decode(hex("4B 57"));
	REQUIRE(without);
	CHECK(*without == iq::implementer{.organization = "KW"});
	CHECK(encode(format, *without) == hex("4B 57"));
	CHECK(format.to_string(*without) == "{organization: \"KW\"}");

	// Empty parts
	CHECK(format.decode(hex("1E")) == iq::implementer{.organization = "", .subgroup = ""});
	CHECK(encode(format, iq::implementer{.organization = "", .subgroup = ""}) == hex("1E"));

	CHECK(encode(format, iq::implementer{.organization = "A\x1E" "B"}) == std::unexpected(field_error::invalid_value));
	CHECK(
	    encode(format, iq::implementer{.organization = std::string(100, 'A'), .subgroup = std::string(28, 'B')})
	    == std::unexpected(field_error::bad_length)
	);
}

TEST_CASE("KLV ST 1108 rejects malformed packs", "[klv][misb1108]") {
	constexpr auto period = iq::metric_period_pack::format;
	CHECK(period.decode(hex("00 05 CA 79 F2 FB E0 00 00 6A CF")) == std::unexpected(field_error::bad_length));
	CHECK(
	    encode(period, iq::metric_period{.offset = std::chrono::microseconds{-1}})
	    == std::unexpected(field_error::out_of_range)
	);
	CHECK(
	    encode(period, iq::metric_period{.offset = std::chrono::microseconds{int64_t{1} << 32}})
	    == std::unexpected(field_error::out_of_range)
	);

	constexpr auto corners = iq::window_corners_pack::format;
	CHECK(corners.decode(hex("00 00 85 50 8A 00 00")) == std::unexpected(field_error::invalid_value));
	CHECK(corners.decode(hex("00 00 85")) == std::unexpected(field_error::invalid_value));
	CHECK(encode(corners, iq::window_corners{.c2_row = 720, .c2_column = 1280}) == hex("00 00 85 50 8A 00"));

	// Compression Type 5 is reserved
	auto const rec = decode_set<iq::interpretability_quality_ls>(hex("05 01 05"));
	REQUIRE(rec);
	CHECK(rec->get<iq::compression_type>().error() == field_error::invalid_value);
}
