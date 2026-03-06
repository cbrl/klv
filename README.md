# klv

klv is a header-only C++23 library that decodes and encodes KLV (SMPTE 336)
metadata. It includes definitions for MISB ST 0601 (UAS Datalink Local Set) and
MISB ST 0102 (Security Metadata Local Set).

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
field that is absent, holds the error value of its format, or has a bad length
has an empty value with the reason (`field_error`).

## Encoding

```cpp
auto uas = klv::record<kv::uas_datalink_ls>{};
uas.set<kv::precision_time_stamp>(timestamp);
uas.set<kv::mission_id>("Mission 12");
uas.set<kv::sensor_latitude>(60.1768);

auto const bytes = klv::encode_packet(uas);  // result<byte_vector>
```

The encoder writes the fields in definition order and adds the checksum as the
last field.

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

The built-in codecs are `integer`, `mapped`, `string`, `timestamp`, `ieee754`,
`enumeration`, `bytes`, and `nested` (a local set inside a field). A new value
format is one new codec type. Each codec has its own header in
[`include/klv/codecs/`](include/klv/codecs).

# Building and installing

See the [BUILDING](BUILDING.md) document.

# Contributing

See the [CONTRIBUTING](CONTRIBUTING.md) document.
