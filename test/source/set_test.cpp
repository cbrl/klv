#include <catch2/catch_test_macros.hpp>

#include <concepts>
#include <cstdint>
#include <expected>
#include <string>
#include <string_view>
#include <vector>

#include "klv_test_util.hpp"
#include "klv/misb0102.hpp"
#include "klv/misb0601.hpp"
#include "klv/set.hpp"

using namespace klv;
using klv_test::append_field;
using klv_test::make_bytes;

namespace sample {

struct count {
	static constexpr uint32_t tag = 1;
	static constexpr std::string_view name = "Count";
	static constexpr codec::integer<uint8_t> format{};
};

struct label {
	static constexpr uint32_t tag = 2;
	static constexpr std::string_view name = "Label";
	static constexpr codec::string format{.max_length = 8};
};

struct angle {
	static constexpr uint32_t tag = 3;
	static constexpr std::string_view name = "Angle";
	static constexpr std::string_view units = "deg";
	static constexpr codec::mapped<int16_t> format{-20.0, 20.0, codec::sentinel::reserve_min};
};

struct high_tag {
	static constexpr uint32_t tag = 200;  // two-byte BER-OID: 0x81 0x48
	static constexpr std::string_view name = "High Tag";
	static constexpr codec::integer<uint16_t> format{};
};

struct inner_ls {
	static constexpr std::string_view name = "Inner";
	using fields = field_list<count>;
};

struct inner {
	static constexpr uint32_t tag = 4;
	static constexpr std::string_view name = "Inner Set";
	static constexpr codec::nested<inner_ls> format{};
};

/// Encode order: label, count, angle, inner, high_tag
struct sample_ls {
	static constexpr std::string_view name = "Sample";
	using fields = field_list<label, count, angle, inner, high_tag>;
};

struct note {
	static constexpr uint32_t tag = 5;
	static constexpr std::string_view name = "Note";
	static constexpr codec::repeated<codec::string> format{};
};

struct inners {
	static constexpr uint32_t tag = 6;
	static constexpr std::string_view name = "Inners";
	static constexpr codec::repeated<codec::nested<inner_ls>> format{};
};

/// A set with repeated fields
struct repeated_ls {
	static constexpr std::string_view name = "Repeated";
	using fields = field_list<count, note, inners>;
};

}  // namespace sample

using sample_record = record<sample::sample_ls>;

// ---- Compile-time checks ----

static_assert(field_descriptor<sample::angle>);
static_assert(local_set_definition<sample::sample_ls>);
static_assert(member_of<sample::count, sample::sample_ls>);
static_assert(!member_of<misb0601::mission_id, sample::sample_ls>);
static_assert(units_of<sample::angle>() == "deg");
static_assert(units_of<sample::count>().empty());

template<typename Record, typename E>
concept can_get = requires(Record const& rec) { rec.template get<E>(); };

// A field of another standard does not compile on a record. The old library returned the 0601 Mission ID here.
static_assert(can_get<record<misb0601::uas_datalink_ls>, misb0601::mission_id>);
static_assert(!can_get<record<misb0601::uas_datalink_ls>, misb0102::classifying_country>);

TEST_CASE("KLV set decodes known, unknown, and out-of-order fields", "[klv]") {
	auto data = byte_vector{};
	append_field(data, 3, make_bytes({0x80, 0x01}));  // angle -20
	append_field(data, 99, make_bytes({0xAA}));  // unknown
	append_field(data, 1, make_bytes({0x07}));  // count
	append_field(data, 200, make_bytes({0x12, 0x34}));

	auto const rec = decode_set<sample::sample_ls>(data);
	REQUIRE(rec);

	CHECK(rec->get<sample::count>() == uint8_t{7});
	CHECK(rec->get<sample::angle>() == -20.0);
	CHECK(rec->get<sample::high_tag>() == uint16_t{0x1234});
	CHECK(rec->get<sample::label>() == std::unexpected(field_error::absent));
	CHECK_FALSE(rec->has<sample::label>());

	REQUIRE(rec->unknown().size() == 1);
	CHECK(rec->unknown()[0] == unknown_field{.tag = 99, .value = make_bytes({0xAA})});

	SECTION("keep_unknown = false drops unknown fields") {
		auto const without = decode_set<sample::sample_ls>(data, {.keep_unknown = false});
		REQUIRE(without);
		CHECK(without->unknown().empty());
	}
}

