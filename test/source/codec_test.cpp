#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "klv_test_util.hpp"
#include "klv/codec.hpp"
#include "klv/misb0102.hpp"

using namespace klv;
using Catch::Approx;
using klv_test::hex;
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

/// IMAPB ranges of the standard deviations of six test items, as a parent standard defines them
auto test_sigma_ranges(uint32_t tag) -> std::optional<imapb_range> {
	switch (tag) {
		case 1:
		case 2:
			return imapb_range{.min = 0.0, .max = 650.0};
		case 3:
			return imapb_range{.min = 0.0, .max = 70.0};
		case 4:
			return imapb_range{.min = 0.0, .max = 2.0};
		case 5:
			return imapb_range{.min = 0.0, .max = 1.0};
		case 6:
			return imapb_range{.min = 0.0, .max = 0.2};
		default:
			return std::nullopt;
	}
}

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

TEST_CASE("KLV IMAPB codec maps with the ST 1201 equations", "[klv]") {
	// b - a = 40900, so bPow = 16. With 3 bytes, dPow = 23, sF = 128, and sR = 1/128.
	constexpr auto altitude = codec::imapb{-900.0, 40'000.0, 3};

	CHECK(encode(altitude, -900.0) == make_bytes({0x00, 0x00, 0x00}));
	CHECK(encode(altitude, 0.0) == make_bytes({0x01, 0xC2, 0x00}));
	CHECK(encode(altitude, 10.5) == make_bytes({0x01, 0xC7, 0x40}));
	CHECK(encode(altitude, 40'000.0) == make_bytes({0x4F, 0xE2, 0x00}));

	CHECK(altitude.decode(make_bytes({0x00, 0x00, 0x00})) == -900.0);
	CHECK(altitude.decode(make_bytes({0x01, 0xC2, 0x00})) == 0.0);
	CHECK(altitude.decode(make_bytes({0x01, 0xC7, 0x40})) == 10.5);
	CHECK(altitude.decode(make_bytes({0x4F, 0xE2, 0x00})) == 40'000.0);
}

TEST_CASE("KLV IMAPB codec decodes each length from 1 to 8 bytes", "[klv]") {
	constexpr auto altitude = codec::imapb{-900.0, 40'000.0, 3};

	// The 2-byte mapping has sF = 1/2, so 0x01C2 is zero
	CHECK(altitude.decode(make_bytes({0x01, 0xC2})) == 0.0);

	for (auto length = size_t{1}; length <= codec::imapb::max_length; ++length) {
		auto const format = codec::imapb{-900.0, 40'000.0, length};
		auto const bytes = encode(format, 1'234.5);
		REQUIRE(bytes);
		CHECK(bytes->size() == length);

		// One step of the mapping is 2^(bPow - dPow)
		auto const step = std::ldexp(1.0, 16 - (static_cast<int>(length) * 8 - 1));
		CHECK(altitude.decode(*bytes).value() == Approx(1'234.5).margin(step + 1e-9));
	}

	CHECK(altitude.decode({}) == std::unexpected(field_error::bad_length));
	CHECK(altitude.decode(byte_vector(9, std::byte{0})) == std::unexpected(field_error::bad_length));
}

TEST_CASE("KLV IMAPB codec maps zero exactly when min is negative", "[klv]") {
	// sF * min = -19.2 is not an integer, so zOffset = 0.8 moves zero to the raw value 20
	constexpr auto format = codec::imapb{-0.3, 1.0, 1};

	CHECK(encode(format, 0.0) == make_bytes({0x14}));
	CHECK(format.decode(make_bytes({0x14})) == 0.0);
}

TEST_CASE("KLV IMAPB codec maps max of a power-of-two range to the flag bit alone", "[klv]") {
	// max - min = 256 is a power of two, so max maps to 0x8000. With no other bit set, that is a normal value.
	constexpr auto format = codec::imapb{0.0, 256.0, 2};

	CHECK(encode(format, 256.0) == make_bytes({0x80, 0x00}));
	CHECK(format.decode(make_bytes({0x80, 0x00})) == 256.0);
	CHECK(format.decode(make_bytes({0x7F, 0xFF})) == 255.9921875);
}

TEST_CASE("KLV IMAPB codec applies the zero offset only to a range that contains zero", "[klv]") {
	// sF = 64 and sF * min = -12.8. The range does not contain zero, so zOffset = 0 and min maps to raw 0.
	constexpr auto format = codec::imapb{-0.2, -0.1, 1};

	CHECK(encode(format, -0.2) == make_bytes({0x00}));
	CHECK(format.decode(make_bytes({0x00})) == -0.2);
}

TEST_CASE("KLV IMAPB codec special values and range checks", "[klv]") {
	constexpr auto altitude = codec::imapb{-900.0, 40'000.0, 3};
	auto const nan = std::numeric_limits<double>::quiet_NaN();
	auto const infinity = std::numeric_limits<double>::infinity();

	CHECK(encode(altitude, nan) == make_bytes({0xD0, 0x00, 0x00}));
	CHECK(encode(altitude, std::copysign(nan, -1.0)) == make_bytes({0xF0, 0x00, 0x00}));
	CHECK(encode(altitude, infinity) == make_bytes({0xC8, 0x00, 0x00}));
	CHECK(encode(altitude, -infinity) == make_bytes({0xE8, 0x00, 0x00}));
	CHECK(encode(altitude, -900.5) == make_bytes({0xE0, 0x00, 0x00}));
	CHECK(encode(altitude, 40'000.5) == make_bytes({0xE1, 0x00, 0x00}));

	auto out = byte_vector{};
	auto writer = byte_writer{out};
	CHECK(altitude.encode_error_indicator(writer));
	CHECK(out == make_bytes({0xD0, 0x00, 0x00}));

	for (auto const first : {0xC8U, 0xE8U, 0xD0U, 0xF0U, 0xD8U, 0xF8U}) {
		CHECK(altitude.decode(make_bytes({first, 0x00, 0x00})) == std::unexpected(field_error::error_indicator));
	}
	// Below minimum, above maximum, user-defined, a reserved MISB value, and a reserved kind
	CHECK(altitude.decode(make_bytes({0xE0, 0x00, 0x00})) == std::unexpected(field_error::out_of_range));
	CHECK(altitude.decode(make_bytes({0xE1, 0x00, 0x00})) == std::unexpected(field_error::out_of_range));
	CHECK(altitude.decode(make_bytes({0xC0, 0x00, 0x01})) == std::unexpected(field_error::invalid_value));
	CHECK(altitude.decode(make_bytes({0xE2, 0x00, 0x00})) == std::unexpected(field_error::invalid_value));
	CHECK(altitude.decode(make_bytes({0x80, 0x00, 0x01})) == std::unexpected(field_error::invalid_value));

	// 0x4FE201 is one step above max. 0x800000 is a normal value, but it is far above max.
	CHECK(altitude.decode(make_bytes({0x4F, 0xE2, 0x01})) == std::unexpected(field_error::out_of_range));
	CHECK(altitude.decode(make_bytes({0x7F, 0xFF, 0xFF})) == std::unexpected(field_error::out_of_range));
	CHECK(altitude.decode(make_bytes({0x80, 0x00, 0x00})) == std::unexpected(field_error::out_of_range));
}

TEST_CASE("KLV integer codec encodes a fixed or variable length", "[klv]") {
	constexpr auto u24 = codec::integer<uint32_t>{.length = 3};
	constexpr auto u64 = codec::integer<uint64_t>{.length = codec::variable_length};
	constexpr auto i32 = codec::integer<int32_t>{.length = codec::variable_length};

	CHECK(encode(u24, 0x12'3456U) == make_bytes({0x12, 0x34, 0x56}));
	CHECK(encode(u24, 0x100'0000U) == std::unexpected(field_error::out_of_range));
	CHECK(encode(u64, 0) == make_bytes({0x00}));
	CHECK(encode(u64, 300) == make_bytes({0x01, 0x2C}));
	CHECK(encode(u64, 409'600) == make_bytes({0x06, 0x40, 0x00}));
	CHECK(encode(i32, -1) == make_bytes({0xFF}));
	CHECK(encode(i32, 128) == make_bytes({0x00, 0x80}));
	CHECK(encode(i32, -128) == make_bytes({0x80}));
	CHECK(encode(i32, -129) == make_bytes({0xFF, 0x7F}));
}

TEST_CASE("KLV BER-OID codec", "[klv]") {
	constexpr auto format = codec::ber_oid{};

	CHECK(format.decode(make_bytes({0x81, 0x48})) == uint32_t{200});
	CHECK(format.decode(make_bytes({0x01, 0x02})) == std::unexpected(field_error::invalid_value));
	CHECK(format.decode({}) == std::unexpected(field_error::invalid_value));
	CHECK(encode(format, 200) == make_bytes({0x81, 0x48}));
}

TEST_CASE("KLV variable IEEE 754 codec accepts both precisions", "[klv]") {
	constexpr auto format = codec::ieee754_variable{};

	CHECK(format.decode(make_bytes({0x43, 0x80, 0x00, 0x00})) == 256.0);
	CHECK(format.decode(make_bytes({0x3F, 0xF0, 0, 0, 0, 0, 0, 0})) == 1.0);
	CHECK(format.decode(make_bytes({0x3F, 0x80})) == std::unexpected(field_error::bad_length));

	// The shortest exact form
	CHECK(encode(format, 256.0) == make_bytes({0x43, 0x80, 0x00, 0x00}));
	CHECK(encode(format, 0.1) == make_bytes({0x3F, 0xB9, 0x99, 0x99, 0x99, 0x99, 0x99, 0x9A}));
	CHECK(encode(format, 1e300).value().size() == 8);

	CHECK(encode(codec::ieee754_variable{.length = 8}, 256.0).value().size() == 8);
	CHECK(encode(codec::ieee754_variable{.length = 4}, 1e300) == std::unexpected(field_error::out_of_range));
}

TEST_CASE("KLV UUID codec", "[klv]") {
	constexpr auto format = codec::uuid{};
	auto const bytes = hex("F8 1D 4F AE 7D EC 11 D0 A7 65 00 A0 C9 1E 6B F6");

	auto const value = format.decode(bytes);
	REQUIRE(value);
	CHECK(format.to_string(*value) == "f81d4fae-7dec-11d0-a765-00a0c91e6bf6");
	CHECK(encode(format, *value) == bytes);
	CHECK(format.decode(make_bytes({0x01})) == std::unexpected(field_error::bad_length));
}

TEST_CASE("KLV repeated codec decodes one occurrence as a list of one item", "[klv]") {
	constexpr auto format = codec::repeated<codec::integer<uint8_t>>{};

	CHECK(format.decode(make_bytes({0x07})) == std::vector<uint8_t>{7});
	CHECK(format.decode({}) == std::unexpected(field_error::bad_length));
	CHECK(encode(format, {7}) == make_bytes({0x07}));
	CHECK(encode(format, {7, 8}) == std::unexpected(field_error::invalid_value));
	CHECK(format.to_string({7, 8}) == "[7, 8]");
}

// The MDARRAY and SDCC-FLP byte examples come from the MISB standards, as the KWIVER test suite holds them
TEST_CASE("KLV MDARRAY codec decodes and encodes the ST 1303 examples", "[klv]") {
	constexpr auto reals = codec::mdarray<double>{};
	constexpr auto integers = codec::mdarray<uint64_t>{};

	SECTION("IMAPB elements") {
		auto const bytes =
		    hex("02 04 02 02 02 3F 80 00 00 41 00 00 00 00 00 10 00 20 00 30 00 40 00 50 00 60 00 70 00");
		auto const value = reals.decode(bytes);
		REQUIRE(value);
		CHECK(value->sizes == std::vector<uint32_t>{4, 2});
		CHECK(value->elements == std::vector<double>{1, 2, 3, 4, 5, 6, 7, 8});
		CHECK(value->element_size == 2);
		CHECK(value->algorithm == array_algorithm::imapb);
		CHECK(value->imapb_min == Approx(1.0));
		CHECK(value->imapb_max == Approx(8.0));
		CHECK(value->imapb_limit_length == 4);
		CHECK(encode(reals, *value) == bytes);
	}

	SECTION("natural binary32 elements") {
		auto const bytes = hex(
		    "02 04 02 04 01 3F 80 00 00 40 00 00 00 40 40 00 00 40 80 00 00 "
		    "40 A0 00 00 40 C0 00 00 40 E0 00 00 41 00 00 00"
		);
		auto const value = reals.decode(bytes);
		REQUIRE(value);
		CHECK(value->elements == std::vector<double>{1, 2, 3, 4, 5, 6, 7, 8});
		CHECK(value->algorithm == array_algorithm::natural);
		CHECK(encode(reals, *value) == bytes);
	}

	SECTION("unsigned integers with a bias") {
		auto const first = hex("02 03 03 01 04 00 0C 36 82 5E 02 90 00 00 7F 81 00 01");
		auto const value = integers.decode(first);
		REQUIRE(value);
		CHECK(value->elements == std::vector<uint64_t>{12, 54, 350, 2, 2048, 0, 127, 128, 1});
		CHECK(encode(integers, *value) == first);

		auto const second = hex("01 05 01 04 81 02 00 28 19 0D 3C");
		auto const biased = integers.decode(second);
		REQUIRE(biased);
		CHECK(biased->elements == std::vector<uint64_t>{130, 170, 155, 143, 190});
		CHECK(encode(integers, *biased) == second);
	}

	SECTION("run-length encoding in three dimensions") {
		auto const bytes = hex(
		    "03 02 03 04 01 05 03 "
		    "01 00 00 00 01 01 02 02 00 00 02 02 01 02 04 00 01 03 01 01 01 "
		    "04 00 02 02 02 01 02 00 01 00 00 01 01 01 01 01 00 01 01 01 01"
		);
		auto const value = integers.decode(bytes);
		REQUIRE(value);
		CHECK(value->sizes == std::vector<uint32_t>{2, 3, 4});
		CHECK(
		    value->elements
		    == std::vector<uint64_t>{1, 1, 2, 2, 3, 3, 3, 4, 3, 3, 4, 4, 0, 1, 2, 2, 3, 3, 3, 3, 3, 3, 4, 4}
		);

		// The encoder finds other rectangles than the example. Thus, its bytes differ.
		auto const encoded = encode(integers, *value);
		REQUIRE(encoded);
		CHECK(integers.decode(*encoded) == value);
	}

	SECTION("text") {
		auto const value = mdarray<double>{.sizes = {2}, .elements = {1.5, 2.0}};
		CHECK(reals.to_string(value) == "{sizes: [2], algorithm: Natural, elements: [1.5, 2]}");
	}
}

TEST_CASE("KLV MDARRAY codec rejects malformed and unsupported arrays", "[klv]") {
	constexpr auto reals = codec::mdarray<double>{};
	constexpr auto integers = codec::mdarray<uint64_t>{};
	auto const invalid = std::unexpected(field_error::invalid_value);

	CHECK(reals.decode(hex("00 04 01")) == invalid);  // No dimensions
	CHECK(reals.decode(hex("01 00 04 01")) == invalid);  // An empty dimension
	CHECK(reals.decode(hex("01 02 04 01 3F 80 00 00")) == invalid);  // One element is missing
	CHECK(reals.decode(hex("01 01 02 01 3F 80")) == invalid);  // 2-byte IEEE 754 elements
	CHECK(integers.decode(hex("01 01 01 02 00")) == invalid);  // IMAPB for integers
	CHECK(integers.decode(hex("02 05 04 01 03 48 A8 F0")) == invalid);  // The Boolean algorithm needs bool elements

	// A run-length rectangle that ends outside the array
	CHECK(integers.decode(hex("01 04 01 05 00 07 03 02")) == invalid);

	// IMAPB limits that are not in order
	CHECK(reals.decode(hex("01 01 01 02 41 00 00 00 3F 80 00 00 00")) == invalid);

	CHECK(encode(reals, mdarray<double>{.sizes = {3}, .elements = {1.0}}) == invalid);
	CHECK(
	    encode(integers, mdarray<uint64_t>{.sizes = {1}, .elements = {256}, .element_size = 1})
	    == std::unexpected(field_error::out_of_range)
	);
}

TEST_CASE("KLV MDARRAY codec decodes and encodes Boolean arrays", "[klv]") {
	constexpr auto format = codec::mdarray<bool>{};

	SECTION("one bit for each element (ST 1303 Appendix D.2)") {
		auto const bytes = hex("02 05 04 01 03 48 A8 F0");
		auto const value = format.decode(bytes);
		REQUIRE(value);
		CHECK(value->sizes == std::vector<uint32_t>{5, 4});
		auto expected = std::vector<bool>{};
		for (auto const bit : std::string_view{"01001000101010001111"}) {
			expected.push_back(bit == '1');
		}
		CHECK(value->elements == expected);
		CHECK(value->algorithm == array_algorithm::boolean);
		CHECK(encode(format, *value) == bytes);

		// The last byte is missing
		CHECK(format.decode(hex("02 05 04 01 03 48 A8")) == std::unexpected(field_error::invalid_value));
	}

	SECTION("natural elements: not zero is true") {
		auto const value = mdarray<bool>{.sizes = {3}, .elements = {true, false, true}, .element_size = 2};
		auto const bytes = hex("01 03 02 01 00 01 00 00 00 01");
		CHECK(encode(format, value) == bytes);
		CHECK(format.decode(bytes) == value);
		CHECK(format.decode(hex("01 01 01 01 80"))->elements == std::vector<bool>{true});
	}

	SECTION("run-length elements") {
		auto const value = mdarray<bool>{
		    .sizes = {2, 3},
		    .elements = {false, true, true, false, false, false},
		    .element_size = 1,
		    .algorithm = array_algorithm::run_length,
		};
		auto const bytes = encode(format, value);
		REQUIRE(bytes);
		CHECK(format.decode(*bytes) == value);
	}

	SECTION("text") {
		auto const value = mdarray<bool>{
		    .sizes = {2},
		    .elements = {true, false},
		    .algorithm = array_algorithm::boolean,
		};
		CHECK(format.to_string(value) == "{sizes: [2], algorithm: Boolean, elements: [true, false]}");
	}
}

TEST_CASE("KLV MDARRAY codec decodes and encodes signed integer arrays", "[klv]") {
	constexpr auto format = codec::mdarray<int64_t>{};

	SECTION("run-length elements (ST 1303 Appendix D.4)") {
		auto const bytes = hex(
		    "02 0A 0A 02 05 FA 70 "
		    "06 78 00 00 04 03 00 00 00 05 04 05 FC 09 04 00 06 03 03 D2 04 05 03 05 04 EC 07 05 03 05"
		);
		auto const value = format.decode(bytes);
		REQUIRE(value);
		CHECK(value->sizes == std::vector<uint32_t>{10, 10});
		CHECK(value->element_size == 2);
		CHECK(value->algorithm == array_algorithm::run_length);

		auto expected = std::vector<int64_t>{};
		for (auto row = 0; row < 10; ++row) {
			auto const left = row < 4 ? int64_t{1656} : int64_t{-1015};
			auto const right = row < 4 ? int64_t{0} : (row < 7 ? int64_t{978} : int64_t{1260});
			for (auto column = 0; column < 10; ++column) {
				expected.push_back(column < 3 ? left : (column < 5 ? int64_t{-1424} : right));
			}
		}
		CHECK(value->elements == expected);

		// The encoder finds the same background and the same five rectangles as the example
		CHECK(encode(format, *value) == bytes);
	}

	SECTION("overlapping run-length rectangles (ST 1303 Appendix D.4, Figure 13 and Figure 14)") {
		auto expected = std::vector<int64_t>{};
		for (auto row = 0; row < 10; ++row) {
			auto const middle = row >= 4 && row <= 6;
			for (auto column = 0; column < 10; ++column) {
				if (column == 3 || column == 4) {
					expected.push_back(275);
				}
				else if (middle) {
					expected.push_back(13);
				}
				else {
					expected.push_back(column < 3 ? 27 : 0);
				}
			}
		}

		// Three rectangles on the background 0. A later rectangle overwrites an earlier one.
		auto const three =
		    format.decode(hex("02 0A 0A 02 05 00 00 00 1B 00 00 0A 03 00 0D 04 00 03 0A 01 13 00 03 0A 02"));
		REQUIRE(three);
		CHECK(three->elements == expected);

		// Four rectangles with 130 elements in an array of 100 elements
		auto const four = format.decode(
		    hex("02 0A 0A 02 05 00 00 00 1B 00 00 0A 03 00 00 00 05 0A 05 00 0D 04 00 03 0A 01 13 00 03 0A 02")
		);
		REQUIRE(four);
		CHECK(four->elements == expected);

		auto const encoded = encode(format, *four);
		REQUIRE(encoded);
		CHECK(format.decode(*encoded) == four);
	}

	SECTION("natural elements in two's complement") {
		auto const value = mdarray<int64_t>{.sizes = {3}, .elements = {-1, 127, -128}, .element_size = 1};
		auto const bytes = hex("01 03 01 01 FF 7F 80");
		CHECK(encode(format, value) == bytes);
		CHECK(format.decode(bytes) == value);

		auto const too_large = mdarray<int64_t>{.sizes = {1}, .elements = {128}, .element_size = 1};
		auto const too_small = mdarray<int64_t>{.sizes = {1}, .elements = {-129}, .element_size = 1};
		CHECK(encode(format, too_large) == std::unexpected(field_error::out_of_range));
		CHECK(encode(format, too_small) == std::unexpected(field_error::out_of_range));
	}

	SECTION("the unsigned integer and Boolean algorithms need other element types") {
		CHECK(format.decode(hex("01 01 01 04 00 05")) == std::unexpected(field_error::invalid_value));
		CHECK(format.decode(hex("01 01 01 03 80")) == std::unexpected(field_error::invalid_value));
	}
}

TEST_CASE("KLV MDARRAY codec decodes an IMAPB special value as NaN", "[klv]") {
	auto const value = codec::mdarray<double>{}.decode(hex("01 02 02 02 3F 80 00 00 41 00 00 00 00 00 D0 00"));
	REQUIRE(value);
	CHECK(value->elements[0] == Approx(1.0));
	CHECK(std::isnan(value->elements[1]));
}

TEST_CASE("KLV SDCC-FLP codec", "[klv]") {
	constexpr auto format = codec::sdcc_flp{};

	SECTION("two parse control bytes, sparse, IMAPB correlation coefficients") {
		auto const bytes = hex("03 B3 04 80 3F 80 00 00 40 00 00 00 00 00 00 00 20 00 00");
		auto const value = format.decode(bytes);
		REQUIRE(value);
		CHECK(value->size == 3);
		CHECK(value->sigma == std::vector{1.0, 2.0, 0.0});
		CHECK(value->sigma_raw.empty());
		CHECK(value->members.empty());
		CHECK(value->rho == std::vector{-0.5, 0.0, 0.0});
		CHECK(value->sigma_length == 4);
		CHECK(value->rho_length == 3);
		CHECK(value->sparse);
		CHECK(value->long_parse_control);
		CHECK(value->rho_imapb);
		CHECK(encode(format, *value) == bytes);
	}

	SECTION("one parse control byte") {
		auto const bytes = hex(
		    "08 40 3F 80 00 00 40 00 00 00 40 40 00 00 40 80 00 00 40 A0 00 00 40 C0 00 00 40 E0 00 00 41 00 00 00"
		);
		auto const value = format.decode(bytes);
		REQUIRE(value);
		CHECK(value->sigma == std::vector{1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0});
		CHECK(value->rho.empty());
		CHECK_FALSE(value->long_parse_control);
		CHECK(encode(format, *value) == bytes);
	}

	SECTION("IMAPB standard deviations stay raw integers without their ranges") {
		auto const bytes = hex("06 80 12 25 80 25 80 1E 00 40 00 80 00 40 00");
		auto const value = format.decode(bytes);
		REQUIRE(value);
		CHECK(value->sigma_imapb);
		CHECK(value->sigma.empty());
		CHECK(value->sigma_raw == std::vector<uint64_t>{0x2580, 0x2580, 0x1E00, 0x4000, 0x8000, 0x4000});
		CHECK(value->rho_length == 0);
		CHECK(encode(format, *value) == bytes);

		// The set gives the tags of the earlier items. The parent standard defines no ranges here.
		auto const preceding = std::vector<uint32_t>{9, 1, 2, 3, 4, 5, 6};
		auto const in_set = format.decode(bytes, preceding);
		REQUIRE(in_set);
		CHECK(in_set->members == std::vector<uint32_t>{1, 2, 3, 4, 5, 6});
		CHECK(in_set->sigma_raw == value->sigma_raw);
		CHECK(format.covered_tags(*in_set).size() == 6);
	}

	SECTION("IMAPB standard deviations use the ranges of the covered items") {
		constexpr auto ranged = codec::sdcc_flp{.sigma_ranges = &test_sigma_ranges};
		auto const bytes = hex("06 80 12 25 80 25 80 1E 00 40 00 80 00 40 00");
		auto const preceding = std::vector<uint32_t>{9, 1, 2, 3, 4, 5, 6};

		auto const value = ranged.decode(bytes, preceding);
		REQUIRE(value);
		CHECK(value->sigma == std::vector{300.0, 300.0, 30.0, 1.0, 1.0, 0.125});
		CHECK(value->sigma_raw.empty());
		CHECK(encode(ranged, *value) == bytes);

		// Without the tags of the covered items, the values stay raw
		CHECK(ranged.decode(bytes)->sigma_raw.size() == 6);

		// The pack covers more items than the set holds before it
		CHECK(ranged.decode(bytes, std::vector<uint32_t>{1, 2}) == std::unexpected(field_error::invalid_value));

		// A covered item without a range
		auto unknown = *value;
		unknown.members.back() = 7;
		CHECK(encode(ranged, unknown) == std::unexpected(field_error::invalid_value));

		// Real IMAPB values need the ranges, and the members must match the matrix size
		CHECK(encode(format, *value) == std::unexpected(field_error::invalid_value));
		unknown.members.pop_back();
		CHECK(encode(ranged, unknown) == std::unexpected(field_error::invalid_value));
	}

	SECTION("malformed packs and values") {
		CHECK(format.decode(hex("00 40")) == std::unexpected(field_error::invalid_value));  // No items
		CHECK(format.decode(hex("01 40 3F 80 00 00 00")) == std::unexpected(field_error::invalid_value));  // Extra byte
		CHECK(format.decode(hex("01 50 3F 80 00 00 00")) == std::unexpected(field_error::bad_length));  // 5-byte IEEE

		// Mode 1 needs IEEE 754 standard deviations and IMAPB correlation coefficients
		auto value = klv::sdcc_flp{
		    .size = 2,
		    .sigma = {1.0, 2.0},
		    .rho = {0.5},
		    .long_parse_control = false,
		    .rho_imapb = false,
		};
		CHECK(encode(format, value) == std::unexpected(field_error::invalid_value));
		value.long_parse_control = true;
		CHECK(encode(format, value).has_value());

		value.sigma.pop_back();
		CHECK(encode(format, value) == std::unexpected(field_error::invalid_value));

		// Raw standard deviations need the IMAPB flag
		auto raw = klv::sdcc_flp{.size = 1, .sigma_raw = {5}, .rho_length = 0, .long_parse_control = true};
		CHECK(encode(format, raw) == std::unexpected(field_error::invalid_value));
		raw.sigma_imapb = true;
		raw.sigma_length = 2;
		CHECK(encode(format, raw) == hex("01 90 12 00 05"));
	}

	SECTION("Mode 2 parse control by default (MISB ST 0601, ST 1107, and ST 1202)") {
		auto const value = klv::sdcc_flp{.size = 1, .sigma = {1.0}, .rho_length = 0};
		CHECK(encode(format, value) == hex("01 90 04 3F 80 00 00"));
	}

	SECTION("a sparse pack without correlation coefficients has no bit vector (MISB ST 1010 Table 3)") {
		auto const bytes = hex("02 A0 04 3F 80 00 00 40 00 00 00");
		auto const value = format.decode(bytes);
		REQUIRE(value);
		CHECK(value->sparse);
		CHECK(value->rho_length == 0);
		CHECK(value->sigma == std::vector{1.0, 2.0});
		CHECK(encode(format, *value) == bytes);
	}

	SECTION("text") {
		auto const value = klv::sdcc_flp{.size = 2, .members = {13, 14}, .sigma = {1.0, 2.0}, .rho = {0.5}};
		CHECK(format.to_string(value) == "{size: 2, members: [13, 14], sigma: [1, 2], rho: [0.5]}");
	}
}
