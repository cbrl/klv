/**
 * @file klv/misb0806/user_defined_ls.hpp
 * @brief MISB ST 0806 User Defined Local Set (RVT tag 11)
 */

#pragma once

#include "klv/codec.hpp"
#include "klv/misb0806/enumerations.hpp"
#include "klv/set.hpp"

#include <cstdint>
#include <expected>
#include <format>
#include <string>
#include <string_view>

namespace klv::misb0806 {

/// Data type and numeric ID of one user-defined data item
struct user_data_id {
	user_data_type type = user_data_type::string;
	uint8_t id = 0;  ///< 0 to 63

	friend auto operator==(user_data_id const&, user_data_id const&) -> bool = default;
};

/// One byte: the data type in bits 7 and 6, the ID in bits 5 to 0
struct user_data_id_codec {
	using value_type = user_data_id;

	[[nodiscard]]
	auto decode(byte_span in) const -> field<user_data_id> {
		if (in.size() != 1) {
			return std::unexpected(field_error::bad_length);
		}
		auto const value = std::to_integer<uint8_t>(in.front());
		return user_data_id{
		    .type = static_cast<user_data_type>(value >> 6U),
		    .id = static_cast<uint8_t>(value & 0x3FU),
		};
	}

	[[nodiscard]]
	auto encode(user_data_id const& value, byte_writer& out) const -> encode_result {
		if (value.id > 0x3F || static_cast<uint8_t>(value.type) > 3) {
			return std::unexpected(field_error::invalid_value);
		}
		out.put_u8(static_cast<uint8_t>((static_cast<uint8_t>(value.type) << 6U) | value.id));
		return {};
	}

	[[nodiscard]]
	auto to_string(user_data_id const& value) const -> std::string {
		return std::format("{{type: {}, id: {}}}", misb0806::to_string(value.type), value.id);
	}
};

struct user_data_type_id {
	static constexpr uint32_t tag = 1;
	static constexpr std::string_view name = "Numeric ID and Data Type";
	static constexpr user_data_id_codec format{};
};

struct user_data {
	static constexpr uint32_t tag = 2;
	static constexpr std::string_view name = "User Data";
	static constexpr codec::bytes format{};
};

/// MISB ST 0806 User Defined Local Set
struct user_defined_ls {
	static constexpr std::string_view name = "User Defined Local Set";

	using fields = field_list<user_data_type_id, user_data>;
};

}  // namespace klv::misb0806
