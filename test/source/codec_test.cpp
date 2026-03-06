#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <cstdint>
#include <limits>
#include <string>

#include "klv_test_util.hpp"
#include "klv/codec.hpp"
#include "klv/misb0102.hpp"

using namespace klv;
using Catch::Approx;
using klv_test::make_bytes;

namespace {

template<typename Codec>
[[nodiscard]]
auto encode(Codec const& codec, typename Codec::value_type const& value) -> field<byte_vector> {
	auto out = byte_vector{};
	auto writer = byte_writer{out};
	return codec.encode(value, writer).transform([&] { return out; });
}

constexpr auto heading = codec::mapped<uint16_t>{0.0, 360.0};
constexpr auto pitch = codec::mapped<int16_t>{-20.0, 20.0, codec::sentinel::reserve_min};
constexpr auto latitude = codec::mapped<int32_t>{-90.0, 90.0, codec::sentinel::reserve_min};

}  // namespace

TEST_CASE("KLV integer codec accepts short encodings", "[klv]") {
	constexpr auto u16 = codec::integer<uint16_t>{};
	constexpr auto i8 = codec::integer<std::int8_t>{};

	CHECK(u16.decode(make_bytes({0x12, 0x34})) == uint16_t{0x1234});
	CHECK(u16.decode(make_bytes({0x12})) == uint16_t{0x12});
	CHECK(u16.decode(make_bytes({0x01, 0x02, 0x03})) == std::unexpected(field_error::bad_length));
	CHECK(u16.decode({}) == std::unexpected(field_error::bad_length));
	CHECK(i8.decode(make_bytes({0xFF})) == std::int8_t{-1});

	CHECK(encode(u16, 0x12) == make_bytes({0x00, 0x12}));
	CHECK(u16.to_string(42) == "42");
	CHECK(codec::integer<uint8_t>{}.to_string(200) == "200");
}

TEST_CASE("KLV mapped codec has no error value for unsigned fields", "[klv]") {
	// Raw 0 is a valid heading (north). The old library decoded it as an error.
	CHECK(heading.decode(make_bytes({0x00, 0x00})) == 0.0);
	CHECK(heading.decode(make_bytes({0xFF, 0xFF})) == 360.0);
	CHECK(heading.decode(make_bytes({0x71, 0xC2})).value() == Approx(29'122.0 * 360.0 / 65'535.0).margin(1e-12));
	CHECK_FALSE(heading.error_value().has_value());
}

