/**
 * @file klv/misb1504/enumerations.hpp
 * @brief MISB ST 1504 enumerations
 */

#pragma once

#include <cstdint>
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

}  // namespace klv::misb1504
