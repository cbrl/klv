/**
 * @file misb1504.hpp
 * @brief MISB ST 1504 - Natural Representation of Orbital State Vectors
 *
 * The State Vector Local Set gives the orbit of a Motion Imagery collector in Earth orbit. Six orbital elements give
 * the orbit: Keplerian elements or Non-Singular Equinoctial elements. The State Vector Status of the State Vector Pack
 * gives the kind of elements and the reference frame.
 *
 * Each pack holds real numbers of one length: 4 or 8 bytes. Encoding writes 4 bytes when binary32 holds each value
 * exactly, and 8 bytes otherwise. A stand-alone set ends with a CRC-16-CCITT (tag 3). A set inside another set has no
 * CRC.
 *
 * ST 1504 section 7.11 keeps value 0 for research in a closed system. This applies to the Reference Frame, the State
 * Vector Type, and the Anomaly Type. The codecs accept value 0, but a set with this value does not conform to ST 1504.
 *
 * The library does not check these requirements:
 * - ST 1504-08: Keplerian elements need the Anomaly Type (tag 5).
 * - ST 1504-10: the Mean Anomaly at Epoch needs the Mean Anomaly Epoch (tag 6).
 * - ST 1504-14: the SDCC-FLP must use Mode 2 parse control.
 */

#pragma once

#include "klv/bytes.hpp"
#include "klv/codec.hpp"
#include "klv/codecs/pack.hpp"
#include "klv/set.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <format>
#include <span>
#include <string>
#include <string_view>

namespace klv::misb1504 {

/**
 * @brief Reference frame of a State Vector Pack (State Vector Status bits 6 to 4, ST 1504 Table 3)
 *
 * In the WGS 84 frame, the longitude of the ascending node starts at the Greenwich Meridian. In the two inertial
 * frames, it starts at the First Point of Aries.
 */
enum class reference_frame : uint8_t {
	experimental = 0,  ///< Reserved for research in a closed system
	wgs84_ecef = 1,  ///< WGS 84 Earth-centered, Earth-fixed
	j2000_eci = 2,  ///< J2000 (FK5) Earth-centered inertial
	true_of_date_eci = 3,  ///< True of Date Earth-centered inertial
};

[[nodiscard]]
constexpr auto to_string(reference_frame value) noexcept -> std::string_view {
	using enum reference_frame;
	switch (value) {
		case experimental:
			return "Experimental";
		case wgs84_ecef:
			return "WGS 84 ECEF";
		case j2000_eci:
			return "J2000 ECI";
		case true_of_date_eci:
			return "True of Date ECI";
	}
	return {};
}

/// Kind of the State Vector Elements (State Vector Status bits 3 to 0, Table 4)
enum class state_vector_kind : uint8_t {
	experimental = 0,  ///< Reserved for research in a closed system
	keplerian = 1,  ///< Keplerian orbital elements
	equinoctial = 2,  ///< Non-Singular Equinoctial elements
};

[[nodiscard]]
constexpr auto to_string(state_vector_kind value) noexcept -> std::string_view {
	using enum state_vector_kind;
	switch (value) {
		case experimental:
			return "Experimental";
		case keplerian:
			return "Keplerian Orbital Elements";
		case equinoctial:
			return "Non-Singular Equinoctial Elements";
	}
	return {};
}

/// Anomaly in State Vector Element 6 of Keplerian elements (tag 5, Table 6)
enum class anomaly_kind : uint8_t {
	experimental = 0,  ///< Reserved for research in a closed system
	eccentric_anomaly = 1,
	mean_anomaly = 2,
	mean_anomaly_at_epoch = 3,  ///< The Mean Anomaly Epoch (tag 6) gives the time of the anomaly
	true_anomaly = 4,
};

[[nodiscard]]
constexpr auto to_string(anomaly_kind value) noexcept -> std::string_view {
	using enum anomaly_kind;
	switch (value) {
		case experimental:
			return "Experimental";
		case eccentric_anomaly:
			return "Eccentric Anomaly";
		case mean_anomaly:
			return "Mean Anomaly";
		case mean_anomaly_at_epoch:
			return "Mean Anomaly at Epoch";
		case true_anomaly:
			return "True Anomaly";
	}
	return {};
}

/**
 * @brief Value of a State Vector Pack: the State Vector Status and the six State Vector Elements
 *
 * `elements[0]` is State Vector Element 1. The kind of the elements gives their meaning (ST 1504 Table 5):
 *
 * | Element | Keplerian                                   | Equinoctial           |
 * |---------|---------------------------------------------|-----------------------|
 * | 1       | Semi-major axis a (m)                       | Semi-major axis a (m) |
 * | 2       | Eccentricity e                              | h                     |
 * | 3       | Inclination i (deg)                         | k                     |
 * | 4       | Argument of perigee (deg)                   | Mean longitude (deg)  |
 * | 5       | Longitude of the ascending node (deg)       | p                     |
 * | 6       | Anomaly that the Anomaly Type selects (deg) | q                     |
 */
struct state_vector {
	reference_frame frame{};
	state_vector_kind kind{};
	std::array<double, 6> elements{};

