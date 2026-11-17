# v0.2.0 local verification

- Portable C++20 core compiled with GCC 14.2.0 and CMake 3.31.6.
- Compiler warnings enabled as errors (`-Wall -Wextra -Wpedantic -Werror`).
- CTest: 1/1 test passed.
- Test coverage includes start settings, `kS38` color channels, object IDs/positions, channel usage, explicit RGB trigger samples, glow-disable flag detection, theme mixing, malformed records, empty input, oversized input, and source-string immutability.

Not verified in this environment: Geode Android64 linking, GMD-API runtime loading, Geometry Dash in-game behavior, and installation on an Android device. Those are the next GitHub Actions/device checks.
