# klv

klv is a header-only C++23 library that decodes and encodes KLV (SMPTE 336)
metadata. It includes definitions for MISB ST 0601 (UAS Datalink Local Set) and
the local sets that nest in it:
- ST 0102 (Security)
- ST 0603 (MISP Time System and Timestamps)
- ST 0806 (Remote Video Terminal)
- ST 0902 (Motion Imagery Sensor Minimum Metadata Set)
- ST 0903 (VMTI)
- ST 1002 (Range Image)
- ST 1107 (Metric Geopositioning)
- ST 1108 (Interpretability and Quality)
- ST 1204 (MIIS Core Identifier)
- ST 1206 (SAR Motion Imagery)
- ST 1403 (SARMI Threshold Metadata Sets)
- ST 1507 (Motion Imagery Sensor Timing)
- ST 1601 (Geo-Registration)
- ST 1602 (Composite Imaging)
- ST 1603 (Time Transfer Metadata)
- ST 1607 (Segment and Amend)

## Decoding

```cpp
#include <klv/klv.hpp>

namespace kv = klv::misb0601;

void handle(klv::byte_span bytes) {
    for (auto const& packet : klv::decode_packets<kv::uas_datalink_ls>(bytes)) {
        if (!packet) {
            // packet.error().message, packet.error().offset
            continue;
        }
        if (auto const* uas = std::get_if<klv::record<kv::uas_datalink_ls>>(&*packet)) {
            // get<E>() returns std::expected<T, klv::field_error>
            if (auto const& latitude = uas->get<kv::sensor_latitude>()) {
                use(*latitude);
            }
        }
    }
}
```

`decode_packets` splits a buffer into packets, checks the universal key and the
checksum, and continues at the next universal label after a malformed packet. A
field can have no value. Its `field_error` gives the reason, for example
`absent`, `empty` (length of zero), `error_indicator`, or `bad_length`.

A set can hold some fields more than once, for example the ST 0601 Segment
Local Set. Such a field has a list value with one item for each occurrence, and
`record::add` appends an item.

The ST 0601 Segment and Amend Local Sets (ST 1607) hold records of the ST 0601
set, so a child set can hold other child sets. `misb1607::merge` applies a
child set to its parent. An empty field in an Amend Local Set clears the parent
value. A set more than `klv::max_nesting_depth` (16) levels deep does not
decode.

## Encoding

```cpp
auto uas = klv::record<kv::uas_datalink_ls>{};
uas.set<kv::precision_time_stamp>(timestamp);
uas.set<kv::mission_id>("Mission 12");
uas.set<kv::sensor_latitude>(60.1768);

auto const bytes = klv::encode_packet(uas);  // result<byte_vector>
```

The encoder writes the fields in definition order and adds the checksum as the
last field. Each set uses the checksum of its standard. ST 0601 and ST 0903 use
a 16-bit sum, ST 1002, ST 1107, and ST 1108 use CRC-16-CCITT, and ST 0806
uses CRC-32/MPEG-2.

A set inside another set has no checksum, as ST 0903.6 and ST 1002.3 require.
The checksum of the parent packet covers the nested bytes. The decoder keeps a
nested checksum value that a producer sends anyway, but does not verify it.

## Defining fields and sets

All metadata is static. Each field is a struct with a tag, a name, optional
units, and a constexpr codec object. Each set is a list of fields. The compiler
builds the tag dispatch table and a typed record for each set, and rejects
duplicate tags and access to a field that is not in the set.

```cpp
struct platform_heading_angle {
    static constexpr uint32_t tag = 5;
    static constexpr std::string_view name = "Platform Heading Angle";
    static constexpr std::string_view units = "deg";
    static constexpr klv::codec::mapped<uint16_t> format{0.0, 360.0};
};

struct my_set {
    static constexpr std::string_view name = "My Set";
    static constexpr klv::universal_key key = klv::ul("06 0E 2B 34 ...");
    using fields = klv::field_list<platform_heading_angle /* , ... */>;
};
```

A set with a checksum names its `checksum_field`. It can also name a
`checksum_function`, for example `klv::crc32_mpeg2`. Without it, the set uses
the ST 0601 16-bit sum.

The built-in base codecs are:
- `integer`
- `ber_oid`
- `mapped`
- `imapb` (MISB ST 1201)
- `imapb_value` (MISB ST 1201, with the byte length in the value)
- `string`
- `timestamp`
- `nano_timestamp`
- `ieee754`
- `ieee754_variable`
- `enumeration`
- `bytes`
- `uuid`
- `nested` (a local set inside a field)
- `repeated` (a field that a set can hold more than once)
- `series` (length-prefixed elements)
- `ber_oid_list`
- `mdarray` (MISB ST 1303)
- `sdcc_flp` (MISB ST 1010)

A new value format is one new codec type. Each codec has its own header in
[`include/klv/codecs/`](include/klv/codecs). The MISB definitions are in
dedicated folders, such as [`include/klv/misb0601/`](include/klv/misb0601).

# Building and installing

See the [BUILDING](BUILDING.md) document.

# Contributing

See the [CONTRIBUTING](CONTRIBUTING.md) document.
