#pragma once

#include <klv/bytes.hpp>

#include <cstddef>
#include <fstream>
#include <iterator>
#include <optional>
#include <vector>

/// Read a whole file into memory, or nothing when the file cannot be opened
inline auto read_file(char const* path) -> std::optional<klv::byte_vector> {
	auto file = std::ifstream(path, std::ios::binary);
	if (!file) {
		return std::nullopt;
	}

	auto const chars = std::vector<char>(std::istreambuf_iterator<char>{file}, std::istreambuf_iterator<char>{});
	auto bytes = klv::byte_vector(chars.size());
	for (auto i = size_t{0}; i < chars.size(); ++i) {
		bytes[i] = static_cast<std::byte>(chars[i]);
	}
	return bytes;
}
