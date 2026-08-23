/**
 * @file misb1108.hpp
 * @brief MISB ST 1108 - Interpretability and Quality Local Set
 *
 * The set records image quality metrics (for example VNIIRS or GSD) and the compression of a Motion Imagery stream.
 * The values apply at one point of the imaging workflow. The set arrives as a stand-alone packet that ends with a
 * CRC-16-CCITT checksum.
 *
 * Each Metric Local Set (tag 4) holds one metric. The set can hold tag 4 more than once.
 */

#pragma once

#include "klv/bytes.hpp"
#include "klv/codec.hpp"
#include "klv/codecs/pack.hpp"
#include "klv/set.hpp"

#include <chrono>
#include <cstdint>
#include <expected>
#include <format>
#include <initializer_list>
#include <limits>
#include <optional>
#include <string>
#include <string_view>

namespace klv::misb1108 {

/// Location in the imaging workflow where the metrics are calculated (tag 1, ST 1108 Table 2)
enum class assessment_location : uint8_t {
	sensor = 1,
	sensor_encoder = 2,
	gcs_received = 3,
	gcs_transmit = 4,
	library_archive = 5,
};

[[nodiscard]]
constexpr auto to_string(assessment_location value) -> std::string_view {
	using enum assessment_location;
	switch (value) {
		case sensor:
			return "Sensor";
		case sensor_encoder:
			return "Sensor Encoder";
		case gcs_received:
			return "GCS Received";
		case gcs_transmit:
			return "GCS Transmit";
		case library_archive:
			return "Library/Archive";
	}
	return {};
}

/// Compression of the stream (tag 5)
enum class compression_method : uint8_t {
	uncompressed = 0,
	h262 = 1,
	h264 = 2,
	h265 = 3,
	jpeg2000 = 4,
};

[[nodiscard]]
constexpr auto to_string(compression_method value) -> std::string_view {
	using enum compression_method;
	switch (value) {
		case uncompressed:
			return "Uncompressed";
		case h262:
			return "H.262";
		case h264:
			return "H.264";
		case h265:
			return "H.265";
		case jpeg2000:
			return "JPEG2000";
	}
	return {};
}

/// Profile of the compression (tag 6)
enum class compression_profile_kind : uint8_t {
	not_applicable = 0,
	main = 1,
	main_10 = 2,
	constrained_baseline = 3,
	high = 4,
	main_422_12 = 5,
	main_444_12 = 6,
	high_422 = 7,
	high_444_predictive = 8,
};

[[nodiscard]]
constexpr auto to_string(compression_profile_kind value) -> std::string_view {
	using enum compression_profile_kind;
	switch (value) {
		case not_applicable:
			return "N/A (uncompressed)";
		case main:
			return "Main (H.264)";
		case main_10:
			return "Main 10 (H.265)";
		case constrained_baseline:
			return "Constrained Baseline (H.264)";
		case high:
			return "High (H.264)";
		case main_422_12:
			return "Main 4:2:2 12 (H.265)";
		case main_444_12:
			return "Main 4:4:4 12 (H.265)";
		case high_422:
			return "High 4:2:2 (H.264)";
		case high_444_predictive:
			return "High 4:4:4 Predictive (H.264)";
	}
	return {};
}

/// Period of the frames that a metric covers (tag 2)
struct metric_period {
	timestamp start{};  ///< Precision Time Stamp of the first frame
	std::chrono::microseconds offset{0};  ///< Time from the first frame to the last frame, 0 to 2^32 - 1 us

	/// Time of the last frame
	[[nodiscard]]
	auto end() const -> timestamp {
		return start + offset;
	}

	friend auto operator==(metric_period const&, metric_period const&) -> bool = default;
};

/// Metric Period Pack: an 8-byte Precision Time Stamp, then a 4-byte offset in microseconds
struct metric_period_codec {
	using value_type = metric_period;

	[[nodiscard]]
	auto decode(byte_span in) const -> field<metric_period> {
		if (in.size() != 12) {
			return std::unexpected(field_error::bad_length);
		}
		auto const start = codec::timestamp{}.decode(in.first(8));
		if (!start) {
			return std::unexpected(start.error());
		}
		auto const offset = read_big_endian<uint32_t>(in.subspan(8));
		return metric_period{.start = *start, .offset = std::chrono::microseconds{offset}};
	}

	[[nodiscard]]
	auto encode(metric_period const& value, byte_writer& out) const -> encode_result {
		auto const offset = value.offset.count();
		if (offset < 0 || offset > std::numeric_limits<uint32_t>::max()) {
			return std::unexpected(field_error::out_of_range);
		}
		auto const start = codec::timestamp{}.encode(value.start, out);
		if (!start) {
			return start;
		}
		out.put_big_endian(static_cast<uint32_t>(offset));
		return {};
	}

