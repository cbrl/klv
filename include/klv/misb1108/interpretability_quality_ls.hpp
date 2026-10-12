/**
 * @file klv/misb1108/interpretability_quality_ls.hpp
 * @brief MISB ST 1108 Interpretability and Quality Local Set definition
 */

#pragma once

#include "klv/bytes.hpp"
#include "klv/misb1108/fields.hpp"
#include "klv/set.hpp"

#include <string_view>

namespace klv::misb1108 {

/**
 * @brief MISB ST 1108 Interpretability and Quality Local Set
 *
 * Packet decoding verifies the CRC-16-CCITT checksum (tag 11), and packet encoding computes it.
 */
struct interpretability_quality_ls {
	static constexpr std::string_view name = "Interpretability and Quality Local Set";
	static constexpr universal_key key = ul("06 0E 2B 34 02 03 01 01 0E 01 03 03 1C 00 00 00");

	using checksum_field = checksum;
	static constexpr auto checksum_function = crc16_ccitt;

	using fields = field_list<
	    assessment_point,
	    metric_period_pack,
	    window_corners_pack,
	    metric_local_set,
	    compression_type,
	    compression_profile,
	    compression_level,
	    compression_ratio,
	    stream_bitrate,
	    document_version,
	    checksum>;
};

}  // namespace klv::misb1108
