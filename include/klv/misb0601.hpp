/**
 * @file misb0601.hpp
 * @brief MISB ST 0601 - UAS Datalink Local Set
 *
 * The primary metadata set of Unmanned Aircraft Systems motion imagery.
 *
 * The local sets of other MISB standards nest in these tags:
 * - 48: ST 0102 Security
 * - 73: ST 0806 Remote Video Terminal
 * - 74: ST 0903 Video Moving Target Indicator
 * - 95: ST 1206 SAR Motion Imagery
 * - 97: ST 1002 Range Image
 * - 98: ST 1601 Geo-Registration
 * - 99: ST 1602 Composite Imaging
 * - 100 and 101: ST 1607 Segment and Amend (child sets of ST 0601)
 *
 * Tags 66 (deprecated) and 143 (Metadata Substream ID Pack) are not defined, so they decode as unknown fields.
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
