/**
 * @file misb1107.hpp
 * @brief MISB ST 1107 - Metric Geopositioning Local Set
 *
 * The set holds the photogrammetry items of MISB ST 0801 that a metric sensor needs for precise geopositioning. It
 * arrives as a stand-alone packet that ends with a CRC-16-CCITT checksum.
 *
 * MISB ST 0801 gives the IMAPB range of each item, but the producer chooses each IMAPB length at run time. These items
 * hold a `klv::imapb_value`, which keeps the length. The float items accept 2, 4, or 8 bytes.
 *
 * An ST 1010 SDCC-FLP (tag 32) gives the uncertainty of items 1 to 31 and item 47. ST 1107 Table 1 defines the IMAPB
 * range of each standard deviation (see `sigma_range`), and requires Mode 2 parse control.
 */

#pragma once

#include "klv/misb1107/enumerations.hpp"
#include "klv/misb1107/fields.hpp"
#include "klv/misb1107/metric_geopositioning_ls.hpp"
#include "klv/misb1107/sdcc.hpp"
