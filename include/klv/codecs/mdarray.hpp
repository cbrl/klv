/**
 * @file klv/codecs/mdarray.hpp
 * @brief MISB ST 1303 multi-dimensional array pack (MDARRAY)
 */

#pragma once

#include "klv/bytes/big_endian.hpp"
#include "klv/bytes/byte_reader.hpp"
#include "klv/bytes/byte_writer.hpp"
#include "klv/bytes/types.hpp"
#include "klv/codecs/enumeration.hpp"
#include "klv/codecs/imapb.hpp"
#include "klv/codecs/pack.hpp"
#include "klv/codecs/text.hpp"
#include "klv/field.hpp"

#include <algorithm>
#include <bit>
#include <cmath>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <format>
#include <limits>
#include <map>
#include <optional>
#include <ranges>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

namespace klv {

/// MISB ST 1303 array processing algorithm (APA): how an MDARRAY encodes its elements
enum class array_algorithm : uint8_t {
	natural = 1,  ///< Each element in its natural form of E bytes: IEEE 754, an integer, or a Boolean value
	imapb = 2,  ///< Each element as an IMAPB value of E bytes. The pack holds the IMAPB range.
	boolean = 3,  ///< One bit for each element, most significant bit first
	unsigned_integer = 4,  ///< Each element as a BER-OID offset from a bias value
	run_length = 5,  ///< Rectangles of equal elements on a background of one element value
};

[[nodiscard]]
constexpr auto to_string(array_algorithm value) -> std::string_view {
	using enum array_algorithm;
	switch (value) {
		case natural:
			return "Natural";
		case imapb:
			return "IMAPB";
		case boolean:
			return "Boolean";
		case unsigned_integer:
			return "Unsigned Integer";
		case run_length:
			return "Run-Length";
	}
	return {};
}

/// A multi-dimensional array and the parameters of its encoding (MISB ST 1303 MDARRAY)
template<typename T>
struct mdarray {
	std::vector<uint32_t> sizes;  ///< Size of each dimension, outermost first
	std::vector<T> elements;  ///< Elements in row-major order: the last dimension changes fastest
	size_t element_size = sizeof(T);  ///< E: bytes for each element. The Boolean algorithm does not use it.
	array_algorithm algorithm = array_algorithm::natural;
	double imapb_min = 0.0;  ///< `array_algorithm::imapb`: lower limit of the range
	double imapb_max = 0.0;  ///< `array_algorithm::imapb`: upper limit of the range
	size_t imapb_limit_length = 4;  ///< `array_algorithm::imapb`: bytes of each IEEE 754 limit (4 or 8)

	// Exact comparison. std::tie compares in the standard library, which keeps -Wfloat-equal quiet.
	friend auto operator==(mdarray const& lhs, mdarray const& rhs) -> bool {
		auto const members = [](mdarray const& value) {
			return std::tie(
			    value.sizes,
			    value.elements,
			    value.element_size,
			    value.algorithm,
			    value.imapb_min,
			    value.imapb_max,
			    value.imapb_limit_length
			);
		};
		return members(lhs) == members(rhs);
	}
};

}  // namespace klv

namespace klv::codec {

/// Element types of `codec::mdarray`
template<typename T>
concept mdarray_element =
    std::same_as<T, double> || std::same_as<T, uint64_t> || std::same_as<T, int64_t> || std::same_as<T, bool>;

/**
 * @brief MISB ST 1303 MDARRAY of real values, unsigned or signed integers, or Boolean values
 *
 * The supported array processing algorithms are:
 * - `double`: `natural` (IEEE 754, E = 4 or 8), `imapb` (E = 1 to 8), and `run_length` (natural elements)
 * - `uint64_t`: `natural` (E = 1 to 8), `unsigned_integer`, and `run_length` (natural elements)
 * - `int64_t`: `natural` (two's complement, E = 1 to 8) and `run_length` (natural elements)
 * - `bool`: `boolean`, `natural` (E = 1 to 8, not zero is true), and `run_length` (natural elements)
 *
 * Decoding fails with `field_error::invalid_value` for another algorithm, a malformed pack, or an array with more than
 * `max_elements` elements. An IMAPB special value (for example "below minimum") decodes as NaN. Encoding writes NaN
 * as the IMAPB NaN value.
 *
 * Run-length rectangles can overlap. A later rectangle overwrites the elements of an earlier one. Decoding fails when
 * the rectangles together hold more than `max_patch_area` elements.
 *
 * A run-length encoding uses the most common element as the background. On a tie, the element that comes first
 * wins. Each rectangle starts at the first element that the background and the earlier rectangles do not cover. It
 * grows along the last dimension, then along each outer dimension, while the new elements are equal to its element.
 */
template<mdarray_element T>
struct mdarray {
	using value_type = klv::mdarray<T>;

