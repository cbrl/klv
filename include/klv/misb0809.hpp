/**
 * @file misb0809.hpp
 * @brief MISB ST 0809 - Meteorological Metadata Local Set
 *
 * The set describes the atmosphere between a sensor and the objects that it records. The items include temperatures,
 * clouds, fog, illumination, and turbulence. The set arrives as a stand-alone packet. It has no checksum.
 *
 * ST 0809 requires the Precision Time Stamp (tag 1) as the first item. The Version (tag 2) must be the second item.
 * Encoding writes the fields in tag order, so these two items come first. Decoding accepts the items in any order.
 * The codecs do not check the ranges of ST 0809 Table 1, for example 0 to 100 for a percentage.
 */

#pragma once

#include "klv/bytes.hpp"
#include "klv/codec.hpp"
#include "klv/set.hpp"

#include <cstdint>
#include <string_view>

namespace klv::misb0809 {

/// Fog Presence value (tag 29)
enum class fog : uint8_t {
	absent = 0,
	present = 1,
};

[[nodiscard]]
constexpr auto to_string(fog value) noexcept -> std::string_view {
	switch (value) {
		case fog::absent:
			return "Absent";
		case fog::present:
			return "Present";
	}
	return {};
}

/// Time at which the measurements are valid
struct precision_time_stamp {
	static constexpr uint32_t tag = 1;
	static constexpr std::string_view name = "Precision Time Stamp";
	static constexpr codec::timestamp format{};
};

/// Version of ST 0809 that the set uses, for example 2 for ST 0809.2
struct version {
	static constexpr uint32_t tag = 2;
	static constexpr std::string_view name = "Version";
	static constexpr codec::integer<uint16_t> format{};
};

/// Name or location of the entity that reports the data
struct reporting_id {
	static constexpr uint32_t tag = 3;
	static constexpr std::string_view name = "Reporting ID";
	static constexpr codec::string format{.max_length = 40};
};

/// Latitude of the sensor or station that reports the data. North is positive.
struct device_latitude {
	static constexpr uint32_t tag = 4;
	static constexpr std::string_view name = "Device Latitude";
	static constexpr std::string_view units = "deg";
	static constexpr codec::ieee754<double> format{};
};

/// Longitude of the sensor or station that reports the data. East is positive.
struct device_longitude {
	static constexpr uint32_t tag = 5;
	static constexpr std::string_view name = "Device Longitude";
	static constexpr std::string_view units = "deg";
	static constexpr codec::ieee754<double> format{};
};

/// Atmospheric pressure from a barometer with a column of liquid
struct barometric_pressure {
	static constexpr uint32_t tag = 6;
	static constexpr std::string_view name = "Barometric Pressure";
	static constexpr std::string_view units = "Pa";
	static constexpr codec::ieee754<float> format{};
};

/// Temperature of a dry bulb inside a black copper sphere. It shows the effects of solar radiation and wind.
struct black_globe_temperature {
	static constexpr uint32_t tag = 7;
	static constexpr std::string_view name = "Black Globe Temperature";
	static constexpr std::string_view units = "degC";
	static constexpr codec::ieee754<float> format{};
};

/// Effect of temperature, humidity, and solar radiation on people. It is 0.7 x Wet Bulb Temperature + 0.1 x Dry Bulb
/// Temperature + 0.2 x Black Globe Temperature.
struct wet_bulb_temperature_heat_index {
	static constexpr uint32_t tag = 8;
	static constexpr std::string_view name = "Wet Bulb Temperature Heat Index";
	static constexpr std::string_view units = "degC";
	static constexpr codec::ieee754<float> format{};
};

/// Part of the sky that clouds hide: clear (0), scattered (10 to 50), broken (60 to 90), or overcast (100)
struct cloud_cover_percentage {
	static constexpr uint32_t tag = 9;
	static constexpr std::string_view name = "Cloud Cover Percentage";
	static constexpr std::string_view units = "%";
	static constexpr codec::ieee754<float> format{};
};

/// Description of the clouds, for example "cirrus"
struct cloud_sky_cover_type {
	static constexpr uint32_t tag = 10;
	static constexpr std::string_view name = "Cloud Sky Cover Type";
	static constexpr codec::string format{.max_length = 40};
};

/// Height of the cloud base above Mean Sea Level
struct cloud_base_height {
	static constexpr uint32_t tag = 11;
	static constexpr std::string_view name = "Cloud Base Height";
	static constexpr std::string_view units = "m";
	static constexpr codec::ieee754<float> format{};
};

/// Height of the cloud top above Mean Sea Level
struct cloud_top_height {
	static constexpr uint32_t tag = 12;
	static constexpr std::string_view name = "Cloud Top Height";
	static constexpr std::string_view units = "m";
	static constexpr codec::ieee754<float> format{};
};

/// Cloud Top Height minus Cloud Base Height
struct cloud_thickness {
	static constexpr uint32_t tag = 13;
	static constexpr std::string_view name = "Cloud Thickness";
	static constexpr std::string_view units = "m";
	static constexpr codec::ieee754<float> format{};
};

struct carbon_dioxide_concentration {
	static constexpr uint32_t tag = 14;
	static constexpr std::string_view name = "Carbon Dioxide Concentration";
	static constexpr std::string_view units = "ppm";
	static constexpr codec::ieee754<float> format{};
};

struct ozone_concentration {
	static constexpr uint32_t tag = 15;
	static constexpr std::string_view name = "Ozone Concentration";
	static constexpr std::string_view units = "ppm";
	static constexpr codec::ieee754<float> format{};
};

/// Temperature of the atmosphere
struct temperature {
	static constexpr uint32_t tag = 16;
	static constexpr std::string_view name = "Temperature";
	static constexpr std::string_view units = "degC";
	static constexpr codec::ieee754<float> format{};
};

/// Temperature of the air from the dry-bulb thermometer of a psychrometer
struct dry_bulb_temperature {
	static constexpr uint32_t tag = 17;
	static constexpr std::string_view name = "Dry Bulb Temperature";
	static constexpr std::string_view units = "degC";
	static constexpr codec::ieee754<float> format{};
};

/// Temperature of the air after the evaporation of water cools it to saturation at constant pressure
struct wet_bulb_temperature {
	static constexpr uint32_t tag = 18;
	static constexpr std::string_view name = "Wet Bulb Temperature";
	static constexpr std::string_view units = "degC";
	static constexpr codec::ieee754<float> format{};
};

/// Partial pressure of the water vapor, divided by the partial pressure at saturation, at the same temperature
struct relative_humidity {
	static constexpr uint32_t tag = 19;
	static constexpr std::string_view name = "Relative Humidity";
	static constexpr std::string_view units = "%";
	static constexpr codec::ieee754<float> format{};
};

/// Temperature at which the air becomes saturated, at constant pressure and constant water-vapor content
struct dew_point {
	static constexpr uint32_t tag = 20;
	static constexpr std::string_view name = "Dew Point";
	static constexpr std::string_view units = "degC";
	static constexpr codec::ieee754<float> format{};
};

/// Decrease of the temperature with altitude (-dT/dz) in dry air
struct temperature_lapse_rate {
	static constexpr uint32_t tag = 21;
	static constexpr std::string_view name = "Temperature Lapse Rate";
	static constexpr std::string_view units = "degC/m";
	static constexpr codec::ieee754<float> format{};
};

/// Description of the lighting, for example the time of day ("dawn") and the clouds ("partly cloudy")
struct lighting_conditions {
	static constexpr uint32_t tag = 22;
	static constexpr std::string_view name = "Lighting Conditions";
	static constexpr codec::string format{.max_length = 40};
};

/// Color temperature of the ambient scene
struct ambient_light_color_temperature {
	static constexpr uint32_t tag = 23;
	static constexpr std::string_view name = "Ambient Light Color Temperature";
	static constexpr std::string_view units = "K";
	static constexpr codec::ieee754<float> format{};
};

/// Description of the visibility, for example "clear" or "haze"
struct visibility_conditions {
	static constexpr uint32_t tag = 24;
	static constexpr std::string_view name = "Visibility Conditions";
	static constexpr codec::string format{.max_length = 40};
};

/// Visibility range from the Koschmieder formula
struct visibility {
	static constexpr uint32_t tag = 25;
	static constexpr std::string_view name = "Visibility";
	static constexpr std::string_view units = "km";
	static constexpr codec::ieee754<float> format{};
};

/// Visible light from the sky that air, water, dust, or clouds scatter to the ground
struct solar_illumination_diffuse {
	static constexpr uint32_t tag = 26;
	static constexpr std::string_view name = "Solar Illumination - Diffuse";
	static constexpr std::string_view units = "lx";
	static constexpr codec::ieee754<float> format{};
};

/// Visible light from the sun that goes through the atmosphere without scattering or absorption
struct solar_illumination_direct {
	static constexpr uint32_t tag = 27;
	static constexpr std::string_view name = "Solar Illumination - Direct";
	static constexpr std::string_view units = "lx";
	static constexpr codec::ieee754<float> format{};
};

/// Radiation of all wavelengths from the sun that goes through the atmosphere without scattering or absorption
struct total_illumination_direct {
	static constexpr uint32_t tag = 28;
	static constexpr std::string_view name = "Total Illumination - Direct";
	static constexpr std::string_view units = "lx";
	static constexpr codec::ieee754<float> format{};
};

/// One byte: 0 (no fog) or 1 (fog). Another value fails with `field_error::invalid_value`.
struct fog_presence {
	static constexpr uint32_t tag = 29;
	static constexpr std::string_view name = "Fog Presence";
	static constexpr codec::enumeration<fog> format{};
};

struct fog_thickness {
	static constexpr uint32_t tag = 30;
	static constexpr std::string_view name = "Fog Thickness";
	static constexpr std::string_view units = "m";
	static constexpr codec::ieee754<float> format{};
};

/// Part of the scene that fog covers
struct fog_cover_percentage {
	static constexpr uint32_t tag = 31;
	static constexpr std::string_view name = "Fog Cover Percentage";
	static constexpr std::string_view units = "%";
	static constexpr codec::ieee754<float> format{};
};

/// Fraction of the light that fog scatters or absorbs, for each unit of distance
struct fog_extinction_coefficient {
	static constexpr uint32_t tag = 32;
	static constexpr std::string_view name = "Fog Extinction Coefficient";
	static constexpr std::string_view units = "1/m";
	static constexpr codec::ieee754<float> format{};
};

/// Description of the precipitation, for example "rain" or "snow"
struct precipitation_type {
	static constexpr uint32_t tag = 33;
	static constexpr std::string_view name = "Precipitation Type";
	static constexpr codec::string format{.max_length = 40};
};

struct precipitation_rate {
	static constexpr uint32_t tag = 34;
	static constexpr std::string_view name = "Precipitation Rate";
	static constexpr std::string_view units = "mm/h";
	static constexpr codec::ieee754<float> format{};
};

/// Fried parameter (r0): the transverse coherence length at 500 nm
struct r_naught {
	static constexpr uint32_t tag = 35;
	static constexpr std::string_view name = "R Naught";
	static constexpr std::string_view units = "cm";
	static constexpr codec::ieee754<float> format{};
};

/// Isoplanatic angle: how fast the atmospheric turbulence changes with the viewing angle from a receiver
struct theta_naught {
	static constexpr uint32_t tag = 36;
	static constexpr std::string_view name = "Theta Naught";
	static constexpr std::string_view units = "urad";
	static constexpr codec::ieee754<float> format{};
};

/// Time scale of the atmospheric turbulence
struct greenwood_frequency {
	static constexpr uint32_t tag = 37;
	static constexpr std::string_view name = "Greenwood Frequency";
	static constexpr std::string_view units = "Hz";
	static constexpr codec::ieee754<float> format{};
};

/// Log-amplitude variance for propagation through a medium with a random index of refraction. It has no unit.
struct rytov_parameter {
	static constexpr uint32_t tag = 38;
	static constexpr std::string_view name = "Rytov Parameter";
	static constexpr codec::ieee754<float> format{};
};

/// First-order tip and tilt tracking frequency that an adaptive optics system needs
struct tyler_tracking_frequency {
	static constexpr uint32_t tag = 39;
	static constexpr std::string_view name = "Tyler Tracking Frequency";
	static constexpr std::string_view units = "Hz";
	static constexpr codec::ieee754<float> format{};
};

/// MISB ST 0809 Meteorological Metadata Local Set
struct meteorological_ls {
	static constexpr std::string_view name = "Meteorological Metadata Local Set";

