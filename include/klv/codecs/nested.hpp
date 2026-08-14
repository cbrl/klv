/**
 * @file klv/codecs/nested.hpp
 * @brief Codec for a local set inside a field of another set
 */

#pragma once

#include "klv/bytes/byte_writer.hpp"
#include "klv/bytes/types.hpp"
#include "klv/field.hpp"
#include "klv/set/decode.hpp"
#include "klv/set/definition.hpp"
#include "klv/set/describe.hpp"
#include "klv/set/encode.hpp"
#include "klv/set/record.hpp"

#include <cstddef>
#include <expected>
#include <format>
#include <string>
#include <utility>

namespace klv {

/// Largest number of nested sets that a set can hold inside each other. A set at a deeper level does not decode.
inline constexpr auto max_nesting_depth = size_t{16};

namespace detail {

/// Number of nested sets that this thread decodes now, inside each other
inline thread_local auto nesting_depth = size_t{0};

/// Counts one nesting level while it exists. A set can hold its own records (MISB ST 1607). Thus, only the input
/// limits the depth.
class nesting_guard {
public:
	nesting_guard() noexcept : allowed_{nesting_depth < max_nesting_depth} {
		++nesting_depth;
	}

	nesting_guard(nesting_guard const&) = delete;
	auto operator=(nesting_guard const&) -> nesting_guard& = delete;

	~nesting_guard() {
		--nesting_depth;
	}

	[[nodiscard]]
	auto allowed() const noexcept -> bool {
		return allowed_;
	}

private:
	bool allowed_;
};

}  // namespace detail

}  // namespace klv

namespace klv::codec {

/**
 * @brief A local set inside a field of another set (for example MISB ST 0102 in ST 0601 tag 48)
 */
template<local_set_definition S>
struct nested {
	using value_type = record<S>;

	/// Decode the set. A set deeper than `max_nesting_depth` gives `invalid_value`.
	[[nodiscard]]
	auto decode(byte_span in) const -> field<record<S>> {
		auto const guard = detail::nesting_guard{};
		if (!guard.allowed()) {
			return std::unexpected(field_error::invalid_value);
		}

		auto rec = decode_set<S>(in);
		if (!rec) {
			return std::unexpected(field_error::invalid_value);
		}
		return std::move(*rec);
	}

	[[nodiscard]]
	auto encode(record<S> const& value, byte_writer& out) const -> encode_result {
		if (!detail::encode_fields(value, out)) {
			return std::unexpected(field_error::invalid_value);
		}
		return {};
	}

	[[nodiscard]]
	auto to_string(record<S> const& value) const -> std::string {
		auto text = std::string{};
		for (auto const& entry : describe(value)) {
			if (!text.empty()) {
				text += "; ";
			}
			text += std::format(
			    "{}: {}",
			    entry.name,
			    entry.value ? *entry.value : std::string{klv::to_string(entry.value.error())}
			);
		}
		return text;
	}
};

}  // namespace klv::codec