	/// Largest element count that decoding accepts
	static constexpr auto max_elements = size_t{1} << 22U;

	/// Largest sum of the run-length rectangle areas that decoding accepts. It limits the work for each array.
	static constexpr auto max_patch_area = 4 * max_elements;

	[[nodiscard]]
	auto decode(byte_span in) const -> field<value_type> {
		auto reader = byte_reader{in};
		auto result = value_type{};

		auto const dimensions = detail::read_oid(reader);
		if (!dimensions || *dimensions == 0) {
			return std::unexpected(field_error::invalid_value);
		}

		auto count = size_t{1};
		for (auto i = uint32_t{0}; i < *dimensions; ++i) {
			auto const size = detail::read_oid(reader);
			if (!size || *size == 0 || *size > max_elements / count) {
				return std::unexpected(field_error::invalid_value);
			}
			count *= *size;
			result.sizes.push_back(*size);
		}

		auto const element_size = detail::read_oid(reader);
		auto const algorithm = reader.read_u8();
		if (!element_size || !algorithm) {
			return std::unexpected(field_error::invalid_value);
		}
		result.element_size = *element_size;
		result.algorithm = static_cast<array_algorithm>(*algorithm);

		auto status = field<void>{};
		switch (result.algorithm) {
			case array_algorithm::natural:
				status = decode_natural(reader, count, result);
				break;
			case array_algorithm::imapb:
				status = decode_imapb(reader, count, result);
				break;
			case array_algorithm::boolean:
				status = decode_boolean(reader, count, result);
				break;
			case array_algorithm::unsigned_integer:
				status = decode_unsigned(reader, count, result);
				break;
			case array_algorithm::run_length:
				status = decode_run_length(reader, count, result);
				break;
			default:
				status = std::unexpected(field_error::invalid_value);
				break;
		}
		if (!status) {
			return std::unexpected(status.error());
		}
		return detail::finish(reader, std::move(result));
	}

	[[nodiscard]]
	auto encode(value_type const& value, byte_writer& out) const -> encode_result {
		auto count = size_t{1};
		for (auto const size : value.sizes) {
			count *= size;
		}
		if (value.sizes.empty() || std::ranges::contains(value.sizes, uint32_t{0}) || count != value.elements.size()
		    || value.element_size > std::numeric_limits<uint32_t>::max()) {
			return std::unexpected(field_error::invalid_value);
		}

		out.put_ber_oid(value.sizes.size());
		for (auto const size : value.sizes) {
			out.put_ber_oid(size);
		}
		out.put_ber_oid(value.element_size);
		out.put_u8(static_cast<uint8_t>(value.algorithm));

		switch (value.algorithm) {
			case array_algorithm::natural:
				return encode_natural(value, out);
			case array_algorithm::imapb:
				return encode_imapb(value, out);
			case array_algorithm::boolean:
				return encode_boolean(value, out);
			case array_algorithm::unsigned_integer:
				return encode_unsigned(value, out);
			case array_algorithm::run_length:
				return encode_run_length(value, out);
			default:
				return std::unexpected(field_error::invalid_value);
		}
	}

	/// Text with the sizes, the algorithm, and up to 16 elements
	[[nodiscard]]
	auto to_string(value_type const& value) const -> std::string {
		constexpr auto shown = size_t{16};

		auto elements = detail::list_text(value.elements | std::views::take(static_cast<std::ptrdiff_t>(shown)));
		if (value.elements.size() > shown) {
			elements.insert(elements.size() - 1, std::format(", ... ({} elements)", value.elements.size()));
		}
		return std::format(
		    "{{sizes: {}, algorithm: {}, elements: {}}}",
		    detail::list_text(value.sizes),
		    detail::enum_text(value.algorithm),
		    elements
		);
	}

private:
	/// Check that `size` bytes can hold one element in its natural form
	[[nodiscard]]
	static constexpr auto is_natural_size(size_t size) noexcept -> bool {
		if constexpr (std::same_as<T, double>) {
			return size == sizeof(float) || size == sizeof(double);
		}
		else {
			return size >= 1 && size <= sizeof(uint64_t);
		}
	}

