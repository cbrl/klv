/**
 * @file misb1002.hpp
 * @brief MISB ST 1002 - Range Motion Imagery: the Range Image Local Set
 *
 * The set usually arrives nested in MISB ST 0601 tag 97 (see `misb0601::range_image_local_set`). This header
 * includes all parts of the definition:
 * - `misb1002/enumerations.hpp`: the enumerations and the Range Image Enumerations pack
 * - `misb1002/section_data.hpp`: the Section Data pack
 * - `misb1002/fields.hpp`: the field descriptors
 * - `misb1002/range_image_ls.hpp`: the set, which lists the fields
 */

#pragma once

#include "klv/misb1002/enumerations.hpp"
#include "klv/misb1002/fields.hpp"
#include "klv/misb1002/range_image_ls.hpp"
#include "klv/misb1002/section_data.hpp"