TEST_CASE("KLV set reads two-byte BER-OID tags", "[klv]") {
	auto const data = make_bytes({0x81, 0x48, 0x02, 0xBE, 0xEF, 0x01, 0x01, 0x05});

	auto const rec = decode_set<sample::sample_ls>(data);
	REQUIRE(rec);
	CHECK(rec->get<sample::high_tag>() == uint16_t{0xBEEF});
	CHECK(rec->get<sample::count>() == uint8_t{5});
}

TEST_CASE("KLV truncated set is an error", "[klv]") {
	auto data = byte_vector{};
	append_field(data, 1, make_bytes({0x07}));
	data.push_back(std::byte{0x02});  // label tag
	data.push_back(std::byte{0x05});  // length 5
	data.push_back(std::byte{'a'});  // only one value byte

	auto const rec = decode_set<sample::sample_ls>(data);
	REQUIRE_FALSE(rec);
	CHECK(rec.error().code == error_code::unexpected_end_of_data);
	CHECK(rec.error().offset == 5);
	CHECK(rec.error().message.starts_with("Sample: "));
}

TEST_CASE("KLV field codec errors mark only that field", "[klv]") {
	auto data = byte_vector{};
	append_field(data, 3, make_bytes({0x80, 0x00}));  // error value
	append_field(data, 1, make_bytes({0x01, 0x02}));  // too long for uint8
	append_field(data, 2, make_bytes({'o', 'k'}));

	auto const rec = decode_set<sample::sample_ls>(data);
	REQUIRE(rec);
	CHECK(rec->get<sample::angle>() == std::unexpected(field_error::error_indicator));
	CHECK(rec->get<sample::count>() == std::unexpected(field_error::bad_length));
	CHECK(rec->get<sample::label>() == std::string{"ok"});
}

TEST_CASE("KLV set encodes in definition order and decodes back", "[klv]") {
	auto rec = sample_record{};
	rec.set<sample::high_tag>(0x0102);
	rec.set<sample::angle>(5.0);
	rec.set<sample::count>(3);
	rec.set<sample::label>("abc");
	rec.add_unknown({.tag = 77, .value = make_bytes({0x55})});

	auto const bytes = encode_set(rec);
	REQUIRE(bytes);

	auto expected = byte_vector{};
	append_field(expected, 2, make_bytes({'a', 'b', 'c'}));
	append_field(expected, 1, make_bytes({0x03}));
	append_field(expected, 3, make_bytes({0x20, 0x00}));  // round(25 * 65534 / 40) - 32767 = 8192
	append_field(expected, 200, make_bytes({0x01, 0x02}));
	append_field(expected, 77, make_bytes({0x55}));
	CHECK(*bytes == expected);

	auto const decoded = decode_set<sample::sample_ls>(*bytes);
	REQUIRE(decoded);
	CHECK(decoded->get<sample::label>() == std::string{"abc"});
	CHECK(decoded->get<sample::count>() == uint8_t{3});
	CHECK(decoded->get<sample::high_tag>() == uint16_t{0x0102});
	CHECK(decoded->unknown().size() == 1);
	CHECK(encode_set(*decoded) == *bytes);
}

TEST_CASE("KLV set encodes the error value and skips absent fields", "[klv]") {
	auto rec = sample_record{};
	rec.set_error_indicator<sample::angle>();
	rec.set<sample::count>(1);
	rec.erase<sample::count>();

	auto expected = byte_vector{};
	append_field(expected, 3, make_bytes({0x80, 0x00}));
	CHECK(encode_set(rec) == expected);
}

