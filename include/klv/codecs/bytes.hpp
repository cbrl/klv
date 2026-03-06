/**
 * @file klv/codecs/bytes.hpp
 * @brief Raw bytes codec
 */

#pragma once

#include "klv/bytes/byte_writer.hpp"
#include "klv/bytes/hex.hpp"
#include "klv/bytes/types.hpp"
#include "klv/field.hpp"

#include <string>

namespace klv::codec {

/**
 * @brief Raw bytes without interpretation
 */
struct bytes {
	using value_type = byte_vector;

	[[nodiscard]]
	auto decode(byte_span in) const -> field<byte_vector> {
		return byte_vector(in.begin(), in.end());
	}

	[[nodiscard]]
	auto encode(byte_vector const& value, byte_writer& out) const -> encode_result {
		out.put_bytes(value);
		return {};
	}

	[[nodiscard]]
	auto to_string(byte_vector const& value) const -> std::string {
		return to_hex(value);
	}
};

}  // namespace klv::codec
