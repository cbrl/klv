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
#include "klv/field.hpp"

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <format>
#include <limits>
#include <optional>
#include <string>
#include <vector>

namespace klv {

/**
 * @brief Standard deviations and correlation coefficients of the items before the pack in its parent set
 *
 * The matrix covers the last `size` items before the pack, in their set order. The parent standard defines which
 * items those are.
 */
struct sdcc_flp {
	uint32_t size = 0;  ///< Number of items that the matrix covers
	std::vector<double> sigma;  ///< Standard deviations in IEEE 754 form: `size` values, or none
	std::vector<uint64_t> sigma_imapb;  ///< Standard deviations in IMAPB form, as raw integers: `size` values, or none
	std::vector<double> rho;  ///< Correlation coefficients of the upper triangle, row by row, or none
	uint8_t sigma_length = 4;  ///< Bytes of each standard deviation, or 0 when there are none
	uint8_t rho_length = 4;  ///< Bytes of each correlation coefficient, or 0 when there are none
	bool sparse = false;  ///< Omit the correlation coefficients that are zero
	bool long_parse_control = false;  ///< Two parse control bytes, which select IEEE 754 or IMAPB for each part
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
 * - a bit vector of the non-zero correlation coefficients, when the pack is sparse
 * - the standard deviations
 * - the correlation coefficients
 *
 * With one parse control byte, the standard deviations use IEEE 754 and the correlation coefficients use IMAPB. Two
 * bytes select the form of each part.
 *
 * IMAPB standard deviations stay raw integers in `sdcc_flp::sigma_imapb`, because the parent standard defines their
 * ranges. An IEEE 754 value has 4 or 8 bytes. An IMAPB correlation coefficient that holds a special value decodes as
 * NaN.
 */
struct sdcc_flp {
	using value_type = klv::sdcc_flp;

	/// Largest matrix size that decoding accepts
	static constexpr auto max_size = uint32_t{65'535};

	[[nodiscard]]
	auto decode(byte_span in) const -> field<klv::sdcc_flp> {
		auto reader = byte_reader{in};
		auto result = klv::sdcc_flp{};

		auto const size = detail::read_oid(reader);
		if (!size || *size == 0 || *size > max_size) {
			return std::unexpected(field_error::invalid_value);
		}
		result.size = *size;

		auto const first = reader.read_u8();
		if (!first) {
			return std::unexpected(field_error::invalid_value);
		}
		auto sigma_imapb = false;
		if ((*first & 0x80U) == 0) {
			// One byte: 0 SSS P RRR (sigma length, sparse flag, rho length)
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
			sigma_imapb = (control & 0x10U) != 0;
			result.rho_length = static_cast<uint8_t>((control >> 7U) & 0x0FU);
			result.rho_imapb = (control & 0x800U) != 0;
			result.sparse = (control & 0x1000U) != 0;
		}

		auto present = byte_span{};
		if (result.sparse) {
			auto const bits = detail::read_exact(reader, (result.rho_count() + 7) / 8);
			if (!bits) {
				return std::unexpected(field_error::invalid_value);
			}
			present = *bits;
		}

		if (result.sigma_length > 0) {
			for (auto i = uint32_t{0}; i < result.size; ++i) {
				auto const bytes = detail::read_exact(reader, result.sigma_length);
				if (!bytes) {
					return std::unexpected(field_error::invalid_value);
				}
				if (sigma_imapb) {
					if (bytes->size() > sizeof(uint64_t)) {
						return std::unexpected(field_error::bad_length);
					}
					result.sigma_imapb.push_back(read_big_endian<uint64_t>(*bytes));
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

	[[nodiscard]]
	auto encode(klv::sdcc_flp const& value, byte_writer& out) const -> encode_result {
		auto const sigma_imapb = !value.sigma_imapb.empty();
		auto const sigma_count = sigma_imapb ? value.sigma_imapb.size() : value.sigma.size();
		auto const length_limit = value.long_parse_control ? 0x0FU : 0x07U;

		if (value.size == 0 || value.size > max_size || value.sigma_length > length_limit
		    || value.rho_length > length_limit || (sigma_imapb && !value.sigma.empty())
		    || sigma_count != (value.sigma_length > 0 ? value.size : 0)
		    || value.rho.size() != (value.rho_length > 0 ? value.rho_count() : 0)) {
			return std::unexpected(field_error::invalid_value);
		}
		if (!value.long_parse_control && (sigma_imapb || !value.rho_imapb)) {
			return std::unexpected(field_error::invalid_value);
		}
		if (sigma_imapb && value.sigma_length > sizeof(uint64_t)) {
			return std::unexpected(field_error::bad_length);
		}

		out.put_ber_oid(value.size);
		if (value.long_parse_control) {
			auto const control = uint32_t{value.sigma_length} | (sigma_imapb ? 0x10U : 0U)
			    | (uint32_t{value.rho_length} << 7U) | (value.rho_imapb ? 0x800U : 0U) | (value.sparse ? 0x1000U : 0U);
			out.put_u8(static_cast<uint8_t>(0x80U | (control >> 7U)));
			out.put_u8(static_cast<uint8_t>(control & 0x7FU));
		}
		else {
			auto const control = (uint32_t{value.sigma_length} << 4U) | (value.sparse ? 0x08U : 0U) | value.rho_length;
			out.put_u8(static_cast<uint8_t>(control));
		}

		if (value.sparse) {
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

		for (auto const sigma : value.sigma_imapb) {
			if (value.sigma_length < sizeof(uint64_t) && (sigma >> (value.sigma_length * 8U)) != 0) {
				return std::unexpected(field_error::out_of_range);
			}
			out.put_big_endian(sigma, value.sigma_length);
		}
		for (auto const sigma : value.sigma) {
			if (auto status = detail::put_ieee(sigma, value.sigma_length, out); !status) {
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
		auto const list = [](auto const& values) {
			auto text = std::string{"["};
			for (auto i = size_t{0}; i < values.size(); ++i) {
				text += std::format("{}{}", i == 0 ? "" : ", ", values[i]);
			}
			return text + "]";
		};
		auto const sigma = value.sigma_imapb.empty() ? list(value.sigma) : list(value.sigma_imapb);
		return std::format("{{size: {}, sigma: {}, rho: {}}}", value.size, sigma, list(value.rho));
	}

private:
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
