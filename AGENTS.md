# Repository Guidelines

## Project Structure & Module Organization
This repository is a small C library plus a sample program for a UHF RFID device.
Core code lives under `src/rfid_uhf/` (public headers and implementation files).
The sample app is `src/example.c`, and build rules are in `src/Makefile`.
There are no dedicated test or asset directories today.

## Build, Test, and Development Commands
- `make -C src` builds the sample binary `src/example` using GCC.
- `make -C src clean` removes object files and the `example` binary.
- `make -C src test` builds and runs a minimal smoke test binary.
- `make -C src rebuild` runs a clean build (equivalent to `clean` then `all`).

## Coding Style & Naming Conventions
Use C with 4-space indentation and Allman-style braces (opening brace on its own line).
Function and variable names use `snake_case`; types end in `_t`; macros are `UPPER_CASE`.
Keep code warning-free with the existing flags in `src/Makefile` (`-Wall -Wextra -Werror -pedantic`).
Prefer adding APIs to `src/rfid_uhf/*.h` and their implementations to matching `.c` files.

## Testing Guidelines
Automated tests are minimal; `tests/test_example.c` is a smoke test to verify the
library builds and headers are accessible. Validate functional changes by building
and running `src/example` against real hardware (document the device and port used).
Add additional tests under `tests/` and name files `test_*.c`.

## Commit & Pull Request Guidelines
Recent history uses short, descriptive commit messages starting with a verb
(e.g., "Added comments", "Refactor + comments"). Follow that pattern and keep
messages under ~60 characters when possible.
Pull requests should include a concise summary, steps to reproduce or verify,
and any hardware/OS specifics (e.g., serial port path or `_WIN32` behavior).

## Configuration & Hardware Notes
This project talks to a serial device; keep platform-specific handling localized
and document any baud rate or timing changes in your PR description.
