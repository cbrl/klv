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

#include <concepts>
#include <cstdint>
#include <expected>
#include <format>
#include <type_traits>
#include <utility>

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

/// Write the fields of a record in definition order, then its unknown fields. The checksum field is not written.
template<local_set_definition S>
[[nodiscard]]
auto encode_fields(record<S> const& rec, byte_writer& out) -> result<void> {
	auto status = result<void>{};
	auto const& slots = record_access::slots(rec);

	auto const encode_one = [&]<typename E>(std::type_identity<E>) -> bool {
		if constexpr (is_checksum_field<S, E>()) {
			return true;
		}
		else {
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

	for (auto const& unknown : rec.unknown()) {
		out.put_ber_oid(unknown.tag);
		out.put_ber_length(unknown.value.size());
		out.put_bytes(unknown.value);
	}
	return {};
}

}  // namespace detail

/**
 * @brief Append the value bytes of a local set to `out`
 *
 * The function writes the fields in definition order, then the unknown fields. It skips absent fields and fields that
 * did not decode. An empty field gets a length of zero. The function does not write a checksum field (see
 * `encode_packet`). On failure, `out` keeps its old size.
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