TEST_CASE("KLV mapped codec reserves the minimum of signed fields", "[klv]") {
	CHECK(pitch.decode(make_bytes({0x80, 0x00})) == std::unexpected(field_error::error_indicator));
	CHECK(pitch.decode(make_bytes({0x80, 0x01})) == -20.0);
	CHECK(pitch.decode(make_bytes({0x7F, 0xFF})) == 20.0);
	CHECK(pitch.decode(make_bytes({0x00, 0x00})) == 0.0);
	CHECK(pitch.decode(make_bytes({0xFD, 0x3D})).value() == Approx(-707.0 * 40.0 / 65'534.0).margin(1e-12));
}

TEST_CASE("KLV mapped latitude maps the full symmetric domain", "[klv]") {
	// ST 0601 maps -(2^31 - 1)..(2^31 - 1) to -90..90. The old library used 2^31 - 2.
	CHECK(latitude.decode(make_bytes({0x7F, 0xFF, 0xFF, 0xFF})) == 90.0);
	CHECK(latitude.decode(make_bytes({0x80, 0x00, 0x00, 0x01})) == -90.0);
	CHECK(latitude.decode(make_bytes({0x00, 0x00, 0x00, 0x00})) == 0.0);
	CHECK(latitude.decode(make_bytes({0x80, 0x00, 0x00, 0x00})) == std::unexpected(field_error::error_indicator));
}

TEST_CASE("KLV mapped codec checks the value length", "[klv]") {
	CHECK(heading.decode(make_bytes({0x71})) == std::unexpected(field_error::bad_length));
	CHECK(heading.decode(make_bytes({0x71, 0xC2, 0x00})) == std::unexpected(field_error::bad_length));
	CHECK(latitude.decode(make_bytes({0x00, 0x00})) == std::unexpected(field_error::bad_length));
}

TEST_CASE("KLV mapped codec rounds to the nearest raw value", "[klv]") {
	// 45.5 * 65535 / 360 = 8282.85, so the nearest raw value is 8283 (0x205B). Truncation gives 8282.
	CHECK(encode(heading, 45.5) == make_bytes({0x20, 0x5B}));
	CHECK(encode(pitch, 0.0) == make_bytes({0x00, 0x00}));
	CHECK(encode(pitch, -20.0) == make_bytes({0x80, 0x01}));
	CHECK(encode(latitude, 90.0) == make_bytes({0x7F, 0xFF, 0xFF, 0xFF}));

	for (auto const value : {0.0, 0.001, 45.5, 123.456, 180.0, 359.99, 360.0}) {
		auto const bytes = encode(heading, value);
		REQUIRE(bytes);
		CHECK(heading.decode(*bytes).value() == Approx(value).margin(360.0 / 65'535.0 / 2.0 + 1e-12));
	}
}

TEST_CASE("KLV mapped codec writes the error value for NaN and out-of-range values", "[klv]") {
	auto const nan = std::numeric_limits<double>::quiet_NaN();

	CHECK(encode(pitch, nan) == make_bytes({0x80, 0x00}));
	CHECK(encode(pitch, 25.0) == make_bytes({0x80, 0x00}));
	CHECK(encode(heading, nan) == std::unexpected(field_error::invalid_value));
	CHECK(encode(heading, 361.0) == std::unexpected(field_error::out_of_range));
	CHECK(encode(heading, -0.5) == std::unexpected(field_error::out_of_range));

	auto out = byte_vector{};
	auto writer = byte_writer{out};
	CHECK(pitch.encode_error_indicator(writer));
	CHECK(out == make_bytes({0x80, 0x00}));
	CHECK(heading.encode_error_indicator(writer) == std::unexpected(field_error::invalid_value));
}

TEST_CASE("KLV timestamp codec", "[klv]") {
	constexpr auto format = codec::timestamp{};
	constexpr auto micros = int64_t{1'224'807'209'913'000};
	auto const value = timestamp{std::chrono::microseconds{micros}};

	auto const bytes = encode(format, value);
	REQUIRE(bytes);
	CHECK(bytes->size() == 8);
	CHECK(format.decode(*bytes) == value);
	CHECK(format.to_string(value) == "2008-10-24T00:13:29.913000Z");

	CHECK(format.decode(make_bytes({0x00, 0x01})) == std::unexpected(field_error::bad_length));
	CHECK(format.decode(make_bytes({0x80, 0, 0, 0, 0, 0, 0, 0})) == std::unexpected(field_error::out_of_range));
	CHECK(encode(format, timestamp{std::chrono::microseconds{-1}}) == std::unexpected(field_error::out_of_range));
}

TEST_CASE("KLV string codec", "[klv]") {
	constexpr auto format = codec::string{.max_length = 4};

	CHECK(format.decode(make_bytes({'H', 'i', 0, 0})) == std::string{"Hi"});
	CHECK(format.decode({}) == std::string{});
	CHECK(encode(format, "Test") == make_bytes({'T', 'e', 's', 't'}));
	CHECK(encode(format, "Tests") == std::unexpected(field_error::bad_length));
}

TEST_CASE("KLV IEEE 754 codec", "[klv]") {
	constexpr auto f32 = codec::ieee754<float>{};
	constexpr auto f64 = codec::ieee754<double>{};

	CHECK(encode(f32, 1.0F) == make_bytes({0x3F, 0x80, 0x00, 0x00}));
	CHECK(f32.decode(make_bytes({0x3F, 0x80, 0x00, 0x00})) == 1.0F);
	CHECK(f64.decode(make_bytes({0x3F, 0xF0, 0, 0, 0, 0, 0, 0})) == 1.0);
	CHECK(f64.decode(make_bytes({0x3F, 0x80, 0x00, 0x00})) == std::unexpected(field_error::bad_length));
}

TEST_CASE("KLV enumeration codec rejects undefined values", "[klv]") {
	constexpr auto format = misb0102::security_classification::format;

	CHECK(format.decode(make_bytes({0x04})) == misb0102::classification::secret);
	CHECK(format.decode(make_bytes({0x09})) == std::unexpected(field_error::invalid_value));
	CHECK(format.decode(make_bytes({0x00, 0x01})) == std::unexpected(field_error::bad_length));
	CHECK(format.to_string(misb0102::classification::top_secret) == "TOP SECRET");
	CHECK(encode(format, static_cast<misb0102::classification>(0x09)) == std::unexpected(field_error::invalid_value));
}

TEST_CASE("KLV bytes codec keeps the bytes", "[klv]") {
	constexpr auto format = codec::bytes{};
	auto const data = make_bytes({0xAB, 0xCD});

	CHECK(format.decode(data) == data);
	CHECK(encode(format, data) == data);
	CHECK(format.to_string(data) == "AB CD");
}