	friend auto operator==(state_vector const&, state_vector const&) -> bool = default;
};

namespace detail {

/// Read `N` real numbers of one length. The length of `in` must be `N` times 4 or `N` times 8 bytes.
template<size_t N>
[[nodiscard]]
auto read_reals(byte_span in) -> field<std::array<double, N>> {
	if (in.size() != N * sizeof(float) && in.size() != N * sizeof(double)) {
		return std::unexpected(field_error::bad_length);
	}
	auto const length = in.size() / N;
	auto values = std::array<double, N>{};
	for (auto i = size_t{0}; i < N; ++i) {
		auto const value = klv::detail::read_ieee(in.subspan(i * length, length));
		if (!value) {
			return std::unexpected(value.error());
		}
		values[i] = *value;
	}
	return values;
}

/// Write each value with `length` bytes. With `variable_length`, the length is 4 when binary32 holds each value.
template<size_t N>
[[nodiscard]]
auto put_reals(std::array<double, N> const& values, size_t length, byte_writer& out) -> encode_result {
	if (length == codec::variable_length) {
		length = sizeof(float);
		for (auto const value : values) {
			if (!klv::detail::fits_binary32(value)) {
				length = sizeof(double);
			}
		}
	}
	for (auto const value : values) {
		if (auto status = klv::detail::put_ieee(value, length, out); !status) {
			return status;
		}
	}
	return {};
}

template<size_t N>
[[nodiscard]]
auto reals_text(std::array<double, N> const& values) -> std::string {
	auto text = std::string{"["};
	for (auto i = size_t{0}; i < N; ++i) {
		text += std::format("{}{}", i == 0 ? "" : ", ", values[i]);
	}
	return text + "]";
}

}  // namespace detail

/**
 * @brief Pack of `N` real numbers of one length: 4 or 8 bytes each
 *
 * Decoding gets the length of each value from the length of the pack. Another pack length fails with
 * `field_error::bad_length`. Encoding writes `length` bytes for each value. With `variable_length`, it writes 4 bytes
 * when binary32 holds each value exactly, and 8 bytes otherwise.
 */
template<size_t N>
struct real_pack_codec {
	using value_type = std::array<double, N>;

	size_t length = codec::variable_length;

	[[nodiscard]]
	auto decode(byte_span in) const -> field<value_type> {
		return detail::read_reals<N>(in);
	}

	[[nodiscard]]
	auto encode(value_type const& value, byte_writer& out) const -> encode_result {
		return detail::put_reals(value, length, out);
	}

	[[nodiscard]]
	auto to_string(value_type const& value) const -> std::string {
		return detail::reals_text(value);
	}
};

/**
 * @brief State Vector Pack: the State Vector Status (BER-OID), then six real numbers of one length
 *
 * Bits 6 to 4 of the first byte of the State Vector Status hold the Reference Frame. Bits 3 to 0 hold the State Vector
 * Type. A later version of ST 1504 can add more bytes, and decoding ignores them. Encoding writes one byte. A reserved
 * value fails with `field_error::invalid_value`.
 */
struct state_vector_codec {
	using value_type = state_vector;

