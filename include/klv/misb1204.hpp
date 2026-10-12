/**
 * @file misb1204.hpp
 * @brief MISB ST 1204 - Motion Imagery Identification System (MIIS) Core Identifier
 *
 * A Core Identifier identifies the source of Motion Imagery. It is one of two kinds:
 * - A Foundational Core Identifier: a Sensor ID, a Platform ID, and a Window ID. Each ID is optional, but at least
 *   one is present.
 * - A Minor Core Identifier: one ID that a system downstream of the platform inserts.
 *
 * Each ID is a 16-byte UUID. MISB ST 0601 tag 94 and MISB ST 0903 tag 13 hold a Core Identifier. A Core Identifier
 * can also be a standalone KLV item (`core_id_item`):
 * @code
 * if (auto const id = klv::decode_packet<klv::misb1204::core_id_item>(bytes)) {
 *     use(id->value);
 * }
 * @endcode
 */

#pragma once

#include "klv/misb1204/check_value.hpp"
#include "klv/misb1204/core_id.hpp"
#include "klv/misb1204/core_id_codec.hpp"
