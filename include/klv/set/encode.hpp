/**
 * @file klv/set/encode.hpp
 * @brief Local set encoding
 */

#pragma once

#include "klv/bytes/byte_writer.hpp"
#include "klv/bytes/types.hpp"
#include "klv/codecs/codec_type.hpp"
#include "klv/error.hpp"
#include "klv/field.hpp"
#include "klv/set/definition.hpp"
#include "klv/set/record.hpp"

#include <algorithm>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <format>
#include <span>
#include <type_traits>
#include <utility>
#include <vector>

namespace klv {

namespace detail {

template<local_set_definition S, typename E>
[[nodiscard]]
constexpr auto is_checksum_field() noexcept -> bool {
	if constexpr (has_checksum<S>) {
		return std::same_as<E, typename S::checksum_field>;
	}
	else {
		return false;
	}
}

/// Write the tag of `E` and the value that `write_value` writes, with its length
template<field_descriptor E, typename F>
[[nodiscard]]
auto encode_occurrence(byte_writer& out, F&& write_value) -> result<void> {
	auto const start = out.size();
	out.put_ber_oid(E::tag);

	auto const status = out.put_length_prefixed(std::forward<F>(write_value));
	if (!status) {
		out.truncate(start);
		return make_error(
		    error_code::invalid_value,
		    start,
		    std::format(
		        "Cannot encode field '{}' (tag {}): {}",
		        E::name,
		        static_cast<uint32_t>(E::tag),
		        klv::to_string(status.error())
		    )
		);
	}
	return {};
}

/// Write one field. A repeated field writes one occurrence for each item.
template<field_descriptor E>
[[nodiscard]]
auto encode_field(field<value_t<E>> const& value, byte_writer& out) -> result<void> {
	// An empty field has a length of zero
	if (!value && value.error() == field_error::empty) {
		return encode_occurrence<E>(out, []() -> encode_result { return {}; });
	}

	// An absent field, or a field that did not decode, has nothing to write
	if (!value && value.error() != field_error::error_indicator) {
		return {};
	}

	if constexpr (repeated_codec<codec_of<E>>) {
		if (!value) {
			return encode_occurrence<E>(out, []() -> encode_result {
				return std::unexpected(field_error::invalid_value);
			});
		}
		for (auto const& item : *value) {
			auto status = encode_occurrence<E>(out, [&] { return E::format.item.encode(item, out); });
			if (!status) {
				return status;
			}
		}
		return {};
	}
	else {
		return encode_occurrence<E>(out, [&]() -> encode_result {
			if (value) {
				return E::format.encode(*value, out);
			}
			if constexpr (has_error_indicator<codec_of<E>>) {
				return E::format.encode_error_indicator(out);
			}
			else {
				return std::unexpected(field_error::invalid_value);
			}
		});
	}
}

/// Items that covering items (for example SDCC-FLPs) write directly before themselves
struct covered_items {
	std::vector<uint32_t> tags;  ///< Tags of the covered items
	std::vector<bool> unknown_written;  ///< The unknown fields of the record that a covering item wrote

