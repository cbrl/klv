/**
 * @file klv/misb0601/packs/countries.hpp
 * @brief MISB ST 0601 Country Codes (tag 122)
 */

#pragma once

#include "klv/bytes.hpp"
#include "klv/codec.hpp"
#include "klv/codecs/pack.hpp"
#include "klv/misb0102/enumerations.hpp"

#include <cstdint>
#include <expected>
#include <format>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

namespace klv::misb0601 {

/// Countries of the platform and its operation. An absent country has no value.
struct countries {
	misb0102::classifying_country_coding coding_method = misb0102::classifying_country_coding::iso_3166_two_letter;
	std::optional<std::string> overflight_country;
	std::optional<std::string> operator_country;
	std::optional<std::string> country_of_manufacture;

	friend auto operator==(countries const&, countries const&) -> bool = default;
};

/**
 * @brief Country Codes: four length-prefixed elements (coding method, then three country codes)
 *
 * A zero length marks an absent country. The writer can omit the last two elements, or the last element.
 */
struct countries_codec {
	using value_type = countries;

	[[nodiscard]]
	auto decode(byte_span in) const -> field<countries> {
		auto reader = byte_reader{in};
		auto const method = detail::read_prefixed(reader);
		if (!method) {
			return std::unexpected(method.error());
		}
		if (method->empty() || method->size() > sizeof(uint64_t)) {
			return std::unexpected(field_error::bad_length);
		}
		auto const raw_method = read_big_endian<uint64_t>(*method);
		if (raw_method > 0xFF) {
			return std::unexpected(field_error::invalid_value);
		}

		auto result = countries{
		    .coding_method = static_cast<misb0102::classifying_country_coding>(raw_method),
		    .overflight_country = std::nullopt,
		    .operator_country = std::nullopt,
		    .country_of_manufacture = std::nullopt,
		};
		for (auto* country : {&result.overflight_country, &result.operator_country, &result.country_of_manufacture}) {
			if (reader.empty()) {
				break;
			}
			auto const code = detail::read_prefixed(reader);
			if (!code) {
				return std::unexpected(code.error());
			}
			if (!code->empty()) {
				*country = detail::to_text(*code);
			}
		}
		return detail::finish(reader, std::move(result));
	}

	[[nodiscard]]
	auto encode(countries const& value, byte_writer& out) const -> encode_result {
		out.put_ber_length(1);
		out.put_u8(static_cast<uint8_t>(value.coding_method));

		auto const put_country = [&](std::optional<std::string> const& country) {
			detail::put_prefixed_text(country ? std::string_view{*country} : std::string_view{}, out);
		};

		put_country(value.overflight_country);
		if (value.operator_country || value.country_of_manufacture) {
			put_country(value.operator_country);
		}
		if (value.country_of_manufacture) {
			put_country(value.country_of_manufacture);
		}
		return {};
	}

	[[nodiscard]]
	auto to_string(countries const& value) const -> std::string {
		auto const country = [](std::optional<std::string> const& code) {
			return code ? *code : std::string{"absent"};
		};
		return std::format(
		    "{{coding method: {}, overflight: {}, operator: {}, manufacture: {}}}",
		    codec::enumeration<misb0102::classifying_country_coding>{}.to_string(value.coding_method),
		    country(value.overflight_country),
		    country(value.operator_country),
		    country(value.country_of_manufacture)
		);
	}
};

}  // namespace klv::misb0601
