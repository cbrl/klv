/**
 * @file klv/codecs/sdcc_flp.hpp
 * @brief MISB ST 1010 Standard Deviation and Cross-Correlation Floating Length Pack (SDCC-FLP)
 */

#pragma once

#include "klv/bytes/big_endian.hpp"
#include "klv/bytes/byte_reader.hpp"
#include "klv/bytes/byte_writer.hpp"
#include "klv/bytes/types.hpp"
#include "klv/codecs/imapb.hpp"
#include "klv/codecs/pack.hpp"
#include "klv/codecs/text.hpp"
#include "klv/field.hpp"

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <format>
#include <limits>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace klv {

/// IMAPB range of the standard deviation of one item. The parent standard of an SDCC-FLP defines it.
struct imapb_range {
	double min = 0.0;
	double max = 0.0;
};

/**
 * @brief Standard deviations and correlation coefficients of the items before the pack in its parent set
 *
 * The matrix covers the last `size` items before the pack, in their set order. Set decoding stores the tags of these
 * items in `members`. Set encoding writes the items that `members` names directly before the pack. Without `members`,
 * the pack covers the items that set encoding writes before it.
 */
struct sdcc_flp {
	uint32_t size = 0;  ///< Number of items that the matrix covers
	std::vector<uint32_t> members;  ///< Tags of the covered items in set order: `size` tags, or none
	std::vector<double> sigma;  ///< Standard deviations: `size` values, or none
	std::vector<uint64_t> sigma_raw;  ///< IMAPB standard deviations as raw integers when their ranges are not known
	std::vector<double> rho;  ///< Correlation coefficients of the upper triangle, row by row, or none
	uint8_t sigma_length = 4;  ///< Bytes of each standard deviation, or 0 when there are none
	uint8_t rho_length = 4;  ///< Bytes of each correlation coefficient, or 0 when there are none
	bool sparse = false;  ///< Omit the correlation coefficients that are zero
	bool long_parse_control = true;  ///< Mode 2: two parse control bytes, which select IEEE 754 or IMAPB for each part
	bool sigma_imapb = false;  ///< Standard deviations use IMAPB. This needs two parse control bytes.
	bool rho_imapb = true;  ///< Correlation coefficients use IMAPB in [-1, 1]. Always true with one control byte.

	/// Number of correlation coefficients: size * (size - 1) / 2
	[[nodiscard]]
	constexpr auto rho_count() const noexcept -> size_t {
		return size_t{size} * (size_t{size} - 1) / 2;
	}

	friend auto operator==(sdcc_flp const&, sdcc_flp const&) -> bool = default;
};

}  // namespace klv

namespace klv::codec {

/**
 * @brief MISB ST 1010 SDCC-FLP
 *
 * The pack holds, in this order:
 * - the matrix size (BER-OID)
 * - one or two parse control bytes
 * - a bit vector of the non-zero correlation coefficients, when the pack is sparse and has correlation coefficients
 * - the standard deviations
 * - the correlation coefficients
 *
 * Mode 2 parse control has two bytes, which select the form of each part. MISB ST 0601, ST 1107, and ST 1202 require
 * Mode 2, and `klv::sdcc_flp` uses it by default. Mode 1 has one byte. The codec reads Mode 1 standard deviations as
 * IEEE 754 and Mode 1 correlation coefficients as IMAPB. An IEEE 754 value has 4 or 8 bytes. An IMAPB value that
 * holds a special value decodes as NaN.
 *
 * The parent standard defines the IMAPB range of each standard deviation. ST 0601 and ST 1202 define none, so their
 * standard deviations use IEEE 754. `sigma_ranges` gives the range for the tag of a covered item. Set decoding gives
 * the codec the tags of the items before the pack. When each covered item has a range, IMAPB standard deviations
 * decode into `klv::sdcc_flp::sigma`. Otherwise they stay raw integers in `sigma_raw`.
 *
 * @code
 * static constexpr codec::repeated<codec::sdcc_flp> format{.item = {.sigma_ranges = &my_sigma_ranges}};
 * @endcode
 */
struct sdcc_flp {
	using value_type = klv::sdcc_flp;

