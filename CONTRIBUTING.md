# Contributing

Format the code with the [`.clang-format`](.clang-format) file of this
project. The `format-check` and `format-fix` developer targets run it.

To add a MISB field, add one descriptor struct to the standard's header and add
the struct to the `field_list` of its set. To add a value format, add one codec
header to [`codecs/`](include/klv/codecs) and include it from
[`codec.hpp`](include/klv/codec.hpp).

## Getting started

Helpful notes for developers can be found in the [`HACKING.md`](HACKING.md)
document.

In addition to the above, if you use the presets file as instructed, then you
should NOT check it into source control, just as the CMake documentation
suggests.
