/**
 * @file misb0102.hpp
 * @brief MISB ST 0102 - Security Metadata Universal Set and Local Set
 *
 * The Local Set usually arrives nested in MISB ST 0601 tag 48 (see `misb0601::security_local_set`). The Universal Set
 * is a packet. `to_local` and `to_universal` convert a record of one set to the other (ST 0102 section 6.8).
 */

#pragma once

#include "klv/misb0102/enumerations.hpp"
#include "klv/misb0102/fields.hpp"
#include "klv/misb0102/security_ls.hpp"
#include "klv/misb0102/security_us.hpp"
