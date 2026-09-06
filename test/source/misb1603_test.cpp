#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <cstdint>
#include <expected>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

#include "klv_test_util.hpp"
#include "klv/misb1603.hpp"
#include "klv/packet.hpp"

using namespace klv;
using klv_test::hex;
using klv_test::make_bytes;

namespace tt = klv::misb1603;

using transfer_record = record<tt::time_transfer_ls>;

namespace {

/// Value of a Time Transfer Local Set with each item
constexpr auto transfer_bytes = std::string_view{
    "01 01 03 "  // Document Version: 3
    "02 01 1D "  // UTC Leap Second Offset: 29 s
    "03 01 1A "  // Time Transfer Parameters: GPS PPS, Slew Correction, synchronous to an atomic source
    "04 04 3F 80 00 00 "  // Synchronization Pulse Frequency: 1.0 Hz
    "05 03 0F 42 40 "  // Unlock Time: 1 000 000
    "06 02 03 E8 "  // Last Synchronization Difference: 1 000
    "07 04 3F 00 00 00 "  // Drift Rate: 0.5 us/s
    "08 02 02 9B "  // Signal Source Delay: 667 ns
    "09 01 64"  // Receptor Clock Uncertainty: 100
};

/// Nano Precision Time Stamp of the example pack: 2023-11-14T22:13:20.123456789Z
constexpr auto time_bytes = std::string_view{"17 97 9C FE 3D 85 CD 15"};

constexpr auto gps_slew = tt::transfer_parameters{
    .source = tt::reference_source::synchronized,
    .correction = tt::correction_method::slew,
    .method = tt::transfer_method::gps_pps,
};

[[nodiscard]]
auto example_time() -> nano_timestamp {
	return nano_timestamp{std::chrono::nanoseconds{1'700'000'000'123'456'789}};
}

[[nodiscard]]
auto make_transfer() -> transfer_record {
	auto rec = transfer_record{};
	rec.set<tt::document_version>(uint8_t{3});
	rec.set<tt::utc_leap_second_offset>(29);
	rec.set<tt::time_transfer_parameters>(gps_slew);
	rec.set<tt::synchronization_pulse_frequency>(1.0);
	rec.set<tt::unlock_time>(1'000'000U);
	rec.set<tt::last_synchronization_difference>(1'000U);
	rec.set<tt::drift_rate>(0.5);
	rec.set<tt::signal_source_delay>(667U);
	rec.set<tt::receptor_clock_uncertainty>(100U);
	return rec;
}

template<typename Codec>
[[nodiscard]]
auto encode(Codec const& codec, typename Codec::value_type const& value) -> field<byte_vector> {
	auto out = byte_vector{};
	auto writer = byte_writer{out};
	return codec.encode(value, writer).transform([&] { return out; });
}

[[nodiscard]]
auto concat(byte_vector first, byte_vector const& second) -> byte_vector {
	first.insert(first.end(), second.begin(), second.end());
	return first;
}

}  // namespace

TEST_CASE("KLV ST 1603 Time Transfer Local Set", "[klv][misb1603]") {
	auto const decoded = decode_set<tt::time_transfer_ls>(hex(transfer_bytes));
	REQUIRE(decoded);
	CHECK(decoded->unknown().empty());
	CHECK(*decoded == make_transfer());
	CHECK(encode_set(make_transfer()) == hex(transfer_bytes));

	// A larger encoding of an integer also decodes
	auto const wide = decode_set<tt::time_transfer_ls>(hex("02 04 FF FF FF E3 08 08 00 00 00 00 00 00 02 9B"));
	REQUIRE(wide);
	CHECK(wide->get<tt::utc_leap_second_offset>() == -29);
	CHECK(wide->get<tt::signal_source_delay>() == 667U);
}

TEST_CASE("KLV ST 1603 Time Transfer Parameters", "[klv][misb1603]") {
	constexpr auto format = tt::transfer_parameters_codec{};

	CHECK(format.decode(make_bytes({0x1A})) == gps_slew);
	CHECK(format.decode(make_bytes({0x00})) == tt::transfer_parameters{});
	CHECK(
	    format.decode(make_bytes({0x75}))
	    == tt::transfer_parameters{
	        .source = tt::reference_source::not_synchronized,
	        .correction = tt::correction_method::jam,
	        .method = tt::transfer_method::irig_b,
	    }
	);
	CHECK(encode(format, gps_slew) == make_bytes({0x1A}));

	// A later version can add more significant bytes
	CHECK(format.decode(make_bytes({0xFF, 0x1A})) == gps_slew);

	CHECK(format.decode({}) == std::unexpected(field_error::bad_length));
	CHECK(format.decode(make_bytes({0x03})) == std::unexpected(field_error::invalid_value));
	CHECK(format.decode(make_bytes({0x0C})) == std::unexpected(field_error::invalid_value));
	CHECK(format.decode(make_bytes({0x80})) == std::unexpected(field_error::invalid_value));
	CHECK(
	    encode(format, {.method = static_cast<tt::transfer_method>(8)}) == std::unexpected(field_error::invalid_value)
	);

	CHECK(
	    format.to_string(gps_slew)
	    == "{source: Synchronous to an Atomic Source, correction: Slew Correction, method: GPS PPS}"
	);
}

TEST_CASE("KLV ST 1603 Nano Time Transfer Pack", "[klv][misb1603]") {
	constexpr auto format = tt::nano_time_transfer_codec{};
	auto const full = tt::nano_time_transfer{.time = example_time(), .transfer = make_transfer()};

	auto const bytes = concat(hex(time_bytes), hex(transfer_bytes));
	CHECK(format.decode(bytes) == full);
	CHECK(encode(format, full) == bytes);

	SECTION("a pack of 8 bytes holds only the time stamp") {
		auto const time_only = tt::nano_time_transfer{.time = example_time(), .transfer = {}};
		CHECK(format.decode(hex(time_bytes)) == time_only);
		CHECK(encode(format, time_only) == hex(time_bytes));
		CHECK(format.to_string(time_only) == "{time: 2023-11-14T22:13:20.123456789Z, transfer: {}}");
	}

	SECTION("errors") {
		CHECK(format.decode(hex("17 97 9C FE 3D 85 CD")) == std::unexpected(field_error::bad_length));
		CHECK(format.decode(hex("80 00 00 00 00 00 00 00")) == std::unexpected(field_error::out_of_range));
		CHECK(
		    format.decode(concat(hex(time_bytes), hex("03 05 1A"))) == std::unexpected(field_error::invalid_value)
		);

		auto early = full;
		early.time = nano_timestamp{std::chrono::nanoseconds{-1}};
		CHECK(encode(format, early) == std::unexpected(field_error::out_of_range));
	}

	SECTION("a standalone KLV item") {
		auto packet_bytes = byte_vector{};
		klv_test::append_packet(packet_bytes, tt::nano_time_transfer_pack_key, bytes);
		CHECK(packet_bytes.size() == 16 + 1 + 45);

		auto found = std::optional<tt::nano_time_transfer>{};
		for (auto const& packet : split_packets(packet_bytes)) {
			REQUIRE(packet);
			if (key_matches(tt::nano_time_transfer_pack_key, packet->key)) {
				found = format.decode(packet->value).value();
			}
		}
		CHECK(found == full);
	}
}

TEST_CASE("KLV ST 1603 Nano Time Transfer Pack text", "[klv][misb1603]") {
	auto transfer = transfer_record{};
	transfer.set<tt::utc_leap_second_offset>(29);
	transfer.set<tt::signal_source_delay>(667U);
	auto const text =
	    tt::nano_time_transfer_codec{}.to_string({.time = example_time(), .transfer = std::move(transfer)});
	CHECK(
	    text
	    == std::string{
	        "{time: 2023-11-14T22:13:20.123456789Z, transfer: {UTC Leap Second Offset: 29; Signal Source Delay: 667}}"
	    }
	);
}
