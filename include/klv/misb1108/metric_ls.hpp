/**
 * @file klv/misb1108/metric_ls.hpp
 * @brief MISB ST 1108 Metric Local Set (tag 4): one metric
 */

#pragma once

#include "klv/bytes.hpp"
#include "klv/codec.hpp"
#include "klv/codecs/pack.hpp"
#include "klv/set.hpp"

#include <cstddef>
#include <cstdint>
#include <expected>
#include <format>
#include <optional>
#include <string>
#include <string_view>

namespace klv::misb1108 {

/// Organization that is responsible for the calculation of a metric (Metric LS tag 3)
struct implementer {
	std::string organization;
	std::optional<std::string> subgroup;  ///< Empty when the text has no record separator

	friend auto operator==(implementer const&, implementer const&) -> bool = default;
};

/**
 * @brief Metric Implementer: the organization, the record separator (0x1E), then the subgroup
 *
 * ST 1108 requires the separator. Decoding accepts text without it, and gives an empty `subgroup`. Encoding fails
 * with `field_error::bad_length` when the text is longer than 128 bytes. It fails with `field_error::invalid_value`
 * when the organization holds the separator.
 */
struct implementer_codec {
	using value_type = implementer;

	static constexpr char separator = '\x1E';
	static constexpr size_t max_length = 128;

	[[nodiscard]]
	auto decode(byte_span in) const -> field<implementer> {
		auto const text = codec::string{}.decode(in);
		if (!text) {
			return std::unexpected(text.error());
		}
		auto const split = text->find(separator);
		if (split == std::string::npos) {
			return implementer{.organization = *text, .subgroup = std::nullopt};
		}
		return implementer{.organization = text->substr(0, split), .subgroup = text->substr(split + 1)};
	}

	[[nodiscard]]
	auto encode(implementer const& value, byte_writer& out) const -> encode_result {
		if (value.organization.find(separator) != std::string::npos) {
			return std::unexpected(field_error::invalid_value);
		}
		auto const length = value.organization.size() + (value.subgroup ? 1 + value.subgroup->size() : 0);
		if (length > max_length) {
			return std::unexpected(field_error::bad_length);
		}
		detail::put_text(value.organization, out);
		if (value.subgroup) {
			out.put_u8(static_cast<uint8_t>(separator));
			detail::put_text(*value.subgroup, out);
		}
		return {};
	}

	[[nodiscard]]
	auto to_string(implementer const& value) const -> std::string {
		if (!value.subgroup) {
			return std::format("{{organization: \"{}\"}}", value.organization);
		}
		return std::format("{{organization: \"{}\", subgroup: \"{}\"}}", value.organization, *value.subgroup);
	}
};

// Metric Local Set items

/// Name of the metric, for example "VNIIRS", "GSD", "RER", "PSNR", or "SSIM" (ST 1108 Table 6)
struct metric_name {
	static constexpr uint32_t tag = 1;
	static constexpr std::string_view name = "Metric Name";
	static constexpr codec::string format{.max_length = 128};
};

/// "Human" for a metric that a person assesses. Otherwise the configuration identifier of the algorithm.
struct metric_version {
	static constexpr uint32_t tag = 2;
	static constexpr std::string_view name = "Metric Version";
	static constexpr codec::string format{.max_length = 32};
};

struct metric_implementer {
	static constexpr uint32_t tag = 3;
	static constexpr std::string_view name = "Metric Implementer";
	static constexpr implementer_codec format{};
};

/// Other conditions of the calculation, for repeatability
struct metric_parameters {
	static constexpr uint32_t tag = 4;
	static constexpr std::string_view name = "Metric Parameters";
	static constexpr codec::string format{.max_length = 128};
};

/// Time of the assessment. It can be after the Metric Period.
struct metric_time {
	static constexpr uint32_t tag = 5;
	static constexpr std::string_view name = "Metric Time";
	static constexpr codec::timestamp format{};
};

struct metric_value {
	static constexpr uint32_t tag = 6;
	static constexpr std::string_view name = "Metric Value";
	static constexpr codec::ieee754_variable format{};
};

/// SI units of the metric. Without this item, the metric has the default units of ST 1108 Table 6.
struct metric_units {
	static constexpr uint32_t tag = 7;
	static constexpr std::string_view name = "Metric Units";
	static constexpr codec::string format{.max_length = 128};
};

/// MISB ST 1108 Metric Local Set: one metric
struct metric_ls {
	static constexpr std::string_view name = "Metric Local Set";

	using fields = field_list<
	    metric_name,
	    metric_version,
	    metric_implementer,
	    metric_parameters,
	    metric_time,
	    metric_value,
	    metric_units>;
};

}  // namespace klv::misb1108
