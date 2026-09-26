# Repository Guidelines

## Project Structure & Module Organization
This repository is a small C library plus sample programs for a UHF RFID reader
(YRM100 series / MagicRF M100).
- Library code lives under `src/yrm100/` — public headers and their matching `.c`
  implementations (`yrm100_command`, `yrm100_frame`, `yrm100_parse`, `yrm100_serial`,
  `yrm100_string`, `yrm100_param`, `yrm100_print`, `yrm100_error`, `yrm100_types`,
  `yrm100_util`, and the `yrm100` init/deinit entry points).
- `src/example.c` — minimal demo program (configure module, single poll, print tags).
- `src/scanner.c` — long-running scanner that polls on an interval and serves EPCs over a
  Unix socket or stdout.
- `tests/` — host-side tests, one `test_*.c` per area, plus `test_serial.[ch]` (a fake
  serial backend).
- `doc/` — protocol and data-structure notes in Markdown.
- `dev-docs/` — vendor protocol and firmware reference material (PDF/DOCX).
- `deploy/` — systemd template unit and target for running the scanner as a service.
- Build output goes to `build/` (gitignored).

## Build, Test, and Development Commands
- `make` builds `build/example` and `build/scanner` with GCC.
- `make clean` removes the `build/` directory.
- `make test` builds and runs the host test binaries (`build/test_example`,
  `build/test_scanner`), exiting non-zero on failure.
- `make rebuild` runs `clean` then `all`.
There is no `configure` script and no `check`/`distcheck` target; `make test` is the
test entry point.

## Coding Style & Naming Conventions
Use C with 4-space indentation and Allman-style braces (opening brace on its own line).
Function and variable names use `snake_case`; types end in `_t`; macros are `UPPER_CASE`.
Keep code warning-free with the existing flags in `Makefile`
(`-Wall -Wextra -Werror -Wconversion -pedantic`, plus UBSan).
Prefer adding APIs to `src/yrm100/*.h` and their implementations to matching `.c` files.

## Testing Guidelines
- Host tests live in `tests/` and are named `test_*.c`. `tests/test_example.c` provides
  `main` and calls each area's `test_*_functions()`; `tests/test_scanner.c` builds a
  separate binary with its own `main`.
- `tests/test_serial.c` supplies a fake serial port so command and parsing logic can be
  tested without hardware — the test build links it instead of
  `src/yrm100/yrm100_serial.c`.
- Validate functional changes against real hardware by building and running
  `build/example` (or `build/scanner`), and document the device and port used.
- Add new tests under `tests/` and name files `test_*.c`.

## Commit & Pull Request Guidelines
Recent history uses short, descriptive commit messages starting with a verb
(e.g., "Add scanner debug output", "Fix POSIX portability"). Follow that pattern and keep
messages under ~60 characters when possible.
Pull requests should include a concise summary, steps to reproduce or verify,
and any hardware/OS specifics (e.g., serial port path or `_WIN32` behavior).

## Configuration & Hardware Notes
This project talks to a serial device at 115200 8N1 with no flow control; keep
platform-specific handling (`_WIN32` vs POSIX) localized and document any baud rate or
timing changes in your PR description.
