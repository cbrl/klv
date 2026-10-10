/**
 * @file klv/set/decode.hpp
 * @brief Set decoding
 *
 * The compiler builds the dispatch table of each set from `S::fields` (see `klv/set/coding.hpp`).
 */

#pragma once

#include "klv/bytes/byte_reader.hpp"
#include "klv/bytes/types.hpp"
#include "klv/codecs/codec_type.hpp"
#include "klv/error.hpp"
#include "klv/field.hpp"
#include "klv/set/coding.hpp"
#include "klv/set/definition.hpp"
#include "klv/set/record.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <format>
#include <span>
#include <utility>
#include <vector>

namespace klv {

/// Decode settings
struct decode_options {
	bool verify_checksum = true;  ///< Packet decoding: verify the checksum of a set that names a checksum field
	bool keep_unknown = true;  ///< Keep items with unknown ids in `record::unknown()`
};

namespace detail {

/// Decode one occurrence with `codec`. A covering codec also gets the tags of the items before the occurrence.
template<typename C>
[[nodiscard]]
auto decode_occurrence(C const& codec, byte_span bytes, std::span<uint32_t const> preceding) {
	if constexpr (covering_codec<C>) {
		return codec.decode(bytes, preceding);
	}
	else {
		static_cast<void>(preceding);
		return codec.decode(bytes);
	}
}

/// Decode one occurrence of field `E`. Each field of each set instantiates this function. Thus, it stays small.
template<set_definition S, typename E>
auto decode_into(record<S>& rec, byte_span bytes, std::span<uint32_t const> preceding) -> void {
	auto& value = static_cast<field_slot<E>&>(record_access::slots(rec)).value;

	if constexpr (repeated_codec<codec_of<E>>) {
		// A zero-length occurrence has no item. It makes the field empty when no other occurrence came before it.
		if (bytes.empty()) {
			if (!value && value.error() == field_error::absent) {
				value = std::unexpected(field_error::empty);
			}
			return;
		}

		// An earlier occurrence that did not decode keeps its error
		if (!value && value.error() != field_error::absent && value.error() != field_error::empty) {
			return;
		}

		auto item = decode_occurrence(E::format.item, bytes, preceding);
		if (!item) {
			value = std::unexpected(item.error());
			return;
		}
		if (!value) {
			value.emplace();
		}
		value->push_back(std::move(*item));
	}
	else if (bytes.empty()) {
		value = std::unexpected(field_error::empty);
	}
	else {
		value = decode_occurrence(E::format, bytes, preceding);
	}
}

template<set_definition S, typename List = typename S::fields>
struct set_decoders;

template<set_definition S, typename... F>
struct set_decoders<S, field_list<F...>> {
	using decode_function = auto (*)(record<S>&, byte_span, std::span<uint32_t const>) -> void;

	static constexpr auto functions = std::array<decode_function, sizeof...(F)>{&decode_into<S, F>...};

	/// Typed decoder for `id`, or null when the set does not define the id
	[[nodiscard]]
	static constexpr auto find(item_id_t<S> const& id) noexcept -> decode_function {
		auto const index = field_index<S>(id);
		return index == no_field ? nullptr : functions[index];
	}
};

/// The last item of a decoded set, for checksum verification
template<typename Id>
struct last_field_info {
	bool present = false;
	Id id{};
	byte_span value;  ///< Value bytes of the item, inside the decoded buffer
};

template<set_definition S>
[[nodiscard]]
auto decode_fields(
	byte_span value,
	size_t base_offset,
	decode_options const& options,
	last_field_info<item_id_t<S>>* last
) -> result<record<S>> {
	auto const fail = [](error problem) {
		problem.message = std::format("{}: {}", S::name, problem.message);
		return std::unexpected(std::move(problem));
	};

	auto reader = byte_reader{value, base_offset};
	auto rec = record<S>{};

	// Tags of the decoded items, for the fields that cover the items before them
	auto history = std::vector<uint32_t>{};

	while (!reader.empty()) {
		auto const id = read_id<S>(reader);
		if (!id) {
			return fail(id.error());
		}

		auto const length = reader.read_ber_length();
		if (!length) {
			return fail(length.error());
		}

		auto const bytes = reader.take(*length);
		if (!bytes) {
			return fail(bytes.error());
		}

		if (auto const decode = set_decoders<S>::find(*id); decode != nullptr) {
			decode(rec, *bytes, history);
		}
		else if (options.keep_unknown) {
			rec.add_unknown({.id = *id, .value = byte_vector(bytes->begin(), bytes->end())});
		}

		if constexpr (has_covering_field<S>) {
			history.push_back(*id);
		}

		if (last != nullptr) {
			*last = last_field_info<item_id_t<S>>{.present = true, .id = *id, .value = *bytes};
		}
	}

	return rec;
}

}  // namespace detail

/**
 * @brief Decode the value bytes of a set
 *
 * A malformed tag, key, length, or field size fails the whole set. A field value that its codec rejects (for example a
 * wrong length or the error value) only marks that field. A field with a length of zero is `field_error::empty`, for
 * each format. A field of a covering codec (for example an SDCC-FLP) gets the tags of the items before it. A key in a
 * universal set can have any version byte.
 */
template<set_definition S>
[[nodiscard]]
auto decode_set(byte_span value, decode_options const& options = {}) -> result<record<S>> {
	return detail::decode_fields<S>(value, 0, options, nullptr);
}

}  // namespace klv
