/**
 * @file klv/misb0102/security_ls.hpp
 * @brief MISB ST 0102 Security Metadata Local Set definition
 *
 * `security_ls` lists the fields in encode order. The set usually arrives nested in another set, without a key. It
 * can also be a packet with its own key (ST 0102 section 6.7).
 */

#pragma once

#include "klv/misb0102/fields.hpp"
#include "klv/set.hpp"

#include <string_view>

namespace klv::misb0102 {

/// MISB ST 0102 Security Metadata Local Set
struct security_ls {
	static constexpr std::string_view name = "Security Local Set";
	static constexpr universal_key key = ul("06 0E 2B 34 02 03 01 01 0E 01 03 03 02 00 00 00");

	using fields = field_list<
	    security_classification,
	    classifying_country_coding_method,
	    classifying_country,
	    sci_shi_information,
	    caveats,
	    releasing_instructions,
	    classified_by,
	    derived_from,
	    classification_reason,
	    declassification_date,
	    classification_marking_system,
	    object_country_coding_method,
	    object_country_codes,
	    classification_comments,
	    umid_video,
	    umid_audio,
	    umid_data,
	    umid_system,
	    stream_id,
	    transport_stream_id,
	    item_designator_id,
	    version,
	    classifying_country_coding_method_version_date,
	    object_country_coding_method_version_date>;
};

}  // namespace klv::misb0102
