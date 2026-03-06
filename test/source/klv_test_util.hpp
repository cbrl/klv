#pragma once

#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <string_view>

#include "klv/bytes.hpp"

namespace klv_test {

/// Bytes from integer literals
[[nodiscard]]
inline auto make_bytes(std::initializer_list<unsigned> values) -> klv::byte_vector {
	auto bytes = klv::byte_vector{};
	bytes.reserve(values.size());
	for (auto const value : values) {
		bytes.push_back(static_cast<std::byte>(value));
	}
	return bytes;
}

/// Bytes from a hex string. The test fails to compile into a useful value if the text is not hex.
[[nodiscard]]
inline auto hex(std::string_view text) -> klv::byte_vector {
	return klv::from_hex(text).value();
}

/// Append one local set field (BER-OID tag, BER length, value) to `out`
inline auto append_field(klv::byte_vector& out, uint32_t tag, klv::byte_span value) -> void {
	auto writer = klv::byte_writer{out};
	writer.put_ber_oid(tag);
	writer.put_ber_length(value.size());
	writer.put_bytes(value);
}

/// Append a universal-key packet (key, BER length, value) to `out`
inline auto append_packet(klv::byte_vector& out, klv::universal_key const& key, klv::byte_span value) -> void {
	auto writer = klv::byte_writer{out};
	writer.put_bytes(key);
	writer.put_ber_length(value.size());
	writer.put_bytes(value);
}

}  // namespace klv_test
