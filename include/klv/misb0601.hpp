/**
 * @file misb0601.hpp
 * @brief MISB ST 0601 - UAS Datalink Local Set
 *
 * The primary metadata set of Unmanned Aircraft Systems motion imagery.
 *
 * These tags are not defined, so they decode as unknown fields:
 * - 66 (deprecated)
 * - 73, 74, 95, and 97 to 102 (local sets and packs of other MISB standards)
 * - 143 (Metadata Substream ID Pack)
 */

#pragma once

#include "klv/misb0102.hpp"
#include "klv/misb0601/alternate_platform.hpp"
#include "klv/misb0601/enumerations.hpp"
#include "klv/misb0601/environment.hpp"
#include "klv/misb0601/image.hpp"
#include "klv/misb0601/mission.hpp"
#include "klv/misb0601/packs.hpp"
#include "klv/misb0601/platform.hpp"
#include "klv/misb0601/sensor.hpp"
#include "klv/misb0601/target.hpp"
#include "klv/misb0601/uas_datalink_ls.hpp"
