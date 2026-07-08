/**
 * @file klv/misb0903/ontology_ls.hpp
 * @brief MISB ST 0903 Ontology Local Set (VMTI tag 103): an ontology that classifies targets
 */

#pragma once

#include "klv/codec.hpp"
#include "klv/set.hpp"

#include <cstdint>
#include <string_view>

namespace klv::misb0903 {

struct ontology_id {
	static constexpr uint32_t tag = 1;
	static constexpr std::string_view name = "ID";
	static constexpr codec::integer<uint64_t> format{.length = codec::variable_length};
};

/// ID of a related ontology
struct ontology_parent_id {
	static constexpr uint32_t tag = 2;
	static constexpr std::string_view name = "Parent ID";
	static constexpr codec::integer<uint64_t> format{.length = codec::variable_length};
};

/// IRI of the ontology
struct ontology_iri {
	static constexpr uint32_t tag = 3;
	static constexpr std::string_view name = "IRI";
	static constexpr codec::string format{};
};

/// IRI of an entity in the ontology
struct ontology_entity {
	static constexpr uint32_t tag = 4;
	static constexpr std::string_view name = "Entity";
	static constexpr codec::string format{};
};

/// IRI of the ontology version
struct ontology_version {
	static constexpr uint32_t tag = 5;
	static constexpr std::string_view name = "Version";
	static constexpr codec::string format{};
};

/// Name of the entity in the ontology
struct ontology_label {
	static constexpr uint32_t tag = 6;
	static constexpr std::string_view name = "Label";
	static constexpr codec::string format{};
};

/// MISB ST 0903 Ontology Local Set
struct ontology_ls {
	static constexpr std::string_view name = "Ontology Local Set";

	using fields = field_list<
	    ontology_id,
	    ontology_parent_id,
	    ontology_iri,
	    ontology_entity,
	    ontology_version,
	    ontology_label>;
};

}  // namespace klv::misb0903
