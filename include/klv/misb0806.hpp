/**
 * @file misb0806.hpp
 * @brief MISB ST 0806 - Remote Video Terminal Local Set
 *
 * The set usually arrives nested in MISB ST 0601 tag 73 (see `misb0601::rvt_local_set`).
 */

#pragma once

#include "klv/misb0806/aoi_ls.hpp"
#include "klv/misb0806/enumerations.hpp"
#include "klv/misb0806/fields.hpp"
#include "klv/misb0806/poi_ls.hpp"
#include "klv/misb0806/rvt_ls.hpp"
#include "klv/misb0806/user_defined_ls.hpp"
