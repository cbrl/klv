/**
 * @file klv/set/record.hpp
 * @brief Typed records that hold the field values of one set
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

/// An item whose id the set definition does not contain. `Id` is `uint32_t` (a tag) or `universal_key`.
template<typename Id>
struct unknown_field {
	Id id{};
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

/**
 * @brief Slot access for the set algorithms, without the member check of `record::get`
 *
 * The algorithms visit only the fields of the set. They cast the slots to `field_slot<E>`. A debug build keeps each
 * small template function as a function, and a cast adds no function for each field.
 */
struct record_access {
	template<typename R>
	[[nodiscard]]
	static auto slots(R& rec) noexcept -> auto& {
		return rec.slots_;
	}
};

}  // namespace detail

/**
 * @brief Decoded (or to be encoded) contents of one set
 *
 * Each field of `S` has a slot of type `field<value_t<E>>`. A slot without a value holds the reason:
 * - `absent`: the set did not contain the field.
 * - `empty`: the field had a length of zero.
 * - a decode error, such as `error_indicator` or `bad_length`.
 */
template<set_definition S>
class record {
	static_assert(detail::set_info<S>::unique_ids);

public:
	using definition = S;
	using unknown_type = unknown_field<item_id_t<S>>;

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

	/// Append one item to the repeated field `E`. A field without a value starts a new list.
	template<member_of<S> E>
	requires repeated_codec<codec_of<E>>
	auto add(typename codec_of<E>::item_type item) -> void {
		auto& value = slot<E>().value;
		if (!value) {
			value.emplace();
		}
		value->push_back(std::move(item));
	}

	/// Mark field `E` with the error value of its format. Encoding writes that value.
	template<member_of<S> E>
	requires has_error_indicator<codec_of<E>>
	auto set_error_indicator() -> void {
		slot<E>().value = std::unexpected(field_error::error_indicator);
	}

	/// Mark field `E` as empty. Encoding writes it with a length of zero. In a MISB ST 1607 child set, an empty field
	/// clears the parent value.
	template<member_of<S> E>
	auto set_empty() -> void {
		slot<E>().value = std::unexpected(field_error::empty);
	}

	/// Remove field `E`
	template<member_of<S> E>
	auto erase() -> void {
		slot<E>().value = std::unexpected(field_error::absent);
	}

	/// Items with ids that `S` does not define, in decode order. Encoding writes them after the known fields.
	[[nodiscard]]
	auto unknown() const noexcept -> std::span<unknown_type const> {
		return unknown_;
	}

	auto add_unknown(unknown_type field) -> void {
		unknown_.push_back(std::move(field));
	}

	auto clear_unknown() -> void {
		unknown_.clear();
	}

	friend auto operator==(record const&, record const&) -> bool = default;

private:
	friend struct detail::record_access;

	template<typename E>
	[[nodiscard]]
	auto slot() noexcept -> detail::field_slot<E>& {
		return static_cast<detail::field_slot<E>&>(slots_);
	}

	detail::field_slots<typename S::fields> slots_;
	std::vector<unknown_type> unknown_;
};

/**
 * @brief Call `visitor(std::type_identity<E>{}, record.get<E>())` for each field `E` of `S`, in definition order
 */
template<set_definition S, typename F>
auto for_each_field(record<S> const& rec, F&& visitor) -> void {
	[&]<typename... E>(field_list<E...>) {
		auto const& slots = detail::record_access::slots(rec);
		(visitor(std::type_identity<E>{}, static_cast<detail::field_slot<E> const&>(slots).value), ...);
	}(typename S::fields{});
}

/**
 * @brief Copy each field of `from` that is not absent into `to`
 *
 * A value replaces the value in `to`. An empty field makes the field in `to` empty. The function ignores a field that
 * the set `To` does not define, and the unknown items of `from`.
 */
template<set_definition From, set_definition To>
auto copy_fields(record<From> const& from, record<To>& to) -> void {
	for_each_field(from, [&]<typename E>(std::type_identity<E>, field<value_t<E>> const& value) {
		if constexpr (member_of<E, To>) {
			if (value || value.error() != field_error::absent) {
				to.template assign<E>(value);
			}
		}
	});
}

namespace detail {

template<typename T>
struct is_record : std::false_type {};

template<typename S>
struct is_record<record<S>> : std::true_type {};

template<typename T>
struct is_record_list : std::false_type {};

template<typename S>
struct is_record_list<std::vector<record<S>>> : std::true_type {};

}  // namespace detail

}  // namespace klv
