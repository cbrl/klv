/**
 * @file klv/misb0903/vchip_ls.hpp
 * @brief MISB ST 0903 VChip Local Set (VTarget tags 105 and 106): an image chip of a target
 */

#pragma once

#include "klv/codec.hpp"
#include "klv/set.hpp"

#include <cstdint>
#include <string_view>

namespace klv::misb0903 {

/// IANA image media subtype: "jpeg" or "png"
struct chip_image_type {
	static constexpr uint32_t tag = 1;
	static constexpr std::string_view name = "Image Type";
	static constexpr codec::string format{};
};

/// IRI of an image on a server
struct chip_image_iri {
	static constexpr uint32_t tag = 2;
	static constexpr std::string_view name = "Image IRI";
	static constexpr codec::string format{};
};

struct chip_embedded_image {
	static constexpr uint32_t tag = 3;
	static constexpr std::string_view name = "Embedded Image";
	static constexpr codec::bytes format{};
};

/// MISB ST 0903 VChip Local Set
struct vchip_ls {
	static constexpr std::string_view name = "VChip Local Set";

	using fields = field_list<chip_image_type, chip_image_iri, chip_embedded_image>;
};

}  // namespace klv::misb0903