	[[nodiscard]]
	auto holds(uint32_t tag) const noexcept -> bool {
		return std::ranges::contains(tags, tag);
	}
};

inline auto put_unknown(unknown_field const& unknown, byte_writer& out) -> void {
	out.put_ber_oid(unknown.tag);
	out.put_ber_length(unknown.value.size());
	out.put_bytes(unknown.value);
}

/// Collect the tags that the covering items of a record cover. Two covering items cannot cover the same item.
template<local_set_definition S>
[[nodiscard]]
auto collect_covered(record<S> const& rec, covered_items& covered, byte_writer const& out) -> result<void> {
	auto const& slots = record_access::slots(rec);
	auto const append = [&](std::span<uint32_t const> tags) {
		covered.tags.insert(covered.tags.end(), tags.begin(), tags.end());
	};

	auto const collect_one = [&]<typename E>(std::type_identity<E>) {
		if constexpr (covering_field<E>) {
			auto const& value = static_cast<field_slot<E> const&>(slots).value;
			if (!value) {
				return;
			}
			if constexpr (repeated_codec<codec_of<E>>) {
				for (auto const& item : *value) {
					append(E::format.item.covered_tags(item));
				}
			}
			else {
				append(E::format.covered_tags(*value));
			}
		}
	};

	[&]<typename... E>(field_list<E...>) {
		(collect_one(std::type_identity<E>{}), ...);
	}(typename S::fields{});

	auto sorted = covered.tags;
	std::ranges::sort(sorted);
	if (auto const twice = std::ranges::adjacent_find(sorted); twice != sorted.end()) {
		return make_error(
		    error_code::invalid_value,
		    out.size(),
		    std::format("{}: two covering items cover the item with tag {}", S::name, *twice)
		);
	}
	covered.unknown_written.assign(rec.unknown().size(), false);
	return {};
}

/**
 * @brief Write the items with the given tags, in order
 *
 * Each tag names a field of the set with a value, or else an unknown field of the record. A covering field and the
 * checksum field cannot be covered items.
 */
template<local_set_definition S>
[[nodiscard]]
auto encode_covered(record<S> const& rec, std::span<uint32_t const> tags, covered_items& covered, byte_writer& out)
    -> result<void> {
	auto const& slots = record_access::slots(rec);
	for (auto const tag : tags) {
		auto const start = out.size();
		auto status = result<void>{};
		auto found = false;

		auto const write_one = [&]<typename E>(std::type_identity<E>) -> bool {
			if (E::tag != tag) {
				return false;
			}
			found = true;
			if constexpr (!covering_field<E> && !is_checksum_field<S, E>()) {
				status = encode_field<E>(static_cast<field_slot<E> const&>(slots).value, out);
			}
			return true;
		};

		[&]<typename... E>(field_list<E...>) {
			static_cast<void>((write_one(std::type_identity<E>{}) || ...));
		}(typename S::fields{});

		auto const unknown = rec.unknown();
		for (auto i = size_t{0}; !found && i < unknown.size(); ++i) {
			if (unknown[i].tag == tag && !covered.unknown_written[i]) {
				put_unknown(unknown[i], out);
				covered.unknown_written[i] = true;
				found = true;
			}
		}

		if (!status) {
			return status;
		}
		if (out.size() == start) {
			return make_error(
			    error_code::invalid_value,
			    start,
			    std::format("{}: the covered item with tag {} has no value to write", S::name, tag)
			);
		}
	}
	return {};
}

/// Write each item of the covering field `E`, directly after the items that it covers
template<local_set_definition S, typename E>
[[nodiscard]]
auto encode_covering(record<S> const& rec, covered_items& covered, byte_writer& out) -> result<void> {
	auto const& value = static_cast<field_slot<E> const&>(record_access::slots(rec)).value;
	if (!value) {
		return encode_field<E>(value, out);
	}

	if constexpr (repeated_codec<codec_of<E>>) {
		for (auto const& item : *value) {
			auto status = encode_covered(rec, E::format.item.covered_tags(item), covered, out);
			if (status) {
				status = encode_occurrence<E>(out, [&] { return E::format.item.encode(item, out); });
			}
			if (!status) {
				return status;
			}
		}
		return {};
	}
	else {
		auto status = encode_covered(rec, E::format.covered_tags(*value), covered, out);
		return status ? encode_field<E>(value, out) : status;
	}
}

/**
 * @brief Write the fields of a record in definition order, then its unknown fields
 *
 * A covering item (for example an SDCC-FLP) comes directly after the items that it covers. The checksum field is not
 * written.
 */
template<local_set_definition S>
[[nodiscard]]
auto encode_fields(record<S> const& rec, byte_writer& out) -> result<void> {
	auto status = result<void>{};
	auto const& slots = record_access::slots(rec);

	auto covered = covered_items{};
	if constexpr (has_covering_field<S>) {
		status = collect_covered(rec, covered, out);
		if (!status) {
			return status;
		}
	}

	auto const encode_one = [&]<typename E>(std::type_identity<E>) -> bool {
		if constexpr (is_checksum_field<S, E>()) {
			return true;
		}
		else if constexpr (covering_field<E>) {
			status = encode_covering<S, E>(rec, covered, out);
			return status.has_value();
		}
		else {
			// A covering item writes its covered fields before itself
			if (has_covering_field<S> && covered.holds(E::tag)) {
				return true;
			}
			status = encode_field<E>(static_cast<field_slot<E> const&>(slots).value, out);
			return status.has_value();
		}
	};

	[&]<typename... E>(field_list<E...>) {
		static_cast<void>((encode_one(std::type_identity<E>{}) && ...));
	}(typename S::fields{});

	if (!status) {
		return status;
	}

	auto const unknown = rec.unknown();
	for (auto i = size_t{0}; i < unknown.size(); ++i) {
		if (covered.unknown_written.empty() || !covered.unknown_written[i]) {
			put_unknown(unknown[i], out);
		}
	}
	return {};
}

}  // namespace detail

/**
 * @brief Append the value bytes of a local set to `out`
 *
 * The function writes the fields in definition order, then the unknown fields. It skips absent fields and fields that
 * did not decode. An empty field gets a length of zero. A covering item (for example an SDCC-FLP) comes directly
 * after the items that it covers.
 *
 * The function does not write a checksum field (see `encode_packet`). On failure, `out` keeps its old size.
 */
template<local_set_definition S>
[[nodiscard]]
auto encode_set(record<S> const& rec, byte_vector& out) -> result<void> {
	auto writer = byte_writer{out};
	auto const start = writer.size();

	auto status = detail::encode_fields(rec, writer);
	if (!status) {
		writer.truncate(start);
	}
	return status;
}

/// Encode the value bytes of a local set
template<local_set_definition S>
[[nodiscard]]
auto encode_set(record<S> const& rec) -> result<byte_vector> {
	auto out = byte_vector{};
	return encode_set(rec, out).transform([&] { return std::move(out); });
}

}  // namespace klv
