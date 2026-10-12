/**
 * @file misb1108.hpp
 * @brief MISB ST 1108 - Interpretability and Quality Local Set
 *
 * The set records image quality metrics (for example VNIIRS or GSD) and the compression of a Motion Imagery stream.
 * The values apply at one point of the imaging workflow. The set arrives as a stand-alone packet that ends with a
 * CRC-16-CCITT checksum.
 *
 * Each Metric Local Set (tag 4) holds one metric. The set can hold tag 4 more than once.
 */

#pragma once

#include "klv/misb1108/enumerations.hpp"
#include "klv/misb1108/fields.hpp"
#include "klv/misb1108/interpretability_quality_ls.hpp"
#include "klv/misb1108/metric_ls.hpp"
#include "klv/misb1108/packs.hpp"
