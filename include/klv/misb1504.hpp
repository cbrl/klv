/**
 * @file misb1504.hpp
 * @brief MISB ST 1504 - Natural Representation of Orbital State Vectors
 *
 * The State Vector Local Set gives the orbit of a Motion Imagery collector in Earth orbit. Six orbital elements give
 * the orbit: Keplerian elements or Non-Singular Equinoctial elements. The State Vector Status of the State Vector Pack
 * gives the kind of elements and the reference frame.
 *
 * Each pack holds real numbers of one length: 4 or 8 bytes. Encoding writes 4 bytes when binary32 holds each value
 * exactly, and 8 bytes otherwise. A stand-alone set ends with a CRC-16-CCITT (tag 3). A set inside another set has no
 * CRC.
 *
 * ST 1504 section 7.11 keeps value 0 for research in a closed system. This applies to the Reference Frame, the State
 * Vector Type, and the Anomaly Type. The codecs accept value 0, but a set with this value does not conform to ST 1504.
 *
 * The library does not check these requirements:
 * - ST 1504-08: Keplerian elements need the Anomaly Type (tag 5).
 * - ST 1504-10: the Mean Anomaly at Epoch needs the Mean Anomaly Epoch (tag 6).
 * - ST 1504-14: the SDCC-FLP must use Mode 2 parse control.
 */

#pragma once

#include "klv/misb1504/enumerations.hpp"
#include "klv/misb1504/fields.hpp"
#include "klv/misb1504/sdcc.hpp"
#include "klv/misb1504/state_vector.hpp"
#include "klv/misb1504/state_vector_ls.hpp"
