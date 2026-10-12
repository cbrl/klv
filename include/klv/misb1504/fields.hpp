/**
 * @file klv/misb1504/fields.hpp
 * @brief MISB ST 1504 State Vector Local Set field descriptors, except the SDCC-FLP
 */

#pragma once

#include "klv/codec.hpp"
#include "klv/misb1504/enumerations.hpp"
#include "klv/misb1504/state_vector.hpp"

#include <cstdint>
#include <string_view>

namespace klv::misb1504 {

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

}  // namespace klv::misb1504
