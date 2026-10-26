#include <catch2/catch_test_macros.hpp>

#include <array>
#include <cstdint>
#include <expected>
#include <limits>

#include "klv_test_util.hpp"
#include "klv/bytes.hpp"

using namespace klv;
using klv_test::hex;
using klv_test::make_bytes;

// A BER-OID value is a number. bool and the character types do not hold one.
static_assert(ber_oid_integer<uint8_t> && ber_oid_integer<uint32_t> && ber_oid_integer<uint64_t>);
static_assert(!ber_oid_integer<bool> && !ber_oid_integer<char8_t> && !ber_oid_integer<char16_t>);
static_assert(!ber_oid_integer<char32_t> && !ber_oid_integer<int32_t>);

namespace {

template<typename T>
concept reads_ber_oid_as = requires(byte_reader reader) { reader.read_ber_oid<T>(); };

static_assert(reads_ber_oid_as<uint64_t> && !reads_ber_oid_as<bool>);

/// Status type for the length-prefixed write tests
using encode_result_for_test = std::expected<void, int>;

}  // namespace

TEST_CASE("KLV big-endian integers keep their sign", "[klv]") {
	CHECK(read_big_endian<uint16_t>(make_bytes({0x12, 0x34})) == 0x1234);
	CHECK(read_big_endian<uint32_t>(make_bytes({0x12})) == 0x12);
	CHECK(read_big_endian<int16_t>(make_bytes({0xFF, 0xFE})) == -2);
	CHECK(read_big_endian<int32_t>(make_bytes({0xFE})) == -2);
	CHECK(read_big_endian<int32_t>(make_bytes({0x7F})) == 127);
	CHECK(read_big_endian<uint8_t>(make_bytes({0xAB})) == 0xAB);
}

TEST_CASE("KLV reader take does not move past the end", "[klv]") {
	auto const data = make_bytes({0x01, 0x02, 0x03});
	auto reader = byte_reader{data};

	auto const too_many = reader.take(4);
	REQUIRE_FALSE(too_many);
	CHECK(too_many.error().code == error_code::unexpected_end_of_data);
	CHECK(reader.position() == 0);

	auto const two = reader.take(2);
	REQUIRE(two);
	CHECK(two->size() == 2);
	CHECK(reader.remaining() == 1);
}

TEST_CASE("KLV BER length decodes the short and long forms", "[klv]") {
	auto const read = [](byte_vector const& bytes) {
		auto reader = byte_reader{bytes};
		return reader.read_ber_length();
	};

	CHECK(read(make_bytes({0x05})) == 5U);
	CHECK(read(make_bytes({0x7F})) == 127U);
	CHECK(read(make_bytes({0x81, 0x80})) == 128U);
	CHECK(read(make_bytes({0x82, 0x01, 0x00})) == 256U);

	SECTION("indefinite form is an error") {
		auto const length = read(make_bytes({0x80}));
		REQUIRE_FALSE(length);
		CHECK(length.error().code == error_code::invalid_ber_length);
	}

	SECTION("more than 8 length bytes is an error") {
		auto const length = read(make_bytes({0x89, 0, 0, 0, 0, 0, 0, 0, 0, 1}));
		REQUIRE_FALSE(length);
		CHECK(length.error().code == error_code::invalid_ber_length);
	}

	SECTION("missing length bytes is an error") {
		auto const length = read(make_bytes({0x82, 0x01}));
		REQUIRE_FALSE(length);
		CHECK(length.error().code == error_code::unexpected_end_of_data);
	}
}

TEST_CASE("KLV huge BER length is an error, not an out-of-bounds read", "[klv]") {
	// Key 0x01, length 2^64 - 1, one value byte. The old parser wrapped its bounds check on this input.
	auto const data = make_bytes({0x01, 0x88, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xAA});
	auto reader = byte_reader{data};

	REQUIRE(reader.read_ber_oid() == 1U);
	auto const length = reader.read_ber_length();
	REQUIRE(length == std::numeric_limits<size_t>::max());

	auto const value = reader.take(*length);
	REQUIRE_FALSE(value);
	CHECK(value.error().code == error_code::unexpected_end_of_data);
}

