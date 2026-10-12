/**
 * @file misb0602.hpp
 * @brief MISB ST 0602 - Annotation Metadata Set
 *
 * An annotation data stream is a sequence of KLV packets. Three standalone preface items help a receiver read the
 * Annotation Universal Sets that follow them. These items are Byte Order, Active Lines per Frame, and Active Samples
 * per Line. Each Annotation Universal Set holds one event of one annotated object, for example NEW or MOVE. The set
 * has no checksum.
 * @code
 * for (auto const& packet : klv::misb0602::decode_stream(bytes)) {
 *     if (!packet) { report(packet.error()); continue; }
 *     if (auto const* annotation = std::get_if<klv::record<klv::misb0602::annotation_us>>(&*packet)) {
 *         for (auto const& finding : klv::misb0602::check(*annotation)) {
 *             // to_string(finding.item), to_string(finding.reason)
 *         }
 *     }
 * }
 * @endcode
 *
 * `check` gives the items that an event requires, but that a record does not hold. It does not check these rules:
 * - The preface items come before an Annotation Universal Set at least each quarter second (ST 0602.4-04 to -06).
 * - Each annotated object gets a STATUS or MODIFY event at least once every five seconds (ST 0602.4-17).
 * - An annotation without a Z-Order has a Z-Order of zero for rendering (ST 0602.4-18).
 */

#pragma once

#include "klv/misb0602/annotation_us.hpp"
#include "klv/misb0602/check.hpp"
#include "klv/misb0602/enumerations.hpp"
#include "klv/misb0602/fields.hpp"
#include "klv/misb0602/preface.hpp"
#include "klv/misb0602/stream.hpp"
