/**
 * @file klv/bytes/types.hpp
 * @brief Byte views, byte containers, and the MISB time stamp types
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

/// MISB nano precision time stamp: nanoseconds since the POSIX epoch. The type ends at 2262-04-11T23:47:16Z.
using nano_timestamp = std::chrono::sys_time<std::chrono::nanoseconds>;

}  // namespace klv
