/**
 * @file misb1507.hpp
 * @brief MISB ST 1507 - Motion Imagery Sensor Timing Metadata
 *
 * The Sensor Timing Local Set gives the exposure time of each detector of a sensor. It supports global shutter,
 * rolling shutter, and multi-Region shutter sensors. The set arrives as a stand-alone packet that ends with a
 * CRC-16-CCITT checksum.
 * - The Reference Time Pack (tag 1) is a MISB ST 1603 Nano Time Transfer Pack. Each exposure time in the set is an
 *   offset in nanoseconds from its time stamp.
 * - The Window Corners Pack (tag 2) gives the detectors that make the image.
 * - The Region List (tag 3) gives the Exposure Pattern of each Region of the Detector Group. Use `add_region` and
 *   `regions` to write and read it.
 *
 * @code
 * auto const& reference = timing.get<klv::misb1507::reference_time_pack>();
 * auto const& list = timing.get<klv::misb1507::region_list>();
 * if (reference && list) {
 *     for (auto const& region : klv::misb1507::regions(*list)) {
 *         auto const start = reference->time + region.start;  // klv::nano_timestamp
 *     }
 * }
 * @endcode
 */

#pragma once

#include "klv/misb1507/detector.hpp"
#include "klv/misb1507/fields.hpp"
#include "klv/misb1507/region.hpp"
#include "klv/misb1507/region_ls.hpp"
#include "klv/misb1507/sensor_timing_ls.hpp"
