/**
 * @file klv/misb0903/vfeature_ls.hpp
 * @brief MISB ST 0903 VFeature Local Set (deprecated): the features of a target
 */

#pragma once

#include "klv/codec.hpp"
#include "klv/set.hpp"

#include <cstdint>
#include <string_view>

namespace klv::misb0903 {

/// Deprecated. Use the Ontology ID.
struct vfeature_schema {
	static constexpr uint32_t tag = 1;
	static constexpr std::string_view name = "Schema";
	static constexpr codec::string format{};
};

/// Deprecated. Use the Ontology ID.
struct vfeature_schema_feature {
	static constexpr uint32_t tag = 2;
	static constexpr std::string_view name = "Schema Feature";
	static constexpr codec::string format{};
};

/// ID of an ontology in the VMTI Ontology Series
struct vfeature_ontology_id {
	static constexpr uint32_t tag = 3;
	static constexpr std::string_view name = "Ontology ID";
	static constexpr codec::integer<uint64_t> format{.length = codec::variable_length};
};

struct vfeature_confidence {
	static constexpr uint32_t tag = 4;
	static constexpr std::string_view name = "Confidence";
	static constexpr std::string_view units = "%";
	static constexpr codec::imapb format{0.0, 100.0, 2};
};

/// MISB ST 0903 VFeature Local Set (VTarget tag 103, deprecated)
struct vfeature_ls {
	static constexpr std::string_view name = "VFeature Local Set";

	using fields = field_list<vfeature_schema, vfeature_schema_feature, vfeature_ontology_id, vfeature_confidence>;
};

}  // namespace klv::misb0903
