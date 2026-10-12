/**
 * @file klv/misb1504/sdcc.hpp
 * @brief MISB ST 1504 SDCC-FLP (tag 15)
 */

#pragma once

#include "klv/bytes.hpp"
#include "klv/codec.hpp"
#include "klv/misb1504/fields.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <span>
#include <string>
#include <string_view>

namespace klv::misb1504 {

/**
 * @brief SDCC-FLP of the elements of the State Vector Pack, the State Vector TROC Pack, or both (tag 15)
 *
 * The matrix has 6 rows for each pack that it covers: one row for each element, in element order. The covered packs
 * come directly before the SDCC-FLP. `klv::sdcc_flp::members` holds the tag of the pack of each row.
 *
 * Set decoding fills `members` from the items before the SDCC-FLP. Set encoding writes the packs that `members` names
 * directly before the SDCC-FLP. Without `members`, 6 rows cover the State Vector Pack. Twelve rows cover the State
 * Vector Pack, then the State Vector TROC Pack.
 *
 * A matrix size other than 6 or 12 fails with `field_error::invalid_value`. Covered items that are not these packs
 * also fail with `field_error::invalid_value`.
 */
struct sdcc_codec {
	using value_type = klv::sdcc_flp;

	/// Decode a pack without its parent set. `members` stays empty.
	[[nodiscard]]
	auto decode(byte_span in) const -> field<klv::sdcc_flp> {
		auto value = generic.decode(in);
		if (value && !is_valid_size(value->size)) {
			return std::unexpected(field_error::invalid_value);
		}
		return value;
	}

	/// Decode a pack in its parent set. `preceding` holds the tags of the items before the pack, in decode order.
	[[nodiscard]]
	auto decode(byte_span in, std::span<uint32_t const> preceding) const -> field<klv::sdcc_flp> {
		auto value = decode(in);
		if (!value) {
			return value;
		}
		auto const count = size_t{value->size / rows_per_pack};
		if (preceding.size() < count) {
			return std::unexpected(field_error::invalid_value);
		}
		auto const packs = preceding.last(count);
		if (!is_valid_packs(packs)) {
			return std::unexpected(field_error::invalid_value);
		}
		for (auto const tag : packs) {
			value->members.insert(value->members.end(), rows_per_pack, tag);
		}
		return value;
	}

	/// Tags of the packs that set encoding writes directly before the SDCC-FLP
	[[nodiscard]]
	auto covered_tags(klv::sdcc_flp const& value) const noexcept -> std::span<uint32_t const> {
		if (!is_valid_size(value.size)) {
			return {};
		}
		if (value.members.empty()) {
			return value.size == rows_per_pack ? std::span<uint32_t const>{state_only} : both;
		}
		if (!is_valid_members(value)) {
			return {};
		}
		auto const state_first = value.members.front() == state_vector_pack::tag;
		if (value.size == rows_per_pack) {
			return state_first ? std::span<uint32_t const>{state_only} : rate_only;
		}
		return state_first ? std::span<uint32_t const>{both} : reversed;
	}

	[[nodiscard]]
	auto encode(klv::sdcc_flp const& value, byte_writer& out) const -> encode_result {
		if (!is_valid_size(value.size) || (!value.members.empty() && !is_valid_members(value))) {
			return std::unexpected(field_error::invalid_value);
		}
		return generic.encode(value, out);
	}

	[[nodiscard]]
	auto to_string(klv::sdcc_flp const& value) const -> std::string {
		return generic.to_string(value);
	}

private:
	static constexpr codec::sdcc_flp generic{};
	static constexpr auto rows_per_pack = uint32_t{6};

	static constexpr auto state_only = std::array{state_vector_pack::tag};
	static constexpr auto rate_only = std::array{state_vector_troc_pack::tag};
	static constexpr auto both = std::array{state_vector_pack::tag, state_vector_troc_pack::tag};
	static constexpr auto reversed = std::array{state_vector_troc_pack::tag, state_vector_pack::tag};

	[[nodiscard]]
	static constexpr auto is_valid_size(uint32_t size) noexcept -> bool {
		return size == rows_per_pack || size == 2 * rows_per_pack;
	}

	/// The tags name the State Vector Pack, the State Vector TROC Pack, or both, each one time
	[[nodiscard]]
	static constexpr auto is_valid_packs(std::span<uint32_t const> tags) noexcept -> bool {
		for (auto i = size_t{0}; i < tags.size(); ++i) {
			if (tags[i] != state_vector_pack::tag && tags[i] != state_vector_troc_pack::tag) {
				return false;
			}
			if (i > 0 && tags[i] == tags[0]) {
				return false;
			}
		}
		return true;
	}

	/// `members` names one pack for each group of 6 rows
	[[nodiscard]]
	static auto is_valid_members(klv::sdcc_flp const& value) noexcept -> bool {
		if (value.members.size() != value.size) {
			return false;
		}
		auto packs = std::array<uint32_t, 2>{};
		auto const count = size_t{value.size / rows_per_pack};
		for (auto i = size_t{0}; i < value.members.size(); ++i) {
			if (value.members[i] != value.members[i - i % rows_per_pack]) {
				return false;
			}
			packs[i / rows_per_pack] = value.members[i];
		}
		return is_valid_packs(std::span{packs}.first(count));
	}
};

struct sdcc_flp {
	static constexpr uint32_t tag = 15;
	static constexpr std::string_view name = "Standard Deviation and Correlation Coefficient FLP";
	static constexpr sdcc_codec format{};
};

}  // namespace klv::misb1504
