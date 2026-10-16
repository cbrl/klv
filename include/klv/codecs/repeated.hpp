/**
 * @file klv/codecs/repeated.hpp
 * @brief Codec for a field that a set can hold more than once
 */

#pragma once

#include "klv/bytes/byte_writer.hpp"
#include "klv/bytes/types.hpp"
#include "klv/codecs/codec_type.hpp"
#include "klv/codecs/pack.hpp"
#include "klv/field.hpp"

#include <expected>
#include <string>
#include <utility>
#include <vector>

namespace klv::codec {

/**
 * @brief A field that a set can hold more than once (for example the MISB ST 0601 Amend Local Set)
 *
 * The value is a list with one item for each occurrence, in decode order. Set decoding appends one item for each
 * occurrence, and set encoding writes one field for each item. When an occurrence does not decode, the field holds
 * that error, and set decoding ignores the later occurrences. `record::add` appends one item.
 *
 * @code
 * static constexpr codec::repeated<codec::nested<poi_ls>> format{};
 * @endcode
 *
 * `Item` has no constraint here. A set can hold a list of its own records (MISB ST 1607). The `codec_type` check of
 * such an item needs the complete record type. Thus, the set definition does the check when it uses the field.
 */
template<typename Item>
struct repeated {
	using item_type = typename Item::value_type;
	using value_type = std::vector<item_type>;

	Item item{};

	/// Decode one occurrence as a list of one item
	[[nodiscard]]
	auto decode(byte_span in) const -> field<value_type> {
		return item.decode(in).transform([](item_type&& value) {
			auto list = value_type{};
			list.push_back(std::move(value));
			return list;
		});
	}

	/// Encode a list of exactly one item. Set encoding does not call this function. It encodes each item separately.
	[[nodiscard]]
	auto encode(value_type const& value, byte_writer& out) const -> encode_result {
		if (value.size() != 1) {
			return std::unexpected(field_error::invalid_value);
		}
		return item.encode(value.front(), out);
	}

	[[nodiscard]]
	auto to_string(value_type const& value) const -> std::string {
		return detail::list_text(value, [&](item_type const& entry) { return item.to_string(entry); });
	}
};

}  // namespace klv::codec