	size_t length = codec::variable_length;  ///< Bytes of each element: 4, 8, or `variable_length`

	[[nodiscard]]
	auto decode(byte_span in) const -> field<state_vector> {
		if (in.empty()) {
			return std::unexpected(field_error::bad_length);
		}

		// Bit 7 of each byte of the BER-OID value, except the last byte, is 1
		auto status_length = size_t{1};
		while ((std::to_integer<uint8_t>(in[status_length - 1]) & 0x80U) != 0) {
			if (status_length == in.size()) {
				return std::unexpected(field_error::invalid_value);
			}
			++status_length;
		}

		auto const bits = std::to_integer<uint8_t>(in.front());
		auto value = state_vector{
		    .frame = static_cast<reference_frame>((bits >> 4U) & 0x07U),
		    .kind = static_cast<state_vector_kind>(bits & 0x0FU),
		    .elements = {},
		};
		if (!is_defined(value)) {
			return std::unexpected(field_error::invalid_value);
		}

		auto const elements = detail::read_reals<6>(in.subspan(status_length));
		if (!elements) {
			return std::unexpected(elements.error());
		}
		value.elements = *elements;
		return value;
	}

	[[nodiscard]]
	auto encode(state_vector const& value, byte_writer& out) const -> encode_result {
		if (!is_defined(value)) {
			return std::unexpected(field_error::invalid_value);
		}
		auto const frame = static_cast<unsigned>(value.frame) << 4U;
		out.put_u8(static_cast<uint8_t>(frame | static_cast<unsigned>(value.kind)));
		return detail::put_reals(value.elements, length, out);
	}

