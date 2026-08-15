/**
 * @file misb1202.hpp
 * @brief MISB ST 1202 - Generalized Transformation Local Set
 *
 * The set maps points from one image to another with a projective transformation:
 * - x' = (A x + B y + C) / (G x + H y + 1)
 * - y' = (D x + E y + F) / (G x + H y + 1)
 *
 * It usually arrives nested in MISB ST 1002 tag 19 (see `misb1002::generalized_transformation_local_set`).
 */

#pragma once

#include "klv/bytes.hpp"
#include "klv/codec.hpp"
#include "klv/set.hpp"

#include <cstdint>
#include <string_view>

namespace klv::misb1202 {

/// Kind of transformation (tag 11)
enum class transformation_kind : uint8_t {
	not_defined = 0,
	chipping = 1,
	child_parent = 2,
	pixel_to_image_space = 3,
	optical = 4,
};

[[nodiscard]]
constexpr auto to_string(transformation_kind value) -> std::string_view {
	using enum transformation_kind;
	switch (value) {
		case not_defined:
			return "Not Defined";
		case chipping:
			return "Chipping";
		case child_parent:
			return "Child-Parent";
		case pixel_to_image_space:
			return "Pixel to Image Space";
		case optical:
			return "Optical";
	}
	return {};
}

/// A
struct x_numerator_x_factor {
	static constexpr uint32_t tag = 1;
	static constexpr std::string_view name = "X Equation Numerator - X Factor";
	static constexpr codec::ieee754<float> format{};
};

/// B
struct x_numerator_y_factor {
	static constexpr uint32_t tag = 2;
	static constexpr std::string_view name = "X Equation Numerator - Y Factor";
	static constexpr codec::ieee754<float> format{};
};

/// C
struct x_numerator_constant {
	static constexpr uint32_t tag = 3;
	static constexpr std::string_view name = "X Equation Numerator - Constant";
	static constexpr codec::ieee754<float> format{};
};

/// D
struct y_numerator_x_factor {
	static constexpr uint32_t tag = 4;
	static constexpr std::string_view name = "Y Equation Numerator - X Factor";
	static constexpr codec::ieee754<float> format{};
};

/// E
struct y_numerator_y_factor {
	static constexpr uint32_t tag = 5;
	static constexpr std::string_view name = "Y Equation Numerator - Y Factor";
	static constexpr codec::ieee754<float> format{};
};

/// F
struct y_numerator_constant {
	static constexpr uint32_t tag = 6;
	static constexpr std::string_view name = "Y Equation Numerator - Constant";
	static constexpr codec::ieee754<float> format{};
};

/// G
struct denominator_x_factor {
	static constexpr uint32_t tag = 7;
	static constexpr std::string_view name = "Denominator - X Factor";
	static constexpr codec::ieee754<float> format{};
};

/// H
struct denominator_y_factor {
	static constexpr uint32_t tag = 8;
	static constexpr std::string_view name = "Denominator - Y Factor";
	static constexpr codec::ieee754<float> format{};
};

/// Uncertainty of items 1 to 8 before it. The set can hold it more than once. ST 1202 requires Mode 2 parse control
/// and IEEE 754 standard deviations.
struct sdcc_flp {
	static constexpr uint32_t tag = 9;
	static constexpr std::string_view name = "SDCC-FLP";
	static constexpr codec::repeated<codec::sdcc_flp> format{};
};

struct document_version {
	static constexpr uint32_t tag = 10;
	static constexpr std::string_view name = "Document Version";
	static constexpr codec::integer<uint8_t> format{};
};

struct transformation_type {
	static constexpr uint32_t tag = 11;
	static constexpr std::string_view name = "Transformation Type";
	static constexpr codec::enumeration<transformation_kind> format{.name_of = &to_string};
};

/// MISB ST 1202 Generalized Transformation Local Set
struct generalized_transformation_ls {
	static constexpr std::string_view name = "Generalized Transformation Local Set";
	static constexpr universal_key key = ul("06 0E 2B 34 02 0B 01 01 0E 01 03 05 05 00 00 00");

	using fields = field_list<
	    x_numerator_x_factor,
	    x_numerator_y_factor,
	    x_numerator_constant,
	    y_numerator_x_factor,
	    y_numerator_y_factor,
	    y_numerator_constant,
	    denominator_x_factor,
	    denominator_y_factor,
	    sdcc_flp,
	    document_version,
	    transformation_type>;
};

}  // namespace klv::misb1202
