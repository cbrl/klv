/**
 * @file klv/misb1002/fields.hpp
 * @brief MISB ST 1002 Range Image Local Set field descriptors
 */

#pragma once

#include "klv/codec.hpp"
#include "klv/misb1002/enumerations.hpp"
#include "klv/misb1002/section_data.hpp"
#include "klv/misb1202.hpp"

#include <cstdint>
#include <string_view>

namespace klv::misb1002 {

struct precision_time_stamp {
	static constexpr uint32_t tag = 1;
	static constexpr std::string_view name = "Precision Time Stamp";
	static constexpr codec::timestamp format{};
};

struct document_version {
	static constexpr uint32_t tag = 11;
	static constexpr std::string_view name = "Document Version";
	static constexpr codec::ber_oid format{};
};

struct range_image_enumerations_pack {
	static constexpr uint32_t tag = 12;
	static constexpr std::string_view name = "Range Image Enumerations";
	static constexpr range_image_enumerations_codec format{};
};

/// Single point range measurement: distance from the principal point to a point in the scene
struct single_point_range_measurement {
	static constexpr uint32_t tag = 13;
	static constexpr std::string_view name = "Single Point Range Measurement";
	static constexpr std::string_view units = "m";
	static constexpr codec::ieee754_variable format{};
};

struct single_point_range_measurement_uncertainty {
	static constexpr uint32_t tag = 14;
	static constexpr std::string_view name = "Single Point Range Measurement Uncertainty";
	static constexpr std::string_view units = "m";
	static constexpr codec::ieee754_variable format{};
};

/// Image row of the single point range measurement
struct single_point_range_measurement_row {
	static constexpr uint32_t tag = 15;
	static constexpr std::string_view name = "Single Point Range Measurement Row Coordinate";
	static constexpr codec::ieee754_variable format{};
};

/// Image column of the single point range measurement
struct single_point_range_measurement_column {
	static constexpr uint32_t tag = 16;
	static constexpr std::string_view name = "Single Point Range Measurement Column Coordinate";
	static constexpr codec::ieee754_variable format{};
};

struct number_of_sections_in_x {
	static constexpr uint32_t tag = 17;
	static constexpr std::string_view name = "Number of Sections in X";
	static constexpr codec::ber_oid format{};
};

struct number_of_sections_in_y {
	static constexpr uint32_t tag = 18;
	static constexpr std::string_view name = "Number of Sections in Y";
	static constexpr codec::ber_oid format{};
};

/// Maps the range image to the image of the collaborating sensor
struct generalized_transformation_local_set {
	static constexpr uint32_t tag = 19;
	static constexpr std::string_view name = "Generalized Transformation Local Set";
	static constexpr codec::nested<misb1202::generalized_transformation_ls> format{};
};

/// One pack for each section. The set can hold the field more than once.
struct section_data_pack {
	static constexpr uint32_t tag = 20;
	static constexpr std::string_view name = "Section Data Pack";
	static constexpr codec::repeated<section_data_codec> format{};
};

/// CRC-16-CCITT of a stand-alone packet (see `range_image_ls`)
struct checksum {
	static constexpr uint32_t tag = 21;
	static constexpr std::string_view name = "Checksum";
	static constexpr codec::integer<uint16_t> format{};
};

}  // namespace klv::misb1002
