/**
 * @file misb1603.hpp
 * @brief MISB ST 1603 - Time Transfer Metadata
 *
 * Time Transfer sends accurate time from a reference clock to a receptor clock. For example, a GPS receiver sends time
 * to a camera. ST 1603 defines two constructs:
 * - The Time Transfer Local Set gives the state of the receptor clock. It applies to the time stamp of the set or the
 *   pack that holds it: the parent time.
 * - The Nano Time Transfer Pack holds a Nano Precision Time Stamp (MISB ST 0603) and a Time Transfer Local Set.
 *
 * The Time Transfer Local Set is always inside another set or pack, so it has no checksum. Some items use the units
 * of the parent time. These are microseconds for a Precision Time Stamp, and nanoseconds in a Nano Time Transfer Pack.
 *
 * A Nano Time Transfer Pack can be a standalone KLV item (`nano_time_transfer_item`):
 * @code
 * if (auto const pack = klv::decode_packet<klv::misb1603::nano_time_transfer_item>(bytes)) {
 *     use(pack->value.time, pack->value.transfer);
 * }
 * @endcode
 */

#pragma once

#include "klv/misb1603/enumerations.hpp"
#include "klv/misb1603/fields.hpp"
#include "klv/misb1603/nano_time_transfer.hpp"
#include "klv/misb1603/time_transfer_ls.hpp"
