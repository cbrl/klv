/**
 * @file klv/misb0903/algorithm_ls.hpp
 * @brief MISB ST 0903 Algorithm Local Set (VMTI tag 102): an algorithm that detects or tracks targets
 */

#pragma once

#include "klv/codec.hpp"
#include "klv/set.hpp"

#include <cstdint>
#include <string_view>

namespace klv::misb0903 {

struct algorithm_id {
	static constexpr uint32_t tag = 1;
	static constexpr std::string_view name = "ID";
	static constexpr codec::integer<uint32_t> format{.length = codec::variable_length};
};

struct algorithm_name {
	static constexpr uint32_t tag = 2;
	static constexpr std::string_view name = "Name";
	static constexpr codec::string format{};
};

struct algorithm_version {
	static constexpr uint32_t tag = 3;
	static constexpr std::string_view name = "Version";
	static constexpr codec::string format{};
};

/// Kind of algorithm, for example "detector" or "classifier"
struct algorithm_class {
	static constexpr uint32_t tag = 4;
	static constexpr std::string_view name = "Class";
	static constexpr codec::string format{};
};

/// Number of frames that the algorithm uses
struct algorithm_frame_count {
	static constexpr uint32_t tag = 5;
	static constexpr std::string_view name = "Frame Count";
	static constexpr codec::integer<uint64_t> format{.length = codec::variable_length};
};

/// MISB ST 0903 Algorithm Local Set
struct algorithm_ls {
	static constexpr std::string_view name = "Algorithm Local Set";

	using fields = field_list<algorithm_id, algorithm_name, algorithm_version, algorithm_class, algorithm_frame_count>;
};

}  // namespace klv::misb0903
