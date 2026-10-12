/**
 * @file misb1403.hpp
 * @brief MISB ST 1403 - SAR Motion Imagery (SARMI) Threshold Metadata Sets
 *
 * MISB ST 1403 is a profile of the MISB ST 0601 UAS Datalink Local Set for SARMI streams. Each image frame of a
 * stream must report all of the items (requirement ST 1403-05). The MISB ST 1206 SAR Motion Imagery Local Set in tag
 * 95 holds the SAR items. The items depend on the product of the stream:
 * - SAR imagery: the items of Table 1.
 * - SAR coherent change products: the items of Table 1 and Table 2.
 *
 * `check` gives the requirements that one record does not meet. Rules of the check:
 * - An empty item (length of zero) does not meet a requirement (requirement ST 1403.2-07).
 * - Items 48/4, 48/5, and 48/6 are conditional in MISB ST 0102. The check does not require them.
 * - The packet functions verify and write the Checksum (tag 1). The check does not require it.
 * - Tag 75 and tag 104 exclude each other. A record with both items gives a `conflict` finding.
 * - The check reads the items of the top-level set. To check one segment of MISB ST 1607, merge the segment first.
 *
 * @code
 * for (auto const& finding : klv::misb1403::check(uas, klv::misb1403::product::imagery)) {
 *     // to_string(finding.item), to_string(finding.reason)
 * }
 * @endcode
 */

#pragma once

#include "klv/misb1403/check.hpp"
#include "klv/misb1403/items.hpp"
