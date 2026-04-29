/**
 * @file misb0102.hpp
 * @brief MISB ST 0102 - Security Metadata Local Set
 *
 * The set usually arrives nested in MISB ST 0601 tag 48 (see `misb0601::security_local_set`).
 */

#pragma once

#include "klv/misb0102/enumerations.hpp"
#include "klv/misb0102/fields.hpp"
#include "klv/misb0102/security_ls.hpp"
