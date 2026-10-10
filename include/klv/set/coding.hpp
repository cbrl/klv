/**
 * @file klv/set/coding.hpp
 * @brief Item ids of local sets and universal sets: display text, reading, writing, and the field lookup
 *
 * A local set identifies each item with a BER-OID tag. A universal set identifies each item with a 16-byte key. The
 * compiler builds the lookup table of each set from `S::fields`. A local set gets a dense table of tags. A universal
 * set gets a sorted table of keys.
 */

#pragma once

#include "klv/bytes/byte_reader.hpp"
#include "klv/bytes/byte_writer.hpp"
#include "klv/bytes/hex.hpp"
#include "klv/bytes/universal_key.hpp"
#include "klv/error.hpp"
#include "klv/set/definition.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <format>
#include <limits>
#include <string>
#include <variant>

namespace klv {

/// The id of an item: a BER-OID tag in a local set, or a 16-byte key in a universal set
using item_id = std::variant<uint32_t, universal_key>;

/// Display text of an item id: "tag 5", or "key" and the key in hex
[[nodiscard]]
inline auto to_string(item_id const& id) -> std::string {
	if (auto const* tag = std::get_if<uint32_t>(&id)) {
		return std::format("tag {}", *tag);
	}
	return std::format("key {}", to_hex(std::get<universal_key>(id)));
}

namespace detail {

/// Read the id of one item of the set `S`
template<set_definition S>
[[nodiscard]]
auto read_id(byte_reader& reader) -> result<item_id_t<S>> {
	if constexpr (universal_set_definition<S>) {
		return reader.read_universal_key();
	}
	else {
		return reader.read_ber_oid();
	}
}

/// Write the id of one item of the set `S`
template<set_definition S>
auto put_id(byte_writer& out, item_id_t<S> const& id) -> void {
	if constexpr (universal_set_definition<S>) {
		out.put_bytes(id);
	}
	else {
		out.put_ber_oid(id);
	}
}

/// The result of `field_index` for an id that the set does not define
inline constexpr auto no_field = std::numeric_limits<size_t>::max();

/// Field index for each tag of a local set, or -1
template<local_set_definition S>
inline constexpr auto tag_index = [] {
	using info = set_info<S>;
	auto table = std::array<int16_t, size_t{info::max_tag} + 1>{};
	table.fill(-1);
	for (auto i = size_t{0}; i < info::count; ++i) {
		table[info::ids[i]] = static_cast<int16_t>(i);
	}
	return table;
}();

/// One key of a universal set, and the index of its field
struct key_entry {
	universal_key key{};
	size_t index = 0;
};

/// The keys of a universal set, sorted with `key_less`
template<universal_set_definition S>
inline constexpr auto sorted_keys = [] {
	using info = set_info<S>;
	auto table = std::array<key_entry, info::count>{};
	for (auto i = size_t{0}; i < info::count; ++i) {
		table[i] = key_entry{.key = info::ids[i], .index = i};
	}
	std::ranges::sort(table, key_less, &key_entry::key);
	return table;
}();

/// Index of the field of `S` with the id `id` in `S::fields`, or `no_field`. A key can have any version byte.
template<set_definition S>
[[nodiscard]]
constexpr auto field_index(item_id_t<S> const& id) noexcept -> size_t {
	if constexpr (universal_set_definition<S>) {
		auto const& table = sorted_keys<S>;
		auto const found = std::ranges::lower_bound(table, id, key_less, &key_entry::key);
		if (found == table.end() || key_less(id, found->key)) {
			return no_field;
		}
		return found->index;
	}
	else {
		auto const& table = tag_index<S>;
		if (id >= table.size() || table[id] < 0) {
			return no_field;
		}
		return static_cast<size_t>(table[id]);
	}
}

}  // namespace detail

}  // namespace klv
