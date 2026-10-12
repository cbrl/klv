/**
 * @file klv/misb1204/core_id.hpp
 * @brief MISB ST 1204 Core Identifier value
 */

#pragma once

#include "klv/codecs/uuid.hpp"

#include <cstdint>
#include <optional>
#include <string_view>

namespace klv::misb1204 {

/// Kind of a Sensor ID or a Platform ID (ST 1204 Table 3)
enum class id_type : uint8_t {
	managed = 1,  ///< The control station generates or stores the ID
	virtual_id = 2,  ///< A host device on the platform (for example a flight computer) generates or stores the ID
	physical = 3,  ///< The device itself generates or stores the ID. The ID never changes.
};

[[nodiscard]]
constexpr auto to_string(id_type value) -> std::string_view {
	using enum id_type;
	switch (value) {
		case managed:
			return "Managed";
		case virtual_id:
			return "Virtual";
		case physical:
			return "Physical";
	}
	return {};
}

/// A Sensor ID or a Platform ID
struct device_id {
	id_type type = id_type::physical;
	klv::uuid id{};

	friend auto operator==(device_id const&, device_id const&) -> bool = default;
};

/**
 * @brief MIIS Core Identifier
 *
 * A Foundational Core Identifier has any of `sensor`, `platform`, and `window`. A Minor Core Identifier has only
 * `minor`.
 */
struct core_id {
	std::optional<device_id> sensor;
	std::optional<device_id> platform;
	std::optional<klv::uuid> window;
	std::optional<klv::uuid> minor;

	friend auto operator==(core_id const&, core_id const&) -> bool = default;
};

}  // namespace klv::misb1204
