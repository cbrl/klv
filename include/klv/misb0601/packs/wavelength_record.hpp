/**
 * @file klv/misb0601/packs/wavelength_record.hpp
 * @brief MISB ST 0601 Wavelengths List (tag 128)
 */

#pragma once

#include "klv/bytes.hpp"
#include "klv/codec.hpp"
#include "klv/codecs/pack.hpp"

#include <array>
#include <cstdint>
#include <expected>
#include <format>
#include <string>
#include <tuple>

namespace klv::misb0601 {

/// One wavelength band of the sensors
struct wavelength_record {
	uint32_t id = 0;
	double minimum = 0.0;  ///< nm
	double maximum = 0.0;  ///< nm
	std::string name;

	// Exact comparison. std::tie compares in the standard library, which keeps -Wfloat-equal quiet.
	friend auto operator==(wavelength_record const& lhs, wavelength_record const& rhs) -> bool {
		return std::tie(lhs.id, lhs.minimum, lhs.maximum, lhs.name)
		    == std::tie(rhs.id, rhs.minimum, rhs.maximum, rhs.name);
	}
};

/// Wavelength record: BER-OID ID, minimum and maximum (IMAPB, 4 bytes each), then the name in the remaining bytes
struct wavelength_record_codec {
	using value_type = wavelength_record;

	static constexpr codec::imapb wavelength_format{0.0, 1.0e9, 4};

	[[nodiscard]]
	auto decode(byte_span in) const -> field<wavelength_record> {
		auto reader = byte_reader{in};
		auto const id = detail::read_oid(reader);
		if (!id) {
			return std::unexpected(id.error());
		}

		auto bounds = std::array<double, 2>{};
		for (auto& bound : bounds) {
			auto const bytes = detail::read_exact(reader, wavelength_format.length());
			if (!bytes) {
				return std::unexpected(bytes.error());
			}
			auto const value = wavelength_format.decode(*bytes);
			if (!value) {
				return std::unexpected(value.error());
			}
			bound = *value;
		}

		return wavelength_record{
		    .id = *id,
		    .minimum = bounds[0],
		    .maximum = bounds[1],
		    .name = detail::to_text(reader.rest()),
		};
	}

	[[nodiscard]]
	auto encode(wavelength_record const& value, byte_writer& out) const -> encode_result {
		out.put_ber_oid(value.id);
		return wavelength_format.encode(value.minimum, out)
		    .and_then([&] { return wavelength_format.encode(value.maximum, out); })
		    .and_then([&] {
			    detail::put_text(value.name, out);
			    return encode_result{};
		    });
	}

	[[nodiscard]]
	auto to_string(wavelength_record const& value) const -> std::string {
		return std::format(
		    "{{id: {}, minimum: {:.3f}, maximum: {:.3f}, name: \"{}\"}}",
		    value.id,
		    value.minimum,
		    value.maximum,
		    value.name
		);
	}
};

}  // namespace klv::misb0601
