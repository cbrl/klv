/**
 * @file klv/misb0903/vobject_ls.hpp
 * @brief MISB ST 0903 VObject Local Set: the class of a target
 */

#pragma once

#include "klv/codec.hpp"
#include "klv/set.hpp"

#include <cstdint>
#include <string_view>

namespace klv::misb0903 {

/// Deprecated. Use the Ontology ID.
struct vobject_ontology {
	static constexpr uint32_t tag = 1;
	static constexpr std::string_view name = "Ontology";
	static constexpr codec::string format{};
};

/// Deprecated. Use the Ontology ID.
struct vobject_ontology_class {
	static constexpr uint32_t tag = 2;
	static constexpr std::string_view name = "Ontology Class";
	static constexpr codec::string format{};
};

/// ID of an ontology in the VMTI Ontology Series
struct vobject_ontology_id {
	static constexpr uint32_t tag = 3;
	static constexpr std::string_view name = "Ontology ID";
	static constexpr codec::integer<uint32_t> format{.length = codec::variable_length};
};

struct vobject_confidence {
	static constexpr uint32_t tag = 4;
	static constexpr std::string_view name = "Confidence";
	static constexpr std::string_view units = "%";
	static constexpr codec::imapb format{0.0, 100.0, 2};
};

/// MISB ST 0903 VObject Local Set (VTarget tags 102 and 107)
struct vobject_ls {
	static constexpr std::string_view name = "VObject Local Set";

	using fields = field_list<vobject_ontology, vobject_ontology_class, vobject_ontology_id, vobject_confidence>;
};

}  // namespace klv::misb0903