TEST_CASE("KLV set encode failure names the field and keeps the buffer", "[klv]") {
	auto rec = sample_record{};
	rec.set<sample::label>("longer than eight");

	auto out = make_bytes({0xEE});
	auto const status = encode_set(rec, out);
	REQUIRE_FALSE(status);
	CHECK(status.error().code == error_code::invalid_value);
	CHECK(status.error().message.find("'Label'") != std::string::npos);
	CHECK(out == make_bytes({0xEE}));
}

TEST_CASE("KLV nested set decodes and encodes", "[klv]") {
	auto inner_bytes = byte_vector{};
	append_field(inner_bytes, 1, make_bytes({0x09}));

	auto data = byte_vector{};
	append_field(data, 4, inner_bytes);

	auto const rec = decode_set<sample::sample_ls>(data);
	REQUIRE(rec);
	auto const& inner = rec->get<sample::inner>();
	REQUIRE(inner);
	CHECK(inner->get<sample::count>() == uint8_t{9});
	CHECK(encode_set(*rec) == data);

	SECTION("a malformed nested set marks the field") {
		auto bad = byte_vector{};
		append_field(bad, 4, make_bytes({0x01, 0x05, 0x00}));
		auto const with_bad = decode_set<sample::sample_ls>(bad);
		REQUIRE(with_bad);
		CHECK(with_bad->get<sample::inner>() == std::unexpected(field_error::invalid_value));
	}
}

TEST_CASE("KLV for_each_field visits fields in definition order", "[klv]") {
	auto rec = sample_record{};
	rec.set<sample::count>(1);

	auto names = std::vector<std::string_view>{};
	auto present = 0;
	for_each_field(rec, [&]<typename E>(std::type_identity<E>, auto const& value) {
		names.push_back(E::name);
		present += value.has_value() ? 1 : 0;
	});

	CHECK(names == std::vector<std::string_view>{"Label", "Count", "Angle", "Inner Set", "High Tag"});
	CHECK(present == 1);
}

TEST_CASE("KLV describe lists display text with nested prefixes", "[klv]") {
	auto inner = record<sample::inner_ls>{};
	inner.set<sample::count>(9);

	auto rec = sample_record{};
	rec.set<sample::angle>(-20.0);
	rec.set<sample::inner>(inner);
	rec.assign<sample::count>(std::unexpected(field_error::bad_length));
	rec.add_unknown({.tag = 77, .value = make_bytes({0x55})});

	auto const text = describe(rec);
	REQUIRE(text.size() == 4);

	CHECK(text[0].name == "Count");
	CHECK(text[0].value == std::unexpected(field_error::bad_length));

	CHECK(text[1].name == "Angle");
	CHECK(text[1].units == "deg");
	CHECK(text[1].value == std::string{"-20.000000"});

	CHECK(text[2].name == "Inner Set.Count");
	CHECK(text[2].value == std::string{"9"});

	CHECK(text[3].name == "Unknown tag 77");
	CHECK(text[3].value == std::string{"55"});

	CHECK(describe(rec, true).size() == 6);
}