	[[nodiscard]]
	auto to_string(state_vector const& value) const -> std::string {
		return std::format(
		    "{{frame: {}, kind: {}, elements: {}}}",
		    misb1504::to_string(value.frame),
		    misb1504::to_string(value.kind),
		    detail::reals_text(value.elements)
		);
	}

private:
	[[nodiscard]]
	static constexpr auto is_defined(state_vector const& value) noexcept -> bool {
		return !misb1504::to_string(value.frame).empty() && !misb1504::to_string(value.kind).empty();
	}
};

struct precision_time_stamp {
	static constexpr uint32_t tag = 1;
	static constexpr std::string_view name = "Precision Time Stamp";
	static constexpr codec::timestamp format{};
};

/// Minor version of ST 1504, for example 1 for ST 1504.1
struct document_version {
	static constexpr uint32_t tag = 2;
	static constexpr std::string_view name = "Document Version";
	static constexpr codec::integer<uint32_t> format{.length = codec::variable_length};
};

/// CRC-16-CCITT of the packet, from the key to the length of this item. Packet encoding writes it as the last item.
struct crc {
	static constexpr uint32_t tag = 3;
	static constexpr std::string_view name = "CRC";
	static constexpr codec::integer<uint16_t> format{};
};

struct state_vector_pack {
	static constexpr uint32_t tag = 4;
	static constexpr std::string_view name = "State Vector Pack";
	static constexpr state_vector_codec format{};
};

/// Anomaly in State Vector Element 6 of Keplerian elements
struct anomaly_type {
	static constexpr uint32_t tag = 5;
	static constexpr std::string_view name = "Anomaly Type";
	static constexpr codec::enumeration<anomaly_kind> format{};
};

/// Time at which the Mean Anomaly at Epoch is valid
struct mean_anomaly_epoch {
	static constexpr uint32_t tag = 6;
	static constexpr std::string_view name = "Mean Anomaly Epoch";
	static constexpr codec::timestamp format{.min_length = 1};
};

/// Rate of change of State Vector Elements 1 to 6. The units are the units of each element per second.
struct state_vector_troc_pack {
	static constexpr uint32_t tag = 7;
	static constexpr std::string_view name = "State Vector TROC Pack";
	static constexpr real_pack_codec<6> format{};
};

/// X, Y, and Z of the eccentricity vector. The vector points to the periapsis. Its magnitude is the eccentricity.
struct eccentricity_vector_pack {
	static constexpr uint32_t tag = 8;
	static constexpr std::string_view name = "Eccentricity Vector Pack";
	static constexpr real_pack_codec<3> format{};
};

/// X, Y, and Z of the unit vector that points to the ascending node
struct node_vector_pack {
	static constexpr uint32_t tag = 9;
	static constexpr std::string_view name = "Node Vector Pack";
	static constexpr real_pack_codec<3> format{};
};

/// X, Y, and Z of the specific angular momentum vector: the cross product of the position and the velocity
struct specific_angular_momentum_pack {
	static constexpr uint32_t tag = 10;
	static constexpr std::string_view name = "Specific Angular Momentum Pack";
	static constexpr std::string_view units = "m^2/s";
	static constexpr real_pack_codec<3> format{};
};

struct mean_motion {
	static constexpr uint32_t tag = 11;
	static constexpr std::string_view name = "Mean Motion";
	static constexpr std::string_view units = "deg/s";
	static constexpr codec::ieee754_variable format{};
};

/// Standard gravitational parameter: G (M Earth + M Satellite)
struct mu {
	static constexpr uint32_t tag = 12;
	static constexpr std::string_view name = "Mu";
	static constexpr std::string_view units = "m^3/s^2";
	static constexpr codec::ieee754_variable format{};
};

struct semilatus_rectum {
	static constexpr uint32_t tag = 13;
	static constexpr std::string_view name = "Semilatus Rectum";
	static constexpr std::string_view units = "m";
	static constexpr codec::ieee754_variable format{};
};

/// Energy for each unit of mass
struct specific_energy {
	static constexpr uint32_t tag = 14;
	static constexpr std::string_view name = "Specific Energy";
	static constexpr std::string_view units = "J/kg";
	static constexpr codec::ieee754_variable format{};
};

/**
 * @brief SDCC-FLP of the elements of the State Vector Pack, the State Vector TROC Pack, or both (tag 15)
 *
 * The matrix has 6 rows for each pack that it covers: one row for each element, in element order. The covered packs
 * come directly before the SDCC-FLP. `klv::sdcc_flp::members` holds the tag of the pack of each row.
 *
 * Set decoding fills `members` from the items before the SDCC-FLP. Set encoding writes the packs that `members` names
 * directly before the SDCC-FLP. Without `members`, 6 rows cover the State Vector Pack. Twelve rows cover the State
 * Vector Pack, then the State Vector TROC Pack.
 *
 * A matrix size other than 6 or 12 fails with `field_error::invalid_value`. Covered items that are not these packs
 * also fail with `field_error::invalid_value`.
 */
struct sdcc_codec {
	using value_type = klv::sdcc_flp;

	/// Decode a pack without its parent set. `members` stays empty.
	[[nodiscard]]
	auto decode(byte_span in) const -> field<klv::sdcc_flp> {
		auto value = generic.decode(in);
		if (value && !is_valid_size(value->size)) {
			return std::unexpected(field_error::invalid_value);
		}
		return value;
	}

	/// Decode a pack in its parent set. `preceding` holds the tags of the items before the pack, in decode order.
	[[nodiscard]]
	auto decode(byte_span in, std::span<uint32_t const> preceding) const -> field<klv::sdcc_flp> {
		auto value = decode(in);
		if (!value) {
			return value;
		}
		auto const count = size_t{value->size / rows_per_pack};
		if (preceding.size() < count) {
			return std::unexpected(field_error::invalid_value);
		}
		auto const packs = preceding.last(count);
		if (!is_valid_packs(packs)) {
			return std::unexpected(field_error::invalid_value);
		}
		for (auto const tag : packs) {
			value->members.insert(value->members.end(), rows_per_pack, tag);
		}
		return value;
	}

