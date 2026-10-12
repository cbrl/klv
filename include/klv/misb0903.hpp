/**
 * @file misb0903.hpp
 * @brief MISB ST 0903 - Video Moving Target Indicator (VMTI) Local Set
 *
 * The set arrives as a stand-alone packet, or nested in MISB ST 0601 tag 74 (see `misb0601::vmti_local_set`).
 */

#pragma once

#include "klv/misb0903/algorithm_ls.hpp"
#include "klv/misb0903/enumerations.hpp"
#include "klv/misb0903/fields.hpp"
#include "klv/misb0903/location.hpp"
#include "klv/misb0903/ontology_ls.hpp"
#include "klv/misb0903/vchip_ls.hpp"
#include "klv/misb0903/vfeature_ls.hpp"
#include "klv/misb0903/vmask_ls.hpp"
#include "klv/misb0903/vmti_ls.hpp"
#include "klv/misb0903/vobject_ls.hpp"
#include "klv/misb0903/vtarget_ls.hpp"
#include "klv/misb0903/vtracker_ls.hpp"
