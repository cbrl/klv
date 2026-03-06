/**
 * @file klv/set/record.hpp
 * @brief Typed records that hold the field values of one local set
 *
 * `record<S>` has one typed slot for each field of `S`. `record<S>::get<E>()` does not compile when `E` is not a field
 * of `S`.
 */

#pragma once

#include "klv/bytes/types.hpp"
#include "klv/codecs/codec_type.hpp"
#include "klv/field.hpp"
#include "klv/set/definition.hpp"

#include <cstdint>
#include <expected>
#include <span>
#include <type_traits>
#include <utility>
#include <vector>

namespace klv {

/// A field whose tag the set definition does not contain
struct unknown_field {
	uint32_t tag = 0;
	byte_vector value;

	friend auto operator==(unknown_field const&, unknown_field const&) -> bool = default;
};

namespace detail {

template<typename E>
struct field_slot {
	field<value_t<E>> value{std::unexpect, field_error::absent};

	friend auto operator==(field_slot const&, field_slot const&) -> bool = default;
};

template<typename List>
struct field_slots;

template<typename... F>
struct field_slots<field_list<F...>> : field_slot<F>... {
	friend auto operator==(field_slots const&, field_slots const&) -> bool = default;
};

}  // namespace detail

/**
 * @brief Decoded (or to be encoded) contents of one local set
 *
 * Each field of `S` has a slot of type `field<value_t<E>>`. An empty slot holds the reason: `absent` when the set did
 * not contain the field, or a decode error such as `error_indicator` or `bad_length`.
 */
template<local_set_definition S>
class record {
	static_assert(detail::list_info<typename S::fields>::unique_tags);

public:
	using definition = S;

	/// The value of field `E`, or the reason that it has no value
	template<member_of<S> E>
	[[nodiscard]]
	auto get() const noexcept -> field<value_t<E>> const& {
		return static_cast<detail::field_slot<E> const&>(slots_).value;
	}

	/// True when field `E` has a value
	template<member_of<S> E>
	[[nodiscard]]
	auto has() const noexcept -> bool {
		return get<E>().has_value();
	}

	/// Set the value of field `E`
	template<member_of<S> E>
	auto set(value_t<E> value) -> void {
		slot<E>().value = std::move(value);
	}

	/// Set the value or the status of field `E`
	template<member_of<S> E>
	auto assign(field<value_t<E>> value) -> void {
		slot<E>().value = std::move(value);
	}

	/// Mark field `E` with the error value of its format. Encoding writes that value.
	template<member_of<S> E>
	requires has_error_indicator<codec_of<E>>
	auto set_error_indicator() -> void {
		slot<E>().value = std::unexpected(field_error::error_indicator);
	}

	/// Remove field `E`
	template<member_of<S> E>
	auto erase() -> void {
		slot<E>().value = std::unexpected(field_error::absent);
	}

	/// Fields with tags that `S` does not define, in decode order. Encoding writes them after the known fields.
	[[nodiscard]]
	auto unknown() const noexcept -> std::span<unknown_field const> {
		return unknown_;
	}

	auto add_unknown(unknown_field field) -> void {
		unknown_.push_back(std::move(field));
	}

	auto clear_unknown() -> void {
		unknown_.clear();
	}

	friend auto operator==(record const&, record const&) -> bool = default;

private:
	template<typename E>
	[[nodiscard]]
	auto slot() noexcept -> detail::field_slot<E>& {
		return static_cast<detail::field_slot<E>&>(slots_);
	}

	detail::field_slots<typename S::fields> slots_;
	std::vector<unknown_field> unknown_;
};

/**
 * @brief Call `visitor(std::type_identity<E>{}, record.get<E>())` for each field `E` of `S`, in definition order
 */
template<local_set_definition S, typename F>
auto for_each_field(record<S> const& rec, F&& visitor) -> void {
	[&]<typename... E>(field_list<E...>) {
		(visitor(std::type_identity<E>{}, rec.template get<E>()), ...);
	}(typename S::fields{});
}

namespace detail {

template<typename T>
struct is_record : std::false_type {};

template<typename S>
struct is_record<record<S>> : std::true_type {};

}  // namespace detail

}  // namespace klv