	/// The sixth byte is 0x2B, as ST 0809 and ST 0807 give it. Most MISB local sets use 0x0B.
	static constexpr universal_key key = ul("06 0E 2B 34 02 2B 01 01 0E 01 03 01 0E 00 00 00");

	using fields = field_list<
	    precision_time_stamp,
	    version,
	    reporting_id,
	    device_latitude,
	    device_longitude,
	    barometric_pressure,
	    black_globe_temperature,
	    wet_bulb_temperature_heat_index,
	    cloud_cover_percentage,
	    cloud_sky_cover_type,
	    cloud_base_height,
	    cloud_top_height,
	    cloud_thickness,
	    carbon_dioxide_concentration,
	    ozone_concentration,
	    temperature,
	    dry_bulb_temperature,
	    wet_bulb_temperature,
	    relative_humidity,
	    dew_point,
	    temperature_lapse_rate,
	    lighting_conditions,
	    ambient_light_color_temperature,
	    visibility_conditions,
	    visibility,
	    solar_illumination_diffuse,
	    solar_illumination_direct,
	    total_illumination_direct,
	    fog_presence,
	    fog_thickness,
	    fog_cover_percentage,
	    fog_extinction_coefficient,
	    precipitation_type,
	    precipitation_rate,
	    r_naught,
	    theta_naught,
	    greenwood_frequency,
	    rytov_parameter,
	    tyler_tracking_frequency>;
};

}  // namespace klv::misb0809