	[[nodiscard]]
	static auto read_natural(byte_span bytes) -> T {
		if constexpr (std::same_as<T, double>) {
			return detail::read_ieee(bytes).value_or(std::numeric_limits<double>::quiet_NaN());
		}
		else if constexpr (std::same_as<T, bool>) {
			return std::ranges::any_of(bytes, [](std::byte byte) { return byte != std::byte{0}; });
		}
		else {
			// A signed type is sign-extended from the first byte (two's complement)
			return read_big_endian<T>(bytes);
		}
	}

	[[nodiscard]]
	static auto put_natural(T element, size_t size, byte_writer& out) -> encode_result {
		if constexpr (std::same_as<T, double>) {
			return detail::put_ieee(element, size, out);
		}
		else if constexpr (std::same_as<T, bool>) {
			out.put_big_endian(static_cast<uint8_t>(element ? 1U : 0U), size);
			return {};
		}
		else {
			if constexpr (std::same_as<T, int64_t>) {
				// E bytes hold -2^(8E-1) to 2^(8E-1) - 1
				auto const limit = size < sizeof(int64_t) ? int64_t{1} << (size * 8 - 1) : int64_t{0};
				if (limit != 0 && (element < -limit || element >= limit)) {
					return std::unexpected(field_error::out_of_range);
				}
			}
			else if (size < sizeof(uint64_t) && (element >> (size * 8)) != 0) {
				return std::unexpected(field_error::out_of_range);
			}
			out.put_big_endian(element, size);
			return {};
		}
	}

	/// The IMAPB format of an array, or no value when its parameters are not valid
	[[nodiscard]]
	static auto imapb_format(double min, double max, size_t element_size) -> std::optional<imapb> {
		if (!std::isfinite(min) || !std::isfinite(max) || !std::isfinite(max - min) || !(min < max)
		    || element_size < 1 || element_size > imapb::max_length) {
			return std::nullopt;
		}
		return imapb{min, max, element_size};
	}

	static auto decode_natural(byte_reader& reader, size_t count, value_type& result) -> field<void> {
		if (!is_natural_size(result.element_size) || reader.remaining() != count * result.element_size) {
			return std::unexpected(field_error::invalid_value);
		}
		result.elements.reserve(count);
		for (auto i = size_t{0}; i < count; ++i) {
			result.elements.push_back(read_natural(*reader.take(result.element_size)));
		}
		return {};
	}

	static auto decode_imapb(byte_reader& reader, size_t count, value_type& result) -> field<void> {
		if constexpr (std::same_as<T, double>) {
			auto const size = result.element_size;
			if (size < 1 || size > imapb::max_length || reader.remaining() < count * size) {
				return std::unexpected(field_error::invalid_value);
			}

			// The two range limits use the bytes before the elements
			auto const limits = reader.remaining() - count * size;
			if (limits != 2 * sizeof(float) && limits != 2 * sizeof(double)) {
				return std::unexpected(field_error::invalid_value);
			}
			result.imapb_limit_length = limits / 2;
			result.imapb_min = *detail::read_ieee(*reader.take(limits / 2));
			result.imapb_max = *detail::read_ieee(*reader.take(limits / 2));

			auto const format = imapb_format(result.imapb_min, result.imapb_max, size);
			if (!format) {
				return std::unexpected(field_error::invalid_value);
			}
			result.elements.reserve(count);
			for (auto i = size_t{0}; i < count; ++i) {
				auto const element = format->decode(*reader.take(size));
				result.elements.push_back(element.value_or(std::numeric_limits<double>::quiet_NaN()));
			}
			return {};
		}
		else {
			return std::unexpected(field_error::invalid_value);
		}
	}

	/// One bit for each element, most significant bit first. The bits after the last element are not used.
	static auto decode_boolean(byte_reader& reader, size_t count, value_type& result) -> field<void> {
		if constexpr (std::same_as<T, bool>) {
			auto const bytes = reader.take((count + 7) / 8);
			if (!bytes) {
				return std::unexpected(field_error::invalid_value);
			}
			result.elements.reserve(count);
			for (auto i = size_t{0}; i < count; ++i) {
				auto const byte = std::to_integer<unsigned>((*bytes)[i / 8]);
				result.elements.push_back(((byte >> (7 - i % 8)) & 1U) != 0);
			}
			return {};
		}
		else {
			return std::unexpected(field_error::invalid_value);
		}
	}

