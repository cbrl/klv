/**
 * @file klv/misb0601/packs.hpp
 * @brief MISB ST 0601 packs: the value types and codecs of the fields with an inner structure
 *
 * Each codec decodes the complete value bytes of one field. Bytes after the last element of a pack are an error
 * (`field_error::invalid_value`).
 */

#pragma once

#include "klv/misb0601/packs/airbases.hpp"
#include "klv/misb0601/packs/command_record.hpp"
#include "klv/misb0601/packs/countries.hpp"
#include "klv/misb0601/packs/frame_rate.hpp"
#include "klv/misb0601/packs/horizon_pixels.hpp"
#include "klv/misb0601/packs/location.hpp"
#include "klv/misb0601/packs/payloads.hpp"
#include "klv/misb0601/packs/view_ranges.hpp"
#include "klv/misb0601/packs/wavelength_record.hpp"
#include "klv/misb0601/packs/waypoint_record.hpp"
#include "klv/misb0601/packs/weapons_store.hpp"
