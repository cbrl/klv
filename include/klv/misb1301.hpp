/**
 * @file misb1301.hpp
 * @brief MISB ST 1301 - Motion Imagery Identification System (MIIS) Augmentation Identifiers
 *
 * The Augmentation Identifiers Local Set holds a MISB ST 1204 Core Identifier and a list of augmentation
 * identifiers. Augmentation identifiers give more information about the Motion Imagery, for example a Ground Station
 * ID. Any system in the data flow can add them. The set has no checksum.
 *
 * ST 1301.2 defines only the Version (tag 3) and the Core Identifier (tag 4). Tags 1 and 2 are deprecated, and tags 5
 * to 7 are reserved. Augmentation items start at tag 8, but ST 1301.2 defines none. Decoding keeps the items of these
 * tags in `record::unknown()`, and encoding writes them again.
 *
 * ST 1301 requires the Version and the Core Identifier in each set (requirements ST 1301.2-02 and ST 1301.2-03). The
 * library does not check this.
 */

#pragma once

#include "klv/bytes.hpp"
#include "klv/codec.hpp"
#include "klv/misb1204/core_id_codec.hpp"
#include "klv/set.hpp"

#include <cstdint>
#include <string_view>

namespace klv::misb1301 {

/// Minor version of ST 1301, for example 2 for ST 1301.2
struct document_version {
	static constexpr uint32_t tag = 3;
	static constexpr std::string_view name = "Version";
	static constexpr codec::ber_oid format{};
};

/// MISB ST 1204 Core Identifier of the Motion Imagery source
struct core_identifier {
	static constexpr uint32_t tag = 4;
	static constexpr std::string_view name = "Core Identifier";
	static constexpr misb1204::core_id_codec format{};
};

/// MISB ST 1301 Augmentation Identifiers Local Set
struct augmentation_ls {
	static constexpr std::string_view name = "MIIS Augmentation Identifiers Local Set";
	static constexpr universal_key key = ul("06 0E 2B 34 02 0B 01 01 0E 01 03 05 03 00 00 00");

	using fields = field_list<document_version, core_identifier>;
};

}  // namespace klv::misb1301