	static auto decode_unsigned(byte_reader& reader, size_t count, value_type& result) -> field<void> {
		if constexpr (std::same_as<T, uint64_t>) {
			auto const bias = detail::read_oid<uint64_t>(reader);
			if (!bias || count > reader.remaining()) {
				return std::unexpected(field_error::invalid_value);
			}
			result.elements.reserve(count);
			for (auto i = size_t{0}; i < count; ++i) {
				// An element must fit in 64 bits
				auto const offset = detail::read_oid<uint64_t>(reader);
				if (!offset || *offset > std::numeric_limits<uint64_t>::max() - *bias) {
					return std::unexpected(field_error::invalid_value);
				}
				result.elements.push_back(*bias + *offset);
			}
			return {};
		}
		else {
			return std::unexpected(field_error::invalid_value);
		}
	}

	static auto decode_run_length(byte_reader& reader, size_t count, value_type& result) -> field<void> {
		auto const size = result.element_size;
		if (!is_natural_size(size) || reader.remaining() < size) {
			return std::unexpected(field_error::invalid_value);
		}
		result.elements.assign(count, read_natural(*reader.take(size)));

		auto const& sizes = result.sizes;
		auto const strides = strides_of(sizes);
		auto filled = size_t{0};

		while (!reader.empty()) {
			auto const bytes = reader.take(size);
			if (!bytes) {
				return std::unexpected(field_error::invalid_value);
			}
			auto const element = read_natural(*bytes);

			auto start = std::vector<size_t>(sizes.size());
			auto extent = std::vector<size_t>(sizes.size());
			for (auto* list : {&start, &extent}) {
				for (auto& item : *list) {
					auto const value = detail::read_oid(reader);
					if (!value) {
						return std::unexpected(field_error::invalid_value);
					}
					item = *value;
				}
			}

			// Each rectangle must be inside the array. Rectangles can overlap, and the last one wins.
			auto area = size_t{1};
			for (auto k = size_t{0}; k < sizes.size(); ++k) {
				if (extent[k] == 0 || start[k] >= sizes[k] || extent[k] > sizes[k] - start[k]) {
					return std::unexpected(field_error::invalid_value);
				}
				area *= extent[k];
			}
			filled += area;
			if (filled > max_patch_area) {
				return std::unexpected(field_error::invalid_value);
			}

			visit_rectangle(strides, start, extent, [&](size_t index) {
				result.elements[index] = element;
				return true;
			});
		}
		return {};
	}

	static auto encode_natural(value_type const& value, byte_writer& out) -> encode_result {
		if (!is_natural_size(value.element_size)) {
			return std::unexpected(field_error::invalid_value);
		}
		for (auto const element : value.elements) {
			if (auto status = put_natural(element, value.element_size, out); !status) {
				return status;
			}
		}
		return {};
	}

	static auto encode_imapb(value_type const& value, byte_writer& out) -> encode_result {
		if constexpr (std::same_as<T, double>) {
			auto const format = imapb_format(value.imapb_min, value.imapb_max, value.element_size);
			if (!format) {
				return std::unexpected(field_error::invalid_value);
			}
			auto status = detail::put_ieee(value.imapb_min, value.imapb_limit_length, out).and_then([&] {
				return detail::put_ieee(value.imapb_max, value.imapb_limit_length, out);
			});
			for (auto i = size_t{0}; status && i < value.elements.size(); ++i) {
				status = format->encode(value.elements[i], out);
			}
			return status;
		}
		else {
			return std::unexpected(field_error::invalid_value);
		}
	}

	/// Write one bit for each element, most significant bit first. Zero bits fill the last byte.
	static auto encode_boolean(value_type const& value, byte_writer& out) -> encode_result {
		if constexpr (std::same_as<T, bool>) {
			auto byte = uint8_t{0};
			for (auto i = size_t{0}; i < value.elements.size(); ++i) {
				if (value.elements[i]) {
					byte = static_cast<uint8_t>(byte | (0x80U >> (i % 8)));
				}
				if (i % 8 == 7 || i + 1 == value.elements.size()) {
					out.put_u8(byte);
					byte = 0;
				}
			}
			return {};
		}
		else {
			return std::unexpected(field_error::invalid_value);
		}
	}

	static auto encode_unsigned(value_type const& value, byte_writer& out) -> encode_result {
		if constexpr (std::same_as<T, uint64_t>) {
			auto const bias = std::ranges::min(value.elements);
			out.put_ber_oid(bias);
			for (auto const element : value.elements) {
				out.put_ber_oid(element - bias);
			}
			return {};
		}
		else {
			return std::unexpected(field_error::invalid_value);
		}
	}

