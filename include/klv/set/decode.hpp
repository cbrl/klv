/**
 * @file klv/set/decode.hpp
 * @brief Local set decoding
 *
 * The compiler builds the tag dispatch table of each set from `S::fields`.
 */

#pragma once

#include "klv/bytes/byte_reader.hpp"
#include "klv/bytes/types.hpp"
#include "klv/error.hpp"
#include "klv/set/definition.hpp"
#include "klv/set/record.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <format>
#include <utility>

namespace klv {

/// Decode settings
struct decode_options {
	bool verify_checksum = true;  ///< Packet decoding: verify the checksum of a set that names a checksum field
	bool keep_unknown = true;  ///< Keep fields with unknown tags in `record::unknown()`
};

namespace detail {

template<local_set_definition S, typename E>
auto decode_into(record<S>& rec, byte_span bytes) -> void {
	rec.template assign<E>(E::format.decode(bytes));
}

template<local_set_definition S, typename List = typename S::fields>
struct set_table;

template<local_set_definition S, typename... F>
struct set_table<S, field_list<F...>> {
	using info = list_info<field_list<F...>>;
	using decode_function = auto (*)(record<S>&, byte_span) -> void;

	static constexpr auto count = info::count;
	static constexpr auto max_tag = info::max_tag;
	static constexpr auto decoders = std::array<decode_function, count>{&decode_into<S, F>...};

	/// Tag to field index, or -1
	static constexpr auto index = [] {
		auto table = std::array<int16_t, size_t{max_tag} + 1>{};
		table.fill(-1);
		for (auto i = size_t{0}; i < count; ++i) {
			table[info::tags[i]] = static_cast<int16_t>(i);
		}
		return table;
	}();

	/// Typed decoder for `tag`, or null when the set does not define the tag
	[[nodiscard]]
	static constexpr auto find(uint32_t tag) noexcept -> decode_function {
		if (tag > max_tag || index[tag] < 0) {
			return nullptr;
		}
		return decoders[static_cast<size_t>(index[tag])];
	}
};

/// The last field of a decoded set, for checksum verification
struct last_field_info {
	bool present = false;
	uint32_t tag = 0;
	byte_span value;  ///< Value bytes of the field, inside the decoded buffer
};

template<local_set_definition S>
[[nodiscard]]
auto decode_fields(
	byte_span value,
	size_t base_offset,
	decode_options const& options,
	last_field_info* last
) -> result<record<S>> {
	using table = set_table<S>;

	auto const fail = [](error problem) {
		problem.message = std::format("{}: {}", S::name, problem.message);
		return std::unexpected(std::move(problem));
	};

	auto reader = byte_reader{value, base_offset};
	auto rec = record<S>{};

	while (!reader.empty()) {
		auto const tag = reader.read_ber_oid();
		if (!tag) {
			return fail(tag.error());
		}

		auto const length = reader.read_ber_length();
		if (!length) {
			return fail(length.error());
		}

		auto const bytes = reader.take(*length);
		if (!bytes) {
			return fail(bytes.error());
		}

		if (auto const decode = table::find(*tag); decode != nullptr) {
			decode(rec, *bytes);
		}
		else if (options.keep_unknown) {
			rec.add_unknown(unknown_field{.tag = *tag, .value = byte_vector(bytes->begin(), bytes->end())});
		}

		if (last != nullptr) {
			*last = last_field_info{.present = true, .tag = *tag, .value = *bytes};
		}
	}

	return rec;
}

}  // namespace detail

/**
 * @brief Decode the value bytes of a local set
 *
 * A malformed tag, length, or field size fails the whole set. A field value that its codec rejects (for example a
 * wrong length or the error value) only marks that field.
 */
template<local_set_definition S>
[[nodiscard]]
auto decode_set(byte_span value, decode_options const& options = {}) -> result<record<S>> {
	return detail::decode_fields<S>(value, 0, options, nullptr);
}

}  // namespace klv