	/// IMAPB range of the standard deviation of the item with `tag`, or no value
	using range_function = auto (*)(uint32_t tag) -> std::optional<imapb_range>;

	/// Largest matrix size that decoding accepts
	static constexpr auto max_size = uint32_t{65'535};

	/// IMAPB ranges of the standard deviations, or null when the parent standard does not define them
	range_function sigma_ranges = nullptr;

	/// Decode a pack without its parent set. `members` stays empty, and IMAPB standard deviations stay raw.
	[[nodiscard]]
	auto decode(byte_span in) const -> field<klv::sdcc_flp> {
		return parse(in, nullptr);
	}

	/**
	 * @brief Decode a pack in its parent set
	 *
	 * `preceding` holds the tags of the items before the pack, in decode order. A pack that covers more items fails
	 * with `field_error::invalid_value`.
	 */
	[[nodiscard]]
	auto decode(byte_span in, std::span<uint32_t const> preceding) const -> field<klv::sdcc_flp> {
		return parse(in, &preceding);
	}

	/// Tags of the items that set encoding writes directly before the pack
	[[nodiscard]]
	auto covered_tags(klv::sdcc_flp const& value) const noexcept -> std::span<uint32_t const> {
		return value.members;
	}

	[[nodiscard]]
	auto encode(klv::sdcc_flp const& value, byte_writer& out) const -> encode_result {
		auto const raw = !value.sigma_raw.empty();
		auto const sigma_count = raw ? value.sigma_raw.size() : value.sigma.size();
		auto const length_limit = value.long_parse_control ? 0x0FU : 0x07U;

		auto const invalid_size = value.size == 0 || value.size > max_size;
		auto const invalid_param_len = value.sigma_length > length_limit || value.rho_length > length_limit;
		auto const invalid_raw_sigma = raw && (!value.sigma_imapb || !value.sigma.empty());
		auto const invalid_members = !value.members.empty() && value.members.size() != value.size;
		auto const invalid_sigma_count = sigma_count != (value.sigma_length > 0 ? value.size : 0);
		auto const invalid_rho_count = value.rho.size() != (value.rho_length > 0 ? value.rho_count() : 0);
		auto const invalid_imapb = !value.long_parse_control && (value.sigma_imapb || !value.rho_imapb);

		if (invalid_size || invalid_param_len || invalid_raw_sigma || invalid_members || invalid_sigma_count
			|| invalid_rho_count || invalid_imapb) {
			return std::unexpected(field_error::invalid_value);
		}
		if (value.sigma_imapb && value.sigma_length > imapb::max_length) {
			return std::unexpected(field_error::bad_length);
		}

		// IMAPB standard deviations as real values need the range of each covered item
		auto const formats = (value.sigma_imapb && !value.sigma.empty()) ? sigma_formats(value) : std::nullopt;
		if (value.sigma_imapb && !value.sigma.empty() && !formats) {
			return std::unexpected(field_error::invalid_value);
		}

		out.put_ber_oid(value.size);
		if (value.long_parse_control) {
			auto const control = uint32_t{value.sigma_length} | (value.sigma_imapb ? 0x10U : 0U)
			    | (uint32_t{value.rho_length} << 7U) | (value.rho_imapb ? 0x800U : 0U) | (value.sparse ? 0x1000U : 0U);
			out.put_u8(static_cast<uint8_t>(0x80U | (control >> 7U)));
			out.put_u8(static_cast<uint8_t>(control & 0x7FU));
		}
		else {
			auto const control = (uint32_t{value.sigma_length} << 4U) | (value.sparse ? 0x08U : 0U) | value.rho_length;
			out.put_u8(static_cast<uint8_t>(control));
		}

		if (value.sparse && value.rho_length > 0) {
			auto bits = std::vector<uint8_t>((value.rho_count() + 7) / 8);
			for (auto i = size_t{0}; i < value.rho.size(); ++i) {
				if (!is_zero(value.rho[i])) {
					bits[i / 8] = static_cast<uint8_t>(bits[i / 8] | (0x80U >> (i % 8)));
				}
			}
			for (auto const byte : bits) {
				out.put_u8(byte);
			}
		}

		for (auto const sigma : value.sigma_raw) {
			if (value.sigma_length < sizeof(uint64_t) && (sigma >> (value.sigma_length * 8U)) != 0) {
				return std::unexpected(field_error::out_of_range);
			}
			out.put_big_endian(sigma, value.sigma_length);
		}
		for (auto i = size_t{0}; i < value.sigma.size(); ++i) {
			auto status = formats ? (*formats)[i].encode(value.sigma[i], out)
			                      : detail::put_ieee(value.sigma[i], value.sigma_length, out);
			if (!status) {
				return status;
			}
		}

		auto const format = rho_format(value);
		if (value.rho_length > 0 && value.rho_imapb && !format) {
			return std::unexpected(field_error::bad_length);
		}
		for (auto const rho : value.rho) {
			if (value.sparse && is_zero(rho)) {
				continue;
			}
			auto status = format ? format->encode(rho, out) : detail::put_ieee(rho, value.rho_length, out);
			if (!status) {
				return status;
			}
		}
		return {};
	}

	[[nodiscard]]
	auto to_string(klv::sdcc_flp const& value) const -> std::string {
		using detail::list_text;
		auto const members =
		    value.members.empty() ? std::string{} : std::format(", members: {}", list_text(value.members));
		auto const sigma = value.sigma_raw.empty() ? list_text(value.sigma) : list_text(value.sigma_raw);
		return std::format("{{size: {}{}, sigma: {}, rho: {}}}", value.size, members, sigma, list_text(value.rho));
	}

private:
	/// Decode a pack. `preceding` is null when the tags of the items before the pack are not known.
	[[nodiscard]]
	auto parse(byte_span in, std::span<uint32_t const> const* preceding) const -> field<klv::sdcc_flp> {
		auto reader = byte_reader{in};
		auto result = klv::sdcc_flp{};

		auto const size = detail::read_oid(reader);
		if (!size || *size == 0 || *size > max_size) {
			return std::unexpected(field_error::invalid_value);
		}
		result.size = *size;

		if (preceding != nullptr) {
			if (preceding->size() < result.size) {
				return std::unexpected(field_error::invalid_value);
			}
			auto const covered = preceding->last(result.size);
			result.members.assign(covered.begin(), covered.end());
		}

		auto const first = reader.read_u8();
		if (!first) {
			return std::unexpected(field_error::invalid_value);
		}
		if ((*first & 0x80U) == 0) {
			// Mode 1, one byte: 0 SSS P RRR (sigma length, sparse flag, rho length)
			result.long_parse_control = false;
			result.sigma_length = static_cast<uint8_t>((*first >> 4U) & 0x07U);
			result.sparse = (*first & 0x08U) != 0;
			result.rho_length = static_cast<uint8_t>(*first & 0x07U);
		}
		else {
			auto const second = reader.read_u8();
			if (!second || (*second & 0x80U) != 0) {
				return std::unexpected(field_error::invalid_value);
			}
			auto const control = static_cast<uint32_t>(((*first & 0x7FU) << 7U) | *second);
			result.long_parse_control = true;
			result.sigma_length = static_cast<uint8_t>(control & 0x0FU);
			result.sigma_imapb = (control & 0x10U) != 0;
			result.rho_length = static_cast<uint8_t>((control >> 7U) & 0x0FU);
			result.rho_imapb = (control & 0x800U) != 0;
			result.sparse = (control & 0x1000U) != 0;
		}

		// Without correlation coefficients, the pack has no bit vector (MISB ST 1010 Table 3)
		auto present = byte_span{};
		if (result.sparse && result.rho_length > 0) {
			auto const bits = detail::read_exact(reader, (result.rho_count() + 7) / 8);
			if (!bits) {
				return std::unexpected(field_error::invalid_value);
			}
			present = *bits;
		}

		if (result.sigma_length > 0) {
			if (result.sigma_imapb && result.sigma_length > imapb::max_length) {
				return std::unexpected(field_error::bad_length);
			}
			auto const formats = result.sigma_imapb ? sigma_formats(result) : std::nullopt;
			for (auto i = uint32_t{0}; i < result.size; ++i) {
				auto const bytes = detail::read_exact(reader, result.sigma_length);
				if (!bytes) {
					return std::unexpected(field_error::invalid_value);
				}
				if (formats) {
					auto const value = (*formats)[i].decode(*bytes);
					result.sigma.push_back(value.value_or(std::numeric_limits<double>::quiet_NaN()));
				}
				else if (result.sigma_imapb) {
					result.sigma_raw.push_back(read_big_endian<uint64_t>(*bytes));
				}
				else {
					auto const value = detail::read_ieee(*bytes);
					if (!value) {
						return std::unexpected(value.error());
					}
					result.sigma.push_back(*value);
				}
			}
		}

		if (result.rho_length > 0) {
			auto const format = rho_format(result);
			if (result.rho_imapb && !format) {
				return std::unexpected(field_error::bad_length);
			}
			for (auto i = size_t{0}; i < result.rho_count(); ++i) {
				if (result.sparse && (std::to_integer<uint8_t>(present[i / 8]) & (0x80U >> (i % 8))) == 0) {
					result.rho.push_back(0.0);
					continue;
				}
				auto const bytes = detail::read_exact(reader, result.rho_length);
				if (!bytes) {
					return std::unexpected(field_error::invalid_value);
				}
				if (format) {
					result.rho.push_back(format->decode(*bytes).value_or(std::numeric_limits<double>::quiet_NaN()));
				}
				else {
					auto const value = detail::read_ieee(*bytes);
					if (!value) {
						return std::unexpected(value.error());
					}
					result.rho.push_back(*value);
				}
			}
		}

		return detail::finish(reader, std::move(result));
	}

	/// IMAPB format of each standard deviation, or no value when the range of a covered item is not known
	[[nodiscard]]
	auto sigma_formats(klv::sdcc_flp const& value) const -> std::optional<std::vector<imapb>> {
		if (sigma_ranges == nullptr || value.members.size() != value.size || value.sigma_length < 1
		    || value.sigma_length > imapb::max_length) {
			return std::nullopt;
		}
		auto formats = std::vector<imapb>{};
		formats.reserve(value.size);
		for (auto const tag : value.members) {
			auto const range = sigma_ranges(tag);
			if (!range || !std::isfinite(range->min) || !std::isfinite(range->max) || !(range->min < range->max)
			    || !std::isfinite(range->max - range->min)) {
				return std::nullopt;
			}
			formats.emplace_back(range->min, range->max, value.sigma_length);
		}
		return formats;
	}

	[[nodiscard]]
	static auto is_zero(double value) noexcept -> bool {
		return std::fpclassify(value) == FP_ZERO;
	}

	/// IMAPB format of the correlation coefficients, or no value when they use IEEE 754 or the length is not valid
	[[nodiscard]]
	static auto rho_format(klv::sdcc_flp const& value) -> std::optional<imapb> {
		if (!value.rho_imapb || value.rho_length < 1 || value.rho_length > imapb::max_length) {
			return std::nullopt;
		}
		return imapb{-1.0, 1.0, value.rho_length};
	}
};

}  // namespace klv::codec
