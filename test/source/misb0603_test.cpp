// The conversion examples come from MISB ST 0603.5 Tables 1 and 2.

#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <cstdint>
#include <expected>
#include <limits>
#include <optional>

#include "klv_test_util.hpp"
#include "klv/misb0603.hpp"
#include "klv/packet.hpp"

using namespace klv;
using klv_test::hex;
using klv_test::make_bytes;
using klv_test::micros;

namespace mt = klv::misb0603;

namespace {

[[nodiscard]]
auto nanos(int64_t count) -> nano_timestamp {
	return nano_timestamp{std::chrono::nanoseconds{count}};
}

template<typename Codec>
[[nodiscard]]
auto encode(Codec const& codec, typename Codec::value_type const& value) -> field<byte_vector> {
	auto out = byte_vector{};
	auto writer = byte_writer{out};
	return codec.encode(value, writer).transform([&] { return out; });
}

}  // namespace

TEST_CASE("KLV ST 0603 nano to precision conversion rounds half up", "[klv][misb0603]") {
	// Table 1
	CHECK(mt::to_precision(nanos(0x7A2C)) == micros(0x1F));
	CHECK(mt::to_precision(nanos(0x92'11DF)) == micros(0x2565));
	CHECK(mt::to_precision(nanos(0x92'1118)) == micros(0x2565));

	CHECK(mt::to_precision(nanos(1'499)) == micros(1));
	CHECK(mt::to_precision(nanos(1'500)) == micros(2));
	CHECK(mt::to_precision(nanos(std::numeric_limits<int64_t>::max())) == micros(9'223'372'036'854'776));
}

TEST_CASE("KLV ST 0603 precision to nano conversion keeps the value", "[klv][misb0603]") {
	// Table 2
	CHECK(mt::to_nano(micros(0x1F)) == nanos(0x7918));
	CHECK(mt::to_nano(micros(0x2565)) == nanos(0x92'1288));

	auto const now = micros(1'700'000'000'123'456);
	auto const nano = mt::to_nano(now);
	REQUIRE(nano);
	CHECK(mt::to_precision(*nano) == now);

	constexpr auto last = std::numeric_limits<int64_t>::max() / 1'000;
	CHECK(mt::to_nano(micros(last)) == nanos(last * 1'000));
	CHECK(mt::to_nano(micros(last + 1)) == std::nullopt);
}

TEST_CASE("KLV ST 0603 Nano Precision Time Stamp codec", "[klv][misb0603]") {
	constexpr auto format = codec::nano_timestamp{};
	auto const value = nanos(1'700'000'000'123'456'789);

	auto const bytes = encode(format, value);
	CHECK(bytes == hex("17 97 9C FE 3D 85 CD 15"));
	CHECK(format.decode(hex("17 97 9C FE 3D 85 CD 15")) == value);
	CHECK(format.to_string(value) == "2023-11-14T22:13:20.123456789Z");

	CHECK(encode(format, nanos(0x7A2C)) == hex("00 00 00 00 00 00 7A 2C"));
	CHECK(format.decode(hex("00 7A 2C")) == std::unexpected(field_error::bad_length));
	CHECK(format.decode(hex("80 00 00 00 00 00 00 00")) == std::unexpected(field_error::out_of_range));
	CHECK(encode(format, nanos(-1)) == std::unexpected(field_error::out_of_range));
}

TEST_CASE("KLV ST 0603 Time Status", "[klv][misb0603]") {
	constexpr auto format = mt::time_status_codec{};

	CHECK(encode(format, {}) == make_bytes({0x1F}));
	CHECK(encode(format, {.locked = false, .jump = mt::time_jump::none}) == make_bytes({0x9F}));
	CHECK(encode(format, {.locked = true, .jump = mt::time_jump::forward}) == make_bytes({0x5F}));
	CHECK(encode(format, {.locked = false, .jump = mt::time_jump::reverse}) == make_bytes({0xFF}));

	CHECK(format.decode(make_bytes({0x1F})) == mt::time_status{});
	CHECK(format.decode(make_bytes({0x7F})) == mt::time_status{.locked = true, .jump = mt::time_jump::reverse});
	CHECK(format.decode(make_bytes({0xDF})) == mt::time_status{.locked = false, .jump = mt::time_jump::forward});

	// Decoding ignores the reserved bits, and bit 5 without bit 6
	CHECK(format.decode(make_bytes({0x00})) == mt::time_status{});
	CHECK(format.decode(make_bytes({0x20})) == mt::time_status{});

	CHECK(format.decode({}) == std::unexpected(field_error::bad_length));
	CHECK(format.decode(make_bytes({0x1F, 0x1F})) == std::unexpected(field_error::bad_length));
	CHECK(
	    encode(format, {.locked = true, .jump = static_cast<mt::time_jump>(3)})
	    == std::unexpected(field_error::invalid_value)
	);
	CHECK(format.to_string({.locked = false, .jump = mt::time_jump::reverse}) == "{locked: false, jump: Reverse}");
}

TEST_CASE("KLV ST 0603 items are standalone KLV items", "[klv][misb0603]") {
	auto bytes = byte_vector{};
	klv_test::append_packet(bytes, mt::time_status_key, make_bytes({0x5F}));
	klv_test::append_packet(bytes, mt::nano_precision_time_stamp_key, hex("17 97 9C FE 3D 85 CD 15"));
	klv_test::append_packet(bytes, mt::precision_time_stamp_key, hex("00 06 0A 24 18 1E 40 00"));

	auto status = std::optional<mt::time_status>{};
	auto nano = std::optional<nano_timestamp>{};
	auto precision = std::optional<timestamp>{};
	for (auto const& packet : split_packets(bytes)) {
		REQUIRE(packet);
		if (key_matches(mt::time_status_key, packet->key)) {
			status = mt::time_status_codec{}.decode(packet->value).value();
		}
		else if (key_matches(mt::nano_precision_time_stamp_key, packet->key)) {
			nano = codec::nano_timestamp{}.decode(packet->value).value();
		}
		else if (key_matches(mt::precision_time_stamp_key, packet->key)) {
			precision = codec::timestamp{}.decode(packet->value).value();
		}
	}
	CHECK(status == mt::time_status{.locked = true, .jump = mt::time_jump::forward});
	CHECK(nano == nanos(1'700'000'000'123'456'789));
	CHECK(precision == micros(1'700'000'000'000'000));
}