TEST_CASE("KLV repeated field collects each occurrence in order", "[klv]") {
	auto inner_7 = byte_vector{};
	append_field(inner_7, 1, make_bytes({0x07}));
	auto inner_8 = byte_vector{};
	append_field(inner_8, 1, make_bytes({0x08}));

	// Definition order: count, note, inners
	auto data = byte_vector{};
	append_field(data, 1, make_bytes({0x05}));
	append_field(data, 5, make_bytes({'h', 'i'}));
	append_field(data, 5, make_bytes({'y', 'o', 'u'}));
	append_field(data, 6, inner_7);
	append_field(data, 6, inner_8);

	auto const rec = decode_set<sample::repeated_ls>(data);
	REQUIRE(rec);
	CHECK(rec->get<sample::note>() == std::vector<std::string>{"hi", "you"});
	auto const& inners = rec->get<sample::inners>();
	REQUIRE(inners);
	REQUIRE(inners->size() == 2);
	CHECK((*inners)[0].get<sample::count>() == uint8_t{7});
	CHECK((*inners)[1].get<sample::count>() == uint8_t{8});

	// Encoding writes one field for each item, at the position of the field
	CHECK(encode_set(*rec) == data);

	SECTION("add appends items") {
		auto built = record<sample::repeated_ls>{};
		built.set<sample::count>(5);
		built.add<sample::note>("hi");
		built.add<sample::note>("you");
		built.add<sample::inners>((*inners)[0]);
		built.add<sample::inners>((*inners)[1]);
		CHECK(built == *rec);
	}

	SECTION("an occurrence that does not decode marks the field") {
		auto bad = byte_vector{};
		append_field(bad, 6, make_bytes({0x01, 0x05, 0x00}));
		append_field(bad, 6, inner_8);
		auto const with_bad = decode_set<sample::repeated_ls>(bad);
		REQUIRE(with_bad);
		CHECK(with_bad->get<sample::inners>() == std::unexpected(field_error::invalid_value));
	}

	SECTION("describe lists each nested set with its index") {
		auto const text = describe(*rec);
		REQUIRE(text.size() == 4);
		CHECK(text[0].name == "Count");
		CHECK(text[1].name == "Note");
		CHECK(text[1].value == std::string{"[hi, you]"});
		CHECK(text[2].name == "Inners[0].Count");
		CHECK(text[2].value == std::string{"7"});
		CHECK(text[3].name == "Inners[1].Count");
	}
}

TEST_CASE("KLV zero-length field is empty for each format", "[klv]") {
	// Definition order: label, count, angle, inner
	auto data = byte_vector{};
	append_field(data, 2, {});
	append_field(data, 1, {});
	append_field(data, 3, {});
	append_field(data, 4, {});

	auto const rec = decode_set<sample::sample_ls>(data);
	REQUIRE(rec);
	CHECK(rec->get<sample::label>() == std::unexpected(field_error::empty));
	CHECK(rec->get<sample::count>() == std::unexpected(field_error::empty));
	CHECK(rec->get<sample::angle>() == std::unexpected(field_error::empty));
	CHECK(rec->get<sample::inner>() == std::unexpected(field_error::empty));
	CHECK(rec->get<sample::high_tag>() == std::unexpected(field_error::absent));

	// Encoding writes an empty field with a length of zero
	CHECK(encode_set(*rec) == data);

	auto built = record<sample::sample_ls>{};
	built.set_empty<sample::label>();
	built.set_empty<sample::count>();
	built.set_empty<sample::angle>();
	built.set_empty<sample::inner>();
	CHECK(built == *rec);

	auto const text = describe(*rec);
	REQUIRE(text.size() == 4);
	CHECK(text[0].value == std::unexpected(field_error::empty));
	CHECK(to_string(field_error::empty) == "empty");
}

TEST_CASE("KLV zero-length occurrence of a repeated field", "[klv]") {
	SECTION("without other occurrences, the field is empty") {
		auto data = byte_vector{};
		append_field(data, 5, {});
		auto const rec = decode_set<sample::repeated_ls>(data);
		REQUIRE(rec);
		CHECK(rec->get<sample::note>() == std::unexpected(field_error::empty));
		CHECK(encode_set(*rec) == data);
	}

	SECTION("with other occurrences, the zero-length occurrence adds no item") {
		auto data = byte_vector{};
		append_field(data, 5, make_bytes({'h', 'i'}));
		append_field(data, 5, {});
		append_field(data, 5, make_bytes({'y', 'o', 'u'}));
		auto const rec = decode_set<sample::repeated_ls>(data);
		REQUIRE(rec);
		CHECK(rec->get<sample::note>() == std::vector<std::string>{"hi", "you"});
	}
}