	[[nodiscard]]
	auto to_string(metric_period const& value) const -> std::string {
		return std::format(
		    "{{start: {}, offset: {} us}}", codec::timestamp{}.to_string(value.start), value.offset.count()
		);
	}
};

/**
 * @brief Sub-region of the frame that the metrics cover (tag 3)
 *
 * C1 is the upper left corner and C2 is the lower right corner. Rows and columns start at zero. Without this pack,
 * the metrics cover the whole frame.
 */
struct window_corners {
	uint32_t c1_row = 0;
	uint32_t c1_column = 0;
	uint32_t c2_row = 0;
	uint32_t c2_column = 0;

	friend auto operator==(window_corners const&, window_corners const&) -> bool = default;
};

/// Window Corners Pack: four BER-OID values in the order C1 row, C1 column, C2 row, C2 column
struct window_corners_codec {
	using value_type = window_corners;

	[[nodiscard]]
	auto decode(byte_span in) const -> field<window_corners> {
		auto reader = byte_reader{in};
		auto value = window_corners{};
		for (auto* item : {&value.c1_row, &value.c1_column, &value.c2_row, &value.c2_column}) {
			auto const number = detail::read_oid(reader);
			if (!number) {
				return std::unexpected(number.error());
			}
			*item = *number;
		}
		return detail::finish(reader, value);
	}

	[[nodiscard]]
	auto encode(window_corners const& value, byte_writer& out) const -> encode_result {
		for (auto const item : {value.c1_row, value.c1_column, value.c2_row, value.c2_column}) {
			out.put_ber_oid(item);
		}
		return {};
	}

	[[nodiscard]]
	auto to_string(window_corners const& value) const -> std::string {
		return std::format(
		    "{{c1_row: {}, c1_column: {}, c2_row: {}, c2_column: {}}}",
		    value.c1_row,
		    value.c1_column,
		    value.c2_row,
		    value.c2_column
		);
	}
};

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

// Interpretability and Quality Local Set items

struct assessment_point {
	static constexpr uint32_t tag = 1;
	static constexpr std::string_view name = "Assessment Point";
	static constexpr codec::enumeration<assessment_location> format{.name_of = &to_string};
};

struct metric_period_pack {
	static constexpr uint32_t tag = 2;
	static constexpr std::string_view name = "Metric Period Pack";
	static constexpr metric_period_codec format{};
};

struct window_corners_pack {
	static constexpr uint32_t tag = 3;
	static constexpr std::string_view name = "Window Corners Pack";
	static constexpr window_corners_codec format{};
};

/// One item for each metric
struct metric_local_set {
	static constexpr uint32_t tag = 4;
	static constexpr std::string_view name = "Metric Local Set";
	static constexpr codec::repeated<codec::nested<metric_ls>> format{};
};

struct compression_type {
	static constexpr uint32_t tag = 5;
	static constexpr std::string_view name = "Compression Type";
	static constexpr codec::enumeration<compression_method> format{.name_of = &to_string};
};

struct compression_profile {
	static constexpr uint32_t tag = 6;
	static constexpr std::string_view name = "Compression Profile";
	static constexpr codec::enumeration<compression_profile_kind> format{.name_of = &to_string};
};

/// Level of the compression, for example "4.1" or "5" (H.264 and H.265) or "HL" (H.262). "N/A" when uncompressed.
struct compression_level {
	static constexpr uint32_t tag = 7;
	static constexpr std::string_view name = "Compression Level";
	static constexpr codec::string format{.max_length = 3};
};

/// Source size divided by compressed size. 1.0 when uncompressed.
struct compression_ratio {
	static constexpr uint32_t tag = 8;
	static constexpr std::string_view name = "Compression Ratio";
	static constexpr codec::ieee754_variable format{};
};

struct stream_bitrate {
	static constexpr uint32_t tag = 9;
	static constexpr std::string_view name = "Stream Bitrate";
	static constexpr std::string_view units = "kbit/s";
	static constexpr codec::integer<uint16_t> format{.length = codec::variable_length};
};

/// Version of ST 1108, for example 4 for ST 1108.4
struct document_version {
	static constexpr uint32_t tag = 10;
	static constexpr std::string_view name = "Document Version";
	static constexpr codec::integer<uint8_t> format{.length = codec::variable_length};
};

/// CRC-16-CCITT of the packet, from the key to the length of this item
struct checksum {
	static constexpr uint32_t tag = 11;
	static constexpr std::string_view name = "CRC-16-CCITT";
	static constexpr codec::integer<uint16_t> format{};
};

/**
 * @brief MISB ST 1108 Interpretability and Quality Local Set
 *
 * Packet decoding verifies the CRC-16-CCITT checksum (tag 11), and packet encoding computes it.
 */
struct interpretability_quality_ls {
	static constexpr std::string_view name = "Interpretability and Quality Local Set";
	static constexpr universal_key key = ul("06 0E 2B 34 02 03 01 01 0E 01 03 03 1C 00 00 00");

	using checksum_field = checksum;
	static constexpr auto checksum_function = crc16_ccitt;

	using fields = field_list<
	    assessment_point,
	    metric_period_pack,
	    window_corners_pack,
	    metric_local_set,
	    compression_type,
	    compression_profile,
	    compression_level,
	    compression_ratio,
	    stream_bitrate,
	    document_version,
	    checksum>;
};

}  // namespace klv::misb1108