	/// Write the background element, then one rectangle for each block of equal elements that it does not cover
	static auto encode_run_length(value_type const& value, byte_writer& out) -> encode_result {
		if (!is_natural_size(value.element_size)) {
			return std::unexpected(field_error::invalid_value);
		}

		auto const background = most_common(value.elements);
		auto status = put_natural(background, value.element_size, out);

		auto const& sizes = value.sizes;
		auto const dimensions = sizes.size();
		auto const strides = strides_of(sizes);
		auto covered = std::vector<bool>(value.elements.size());

		// The elements of a rectangle that the background and the earlier rectangles do not cover, all equal
		auto const fits = [&](std::vector<size_t> const& start, std::vector<size_t> const& extent, T element) {
			return visit_rectangle(strides, start, extent, [&](size_t index) {
				return !covered[index] && same_bits(value.elements[index], element);
			});
		};

		for (auto first = size_t{0}; status && first < value.elements.size(); ++first) {
			auto const element = value.elements[first];
			if (covered[first] || same_bits(element, background)) {
				continue;
			}

			auto start = std::vector<size_t>(dimensions);
			auto index = first;
			for (auto k = size_t{0}; k < dimensions; ++k) {
				start[k] = index / strides[k];
				index %= strides[k];
			}

			// Grow along the last dimension first. Each step adds one slice of the rectangle.
			auto extent = std::vector<size_t>(dimensions, 1);
			for (auto k = dimensions; k > 0; --k) {
				auto const dimension = k - 1;
				auto slice_start = start;
				auto slice_extent = extent;
				slice_extent[dimension] = 1;
				for (;;) {
					slice_start[dimension] = start[dimension] + extent[dimension];
					if (slice_start[dimension] >= sizes[dimension] || !fits(slice_start, slice_extent, element)) {
						break;
					}
					++extent[dimension];
				}
			}

			visit_rectangle(strides, start, extent, [&](size_t covered_index) {
				covered[covered_index] = true;
				return true;
			});

			status = put_natural(element, value.element_size, out);
			for (auto const* list : {&start, &extent}) {
				for (auto const item : *list) {
					out.put_ber_oid(item);
				}
			}
		}
		return status;
	}

	/**
	 * @brief Call `visit(index)` for each element of a rectangle, in row-major order
	 *
	 * The rectangle starts at `start` and has `extent` elements in each dimension. The function stops when `visit`
	 * returns false. It returns false in that case, and true otherwise.
	 */
	template<typename F>
	static auto visit_rectangle(
	    std::vector<size_t> const& strides,
	    std::vector<size_t> const& start,
	    std::vector<size_t> const& extent,
	    F&& visit
	) -> bool {
		auto offset = std::vector<size_t>(start.size(), 0);
		for (;;) {
			auto index = size_t{0};
			for (auto k = size_t{0}; k < start.size(); ++k) {
				index += (start[k] + offset[k]) * strides[k];
			}
			if (!visit(index)) {
				return false;
			}

			// Step to the next element. The last dimension changes fastest.
			auto k = start.size();
			for (; k > 0; --k) {
				if (++offset[k - 1] < extent[k - 1]) {
					break;
				}
				offset[k - 1] = 0;
			}
			if (k == 0) {
				return true;
			}
		}
	}

	/// Distance between two neighbors along each dimension, in elements
	[[nodiscard]]
	static auto strides_of(std::vector<uint32_t> const& sizes) -> std::vector<size_t> {
		auto strides = std::vector<size_t>(sizes.size(), 1);
		for (auto k = sizes.size(); k > 1; --k) {
			strides[k - 2] = strides[k - 1] * sizes[k - 1];
		}
		return strides;
	}

	/// Bit pattern of an element. Two NaN values with the same bits are equal.
	[[nodiscard]]
	static auto bits_of(T element) noexcept -> uint64_t {
		if constexpr (std::same_as<T, bool>) {
			return element ? 1U : 0U;
		}
		else {
			return std::bit_cast<uint64_t>(element);
		}
	}

	[[nodiscard]]
	static auto same_bits(T lhs, T rhs) noexcept -> bool {
		return bits_of(lhs) == bits_of(rhs);
	}

	/// The most common element. On a tie, the element that comes first.
	[[nodiscard]]
	static auto most_common(std::vector<T> const& elements) -> T {
		auto counts = std::map<uint64_t, size_t>{};
		for (auto const element : elements) {
			++counts[bits_of(element)];
		}
		auto best = elements.front();
		auto best_count = size_t{0};
		for (auto const element : elements) {
			if (auto const count = counts[bits_of(element)]; count > best_count) {
				best = element;
				best_count = count;
			}
		}
		return best;
	}
};

}  // namespace klv::codec