	/// Tags of the packs that set encoding writes directly before the SDCC-FLP
	[[nodiscard]]
	auto covered_tags(klv::sdcc_flp const& value) const noexcept -> std::span<uint32_t const> {
		if (!is_valid_size(value.size)) {
			return {};
		}
		if (value.members.empty()) {
			return value.size == rows_per_pack ? std::span<uint32_t const>{state_only} : both;
		}
		if (!is_valid_members(value)) {
			return {};
		}
		auto const state_first = value.members.front() == state_vector_pack::tag;
		if (value.size == rows_per_pack) {
			return state_first ? std::span<uint32_t const>{state_only} : rate_only;
		}
		return state_first ? std::span<uint32_t const>{both} : reversed;
	}

	[[nodiscard]]
	auto encode(klv::sdcc_flp const& value, byte_writer& out) const -> encode_result {
		if (!is_valid_size(value.size) || (!value.members.empty() && !is_valid_members(value))) {
			return std::unexpected(field_error::invalid_value);
		}
		return generic.encode(value, out);
	}

	[[nodiscard]]
	auto to_string(klv::sdcc_flp const& value) const -> std::string {
		return generic.to_string(value);
	}

private:
	static constexpr codec::sdcc_flp generic{};
	static constexpr auto rows_per_pack = uint32_t{6};

	static constexpr auto state_only = std::array{state_vector_pack::tag};
	static constexpr auto rate_only = std::array{state_vector_troc_pack::tag};
	static constexpr auto both = std::array{state_vector_pack::tag, state_vector_troc_pack::tag};
	static constexpr auto reversed = std::array{state_vector_troc_pack::tag, state_vector_pack::tag};

	[[nodiscard]]
	static constexpr auto is_valid_size(uint32_t size) noexcept -> bool {
		return size == rows_per_pack || size == 2 * rows_per_pack;
	}

	/// The tags name the State Vector Pack, the State Vector TROC Pack, or both, each one time
	[[nodiscard]]
	static constexpr auto is_valid_packs(std::span<uint32_t const> tags) noexcept -> bool {
		for (auto i = size_t{0}; i < tags.size(); ++i) {
			if (tags[i] != state_vector_pack::tag && tags[i] != state_vector_troc_pack::tag) {
				return false;
			}
			if (i > 0 && tags[i] == tags[0]) {
				return false;
			}
		}
		return true;
	}

	/// `members` names one pack for each group of 6 rows
	[[nodiscard]]
	static auto is_valid_members(klv::sdcc_flp const& value) noexcept -> bool {
		if (value.members.size() != value.size) {
			return false;
		}
		auto packs = std::array<uint32_t, 2>{};
		auto const count = size_t{value.size / rows_per_pack};
		for (auto i = size_t{0}; i < value.members.size(); ++i) {
			if (value.members[i] != value.members[i - i % rows_per_pack]) {
				return false;
			}
			packs[i / rows_per_pack] = value.members[i];
		}
		return is_valid_packs(std::span{packs}.first(count));
	}
};

struct sdcc_flp {
	static constexpr uint32_t tag = 15;
	static constexpr std::string_view name = "Standard Deviation and Correlation Coefficient FLP";
	static constexpr sdcc_codec format{};
};

/**
 * @brief MISB ST 1504 State Vector Local Set
 *
 * The set needs the Precision Time Stamp, the Document Version, and the State Vector Pack. Packet decoding verifies
 * the CRC-16-CCITT (tag 3), and packet encoding computes it.
 */
struct state_vector_ls {
	static constexpr std::string_view name = "State Vector Local Set";
	static constexpr universal_key key = ul("06 0E 2B 34 02 0B 01 01 0E 01 03 01 03 00 00 00");

	using checksum_field = crc;
	static constexpr auto checksum_function = crc16_ccitt;

	using fields = field_list<
	    precision_time_stamp,
	    document_version,
	    crc,
	    state_vector_pack,
	    anomaly_type,
	    mean_anomaly_epoch,
	    state_vector_troc_pack,
	    eccentricity_vector_pack,
	    node_vector_pack,
	    specific_angular_momentum_pack,
	    mean_motion,
	    mu,
	    semilatus_rectum,
	    specific_energy,
	    sdcc_flp>;
};

}  // namespace klv::misb1504
