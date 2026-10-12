/**
 * @file misb0902.hpp
 * @brief MISB ST 0902 - Motion Imagery Sensor Minimum Metadata Set (MISMMS)
 *
 * The MISMMS is a profile of the MISB ST 0601 UAS Datalink Local Set. Table 1 lists the items that a stream must
 * report. The stream reports each item at least once every 30 seconds (requirement ST 0902.8-05). Thus, one packet
 * does not have to hold all of the items.
 * - `check` gives the requirements that one record does not meet.
 * - `report_monitor` checks the 30-second rule over a stream of records.
 *
 * Rules of the check:
 * - An empty item (length of zero) does not meet a requirement.
 * - Items 48/4, 48/5, and 48/6 are conditional in MISB ST 0102. The check does not require them.
 * - The packet functions verify and write the Checksum (tag 1). The check does not require it.
 * - Tag 75 and tag 104 exclude each other. A record with both items gives a `conflict` finding.
 * - The check reads the items of the top-level set. To check one segment of MISB ST 1607, merge the segment first.
 *
 * @code
 * for (auto const& finding : klv::misb0902::check(uas)) {
 *     // to_string(finding.item), to_string(finding.reason)
 * }
 * @endcode
 */

#pragma once

#include "klv/misb0902/check.hpp"
#include "klv/misb0902/items.hpp"
#include "klv/misb0902/report_monitor.hpp"
