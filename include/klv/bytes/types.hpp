/**
 * @file klv/bytes/types.hpp
 * @brief Byte views, byte containers, and the MISB time stamp type
 */

#pragma once

#include <chrono>
#include <cstddef>
#include <span>
#include <vector>

namespace klv {

/// Immutable view over raw bytes
using byte_span = std::span<std::byte const>;

/// Owning byte container
using byte_vector = std::vector<std::byte>;

/// MISB precision time stamp: microseconds since the POSIX epoch
using timestamp = std::chrono::sys_time<std::chrono::microseconds>;

}  // namespace klv