TEST_CASE("KLV BER-OID tags decode one and two byte forms", "[klv]") {
	auto const read = [](byte_vector const& bytes) {
		auto reader = byte_reader{bytes};
		return reader.read_ber_oid();
	};

	CHECK(read(make_bytes({0x05})) == 5U);
	CHECK(read(make_bytes({0x7F})) == 127U);
	CHECK(read(make_bytes({0x81, 0x00})) == 128U);
	CHECK(read(make_bytes({0x81, 0x0F})) == 143U);
	CHECK(read(make_bytes({0x82, 0x00})) == 256U);

	SECTION("leading 0x80 is an error") {
		auto const tag = read(make_bytes({0x80, 0x01}));
		REQUIRE_FALSE(tag);
		CHECK(tag.error().code == error_code::invalid_ber_oid);
	}

	SECTION("more than 32 bits is an error") {
		auto const tag = read(make_bytes({0x81, 0x80, 0x80, 0x80, 0x80, 0x00}));
		REQUIRE_FALSE(tag);
		CHECK(tag.error().code == error_code::invalid_ber_oid);
	}

	SECTION("truncated tag is an error") {
		auto const tag = read(make_bytes({0x81}));
		REQUIRE_FALSE(tag);
		CHECK(tag.error().code == error_code::unexpected_end_of_data);
	}
}

TEST_CASE("KLV writer BER length and BER-OID read back", "[klv]") {
	for (auto const length : std::array<size_t, 8>{0, 127, 128, 255, 256, 65'535, 65'536, 16'777'216}) {
		auto out = byte_vector{};
		byte_writer{out}.put_ber_length(length);

		auto reader = byte_reader{out};
		CHECK(reader.read_ber_length() == length);
		CHECK(reader.empty());
	}

	for (auto const tag :
	     std::array<uint32_t, 7>{0, 1, 127, 128, 16'383, 16'384, std::numeric_limits<uint32_t>::max()}) {
		auto out = byte_vector{};
		byte_writer{out}.put_ber_oid(tag);

		auto reader = byte_reader{out};
		CHECK(reader.read_ber_oid() == tag);
		CHECK(reader.empty());
	}

	auto out = byte_vector{};
	byte_writer{out}.put_ber_oid(128);
	CHECK(out == make_bytes({0x81, 0x00}));
}

TEST_CASE("KLV length-prefixed write uses the long form for long values", "[klv]") {
	auto out = make_bytes({0xEE});
	auto writer = byte_writer{out};

	auto const status = writer.put_length_prefixed([&]() -> encode_result_for_test {
		for (auto i = 0; i < 200; ++i) {
			writer.put_u8(static_cast<uint8_t>(i));
		}
		return {};
	});

	REQUIRE(status);
	REQUIRE(out.size() == 1 + 2 + 200);
	CHECK(out[0] == std::byte{0xEE});
	CHECK(out[1] == std::byte{0x81});
	CHECK(out[2] == std::byte{200});
	CHECK(out[3] == std::byte{0});
	CHECK(out.back() == std::byte{199});
}

TEST_CASE("KLV length-prefixed write restores the buffer on failure", "[klv]") {
	auto out = make_bytes({0xEE});
	auto writer = byte_writer{out};

	auto const status = writer.put_length_prefixed([&]() -> encode_result_for_test {
		writer.put_u8(1);
		return std::unexpected(0);
	});

	CHECK_FALSE(status);
	CHECK(out == make_bytes({0xEE}));
}

TEST_CASE("KLV hex text converts both ways", "[klv]") {
	auto const bytes = hex("06 0e 2B34");
	CHECK(bytes == make_bytes({0x06, 0x0E, 0x2B, 0x34}));
	CHECK(to_hex(bytes) == "06 0E 2B 34");
	CHECK(to_hex(bytes, "") == "060E2B34");
	CHECK_FALSE(from_hex("0G"));
	CHECK_FALSE(from_hex("123"));
}

TEST_CASE("KLV universal key literal and matching", "[klv]") {
	static constexpr auto key = ul("06 0E 2B 34 02 0B 01 01 0E 01 03 01 01 00 00 00");
	static_assert(key[0] == std::byte{0x06} && key[15] == std::byte{0x00});

	auto other_version = byte_vector(key.begin(), key.end());
	other_version[7] = std::byte{0x02};
	CHECK(key_matches(key, other_version));

	auto other_key = byte_vector(key.begin(), key.end());
	other_key[8] = std::byte{0x0F};
	CHECK_FALSE(key_matches(key, other_key));

	CHECK_FALSE(key_matches(key, byte_span{other_key}.first(15)));
}

TEST_CASE("KLV packet checksum sums big-endian byte pairs", "[klv]") {
	CHECK(packet_checksum(make_bytes({0x01, 0x02, 0x03})) == 0x0402);
	CHECK(packet_checksum(make_bytes({0xFF, 0xFF, 0x00, 0x02})) == 0x0001);
	CHECK(packet_checksum({}) == 0);
}

TEST_CASE("KLV CRC functions give the standard check values", "[klv]") {
	// An empty input gives the initial value. The check value is the CRC of the ASCII text "123456789".
	auto const text = hex("31 32 33 34 35 36 37 38 39");
	STATIC_CHECK(crc16_ccitt({}) == 0x1D0F);
	CHECK(crc16_ccitt(text) == 0xE5CC);
	CHECK(crc32_mpeg2(text) == 0x0376'E6E7);
}
