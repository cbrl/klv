/**
 * @file klv/misb0602/stream.hpp
 * @brief MISB ST 0602 annotation data stream decoding
 */

#pragma once

#include "klv/bytes.hpp"
#include "klv/misb0602/annotation_us.hpp"
#include "klv/misb0602/preface.hpp"
#include "klv/packet.hpp"

#include <generator>

namespace klv::misb0602 {

/// A decoded packet of an annotation data stream
using stream_packet = packet<byte_order_item, active_lines_per_frame_item, active_samples_per_line_item, annotation_us>;

/// Decode an annotation data stream: the preface items, the Annotation Universal Sets, and unknown packets. The
/// buffer must outlive the generator.
[[nodiscard]]
inline auto decode_stream(byte_span bytes, decode_options options = {}) -> std::generator<result<stream_packet>> {
	return decode_packets<byte_order_item, active_lines_per_frame_item, active_samples_per_line_item, annotation_us>(
	    bytes,
	    options
	);
}

}  // namespace klv::misb0602
