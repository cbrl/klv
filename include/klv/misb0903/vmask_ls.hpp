/**
 * @file klv/misb0903/vmask_ls.hpp
 * @brief MISB ST 0903 VMask Local Set (VTarget tag 101): the outline of a target
 */

#pragma once

#include "klv/bytes.hpp"
#include "klv/codec.hpp"
#include "klv/codecs/pack.hpp"
#include "klv/set.hpp"

#include <cstddef>
#include <cstdint>
#include <expected>
#include <format>
#include <string>
#include <string_view>

namespace klv::misb0903 {

/// Run of target pixels from left to right
struct pixel_run {
	uint64_t index = 0;  ///< First pixel: (row - 1) * frame width + column, with row and column from 1
	uint64_t length = 0;  ///< Number of pixels

	friend auto operator==(pixel_run const&, pixel_run const&) -> bool = default;
};

/// Pixel run pack: the index length (BER length), the pixel index, then the run length (BER length)
struct pixel_run_codec {
	using value_type = pixel_run;

	[[nodiscard]]
	auto decode(byte_span in) const -> field<pixel_run> {
		auto reader = byte_reader{in};
		auto const index = klv::detail::read_prefixed(reader);
		if (!index) {
			return std::unexpected(index.error());
		}
		if (index->empty() || index->size() > sizeof(uint64_t)) {
			return std::unexpected(field_error::bad_length);
		}

		auto const length = reader.read_ber_length();
		if (!length) {
			return std::unexpected(field_error::invalid_value);
		}
		return klv::detail::finish(reader, pixel_run{.index = read_big_endian<uint64_t>(*index), .length = *length});
	}

	[[nodiscard]]
	auto encode(pixel_run const& value, byte_writer& out) const -> encode_result {
		auto const index_length = codec::integer<uint64_t>{.length = codec::variable_length};
		auto status = out.put_length_prefixed([&] { return index_length.encode(value.index, out); });
		out.put_ber_length(value.length);
		return status;
	}

	[[nodiscard]]
	auto to_string(pixel_run const& value) const -> std::string {
		return std::format("{{index: {}, length: {}}}", value.index, value.length);
	}
};

/// Pixel indexes of the polygon vertices around the target, clockwise
struct pixel_contour {
	static constexpr uint32_t tag = 1;
	static constexpr std::string_view name = "Pixel Contour";
	static constexpr codec::series<codec::integer<uint64_t>> format{{.length = codec::variable_length}};
};

/// Runs of target pixels
struct bitmask_series {
	static constexpr uint32_t tag = 2;
	static constexpr std::string_view name = "Bitmask Series";
	static constexpr codec::series<pixel_run_codec> format{};
};

/// MISB ST 0903 VMask Local Set
struct vmask_ls {
	static constexpr std::string_view name = "VMask Local Set";

	using fields = field_list<pixel_contour, bitmask_series>;
};

}  // namespace klv::misb0903
