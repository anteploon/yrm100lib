# Code Review — yrm100lib

**Scope:** full library (`src/yrm100/`, `src/example.c`, `src/scanner.c`), build system,
tests, CI, and docs.
**Method:** static reading plus runtime verification — clean build, `make test`, an
ASan/UBSan fuzz of the command/response path, and targeted probes.

**Verification performed on this checkout:**

| Check | Command | Result |
| --- | --- | --- |
| Build | `make` (gcc 14.2, `-Wall -Wextra -Werror -Wconversion -pedantic`) | clean, exit 0 |
| Unit tests | `make test` | `PASS`, exit 0 |
| Parser fuzz | 2,000,000 random streams through `yrm100_command_read_response` under `-fsanitize=address,undefined` | no findings |
| Module-error probe | crafted error frame → `single_poll` / `kill` | both return `+21` (see #2) |
| Command latency | 200 no-op commands | 15,018 ms ≈ 75 ms each (see #3) |

---

## Summary

| # | Severity | Area | Finding |
| --- | --- | --- | --- |
| 1 | High | CI | Workflow calls `./configure`, `make check`, `make distcheck` — none exist; every run fails |
| 2 | High | Error model | Module error codes return as **positive** values; `< 0` callers treat failures as success |
| 3 | Medium | Performance | Hard-coded 75 ms blocking sleep after every command |
| 4 | Medium | Parser | Read-memory parser trusts caller's length, ignores the response's length field |
| 5 | Medium | Docs | `AGENTS.md` / `README.md` describe a different tree and stale feature set |
| 6 | Low | API | Naming, NULL-check, and return-convention inconsistencies |
| 7 | Low | Robustness | A few sharp edges (static buffer, handle-vs-error pun, ignored returns) |
| 8 | Low | Tests | No direct coverage for several modules |

---

## High

### 1. CI workflow can never pass

`.github/workflows/c-cpp.yml:16-23` runs:

```yaml
- run: ./configure      # does not exist
- run: make             # works
- run: make check       # no such target
- run: make distcheck   # no such target
```

Verified:

```
$ ls configure
ls: cannot access 'configure': No such file or directory
$ make check
make: *** No rule to make target 'check'.  Stop.
```

The project has no autotools/`configure` and no `check`/`distcheck` targets — its test
entry point is `make test`. Every push and PR to `main` fails at the "configure" step
before building anything.

**Fix:** replace the steps with what the Makefile actually provides:

```yaml
- run: make
- run: make test
```

### 2. Module errors are returned as positive values, so `< 0` checks see success

Library functions document "`0` on success, otherwise error code". But the 14 error
branches that call `yrm100_parse_get_error_code()`
(`src/yrm100/yrm100_command.c:340,387,434,475,484,749,791,835,877,912,955,1020,1053,1086`)
return the **module's** code, which is a positive byte (`0x09`–`0x20`, per
`yrm100_error.h`). `yrm100_parse_get_error_code()` returns `buf[5]` verbatim
(`src/yrm100/yrm100_parse.c:159`).

Reproduced with a crafted `0x15` (inventory-failed) error response:

```
single_poll() on module error 0x15 -> 21  (caller '<0' means error: NO - looks like success)
kill()        on module error       -> 21  (caller '<0' means error: NO - looks like success)
```

Consequences:

- `src/scanner.c:228` does `if (result < 0)` after `yrm100_command_single_poll()`. A reader
  that reports a real module error is treated as a **successful, tag-less scan** — the
  failure is swallowed and output silently stops.
- `yrm100_error_code_to_string()` has no cases for positive module codes, so logging
  `21` yields `"Unknown error"`; the caller should have used
  `yrm100_module_error_code_to_string()`.
- The convention is already muddled: `get_tx_power` / `get_operating_region` return a
  **positive** payload on success, while setters return `0`.

**Fix (pick one and apply consistently):**
- Map module errors to a library error (e.g. return `YRM100_ERROR_COMMAND_FAILED`,
  keep the module byte in a new context field or resolve it via
  `yrm100_module_error_code_to_string()` at the call site); or
- Define negative library aliases for the module codes and translate before returning.

Either way, callers must be able to distinguish success from failure with a single
`< 0` / `== YRM100_STATUS_OK` test.

---

## Medium

### 3. Fixed 75 ms sleep on every command

`src/yrm100/yrm100_command.c:59` unconditionally (on the send path) sleeps
`YRM100_COMMAND_RESPONSE_DELAY_USEC` = `75000` (`yrm100_command.h:10`) before reading.
Measured:

```
200 commands took 15018 ms total   (~75 ms each)
 20 commands took  1504 ms total
```

This caps throughput at ~13 commands/second and taxes **every** API call, including ones
that could return instantly. The subsequent read already uses a timeout (`VTIME=3`), so
the fixed sleep is largely redundant. Prefer removing it and polling the port with a
short timeout (or make the delay configurable/adaptive).

### 4. `yrm100_parse_read_tag_memory_response` trusts the requested length

`src/yrm100/yrm100_parse.c:60`:

```c
size_t data_byte_count = data_length * 2; // caller's requested words -> bytes
```

The parser copies `data_byte_count` bytes from the response regardless of the payload
length actually declared in the frame (`response[3..4]`). It is memory-safe today only
because `yrm100_frame_is_read_tag_memory_response()` pins `buf_size == payload + 7` and
the guard rejects a short frame. But if the module ever returns a different word count
than requested, the result is a silently truncated (or rejected) tag and a `data_length`
that reflects the *request*, not the *actual* payload:

```c
tag->data = data_buf;
tag->data_length = data_byte_count;   // requested, not received
```

**Fix:** derive the byte count from the frame's payload-length field, validate it against
the requested maximum, and set `data_length` to the real value.

### 5. Documentation describes a different repository

`AGENTS.md` (agent-facing) is materially wrong:

- `AGENTS.md:5,19` — "Core code lives under `src/rfid_uhf/`". Actual directory is
  `src/yrm100/`; there is no `src/rfid_uhf/`.
- `AGENTS.md:7` — "There are no dedicated test or asset directories today." `tests/`
  exists with 8 files (`test_context.c`, `test_command.c`, `test_parse.c`, `test_string.c`,
  `test_serial.c/h`, `test_scanner.c`, `test_example.c`).
- `AGENTS.md:18` lists the flags as `-Wall -Wextra -Werror -pedantic`, but `Makefile:6`
  also enables `-Wconversion` and the UBSan flags.

`README.md` is stale relative to the code:

- `README.md:9,20` — "setting the select parameters [is] not yet implemented" /
  "[ ] Get and set select parameters". Both are implemented
  (`yrm100_command_get_select_parameters`, `yrm100_command_set_select_parameters`).
- `README.md:32` — "[ ] Read tag memory area" is unchecked, but
  `yrm100_command_read_tag_memory_area()` is implemented and exported.
- `README.md:44` shows sample output "Set operating region to China 900MHz", while
  `src/example.c:52` now sets `YRM100_PARAM_REGION_EUROPE`.

Guidance (and this repo's own stated preference) is that documented claims should be
verified against the tree; these are not. Recommend refreshing both files, ideally with a
check that the feature checklist matches the exported API.

---

## Low / polish

### 6. API consistency

- **Naming:** `unpack_query_parameters` (`yrm100_command.c:289`) is the only externally
  linked function without the `yrm100_` prefix, and it has no prototype in any header.
  Make it `static` or rename + declare it.
- **Typo in the public API:** `continous` is misspelled throughout
  (`yrm100_command_set_continous_wave`, `yrm100_command_enable_continous_wave`,
  `YRM100_PARAM_CONTINOUS_WAVE_ON/OFF`). It's a stable, exported spellings — worth fixing
  now if there are no external consumers.
- **NULL checks:** `yrm100_pack_select_parameters` (`yrm100_command.c:269`) dereferences
  `data` without a check, while the sibling `yrm100_pack_query_parameters`
  (`yrm100_command.c:274`) checks.
- **Read-only params:** most `*_parameters_t *` parameters are not `const`.

### 7. Robustness sharp edges

- `yrm100_get_tag_pc_string` / `_crc_string` / `_epc_string` / `_rssi_string`
  (`yrm100_string.c:7,13,19,34`) dereference `tag` without a NULL check, unlike
  `yrm100_print_tag_info`.
- `yrm100_convert_to_q_string` (`yrm100_string.c:182`) returns a pointer to a `static`
  buffer — not thread-safe/reentrant.
- `yrm100_reset_tag_buf` (`yrm100_util.c:39,57`) calls `free(t->data)`, so it is only safe
  on zero-initialized memory (documented, but easy to misuse). `yrm100_deinit()` does not
  free `multi_poll_target` data.
- `yrm100_serial_open` returns a status code *as* the port handle on POSIX
  (`yrm100_serial.c:116`, a negative value in an `int` fd), and on Windows returns
  `INVALID_HANDLE_VALUE` after `perror` with a dead commented-out return
  (`yrm100_serial.c:19`). `yrm100_init` copes, but conflating handles with status codes is
  fragile if more call sites are added.
- `yrm100_command_get_module_manufacturer` omits the `result == 0` guard that the hardware
  and software getters have; that guard is dead anyway because
  `yrm100_command_read_response` never returns `0` (it returns `YRM100_ERROR_READ_TIMEOUT`
  for an empty first read).
- `src/example.c:64` ignores the return of `yrm100_command_get_query_parameters` and then
  prints the (zeroed) parameters.
- `src/scanner.c` `scan_for_tags()` logs a poll failure but returns `0`, so the loop
  continues forever on a dead reader — combined with #2, a failing reader can silently
  emit nothing while appearing healthy.

### 8. Test coverage

`make test` covers: context init/deinit, a handful of getters, ASCII/poll/read-memory
parsing, string formatting, and the scanner's `write_all` blocking behavior. There is no
direct instrumentation for `yrm100_frame.c`, `yrm100_error.c`, `yrm100_param.c`,
`yrm100_util.c`, or the multi-frame/notice branch of `yrm100_command_read_response`, and
no negative tests for `yrm100_command_read_tag_memory_area`. The fuzz run above exercised
the single-response getter path heavily and found nothing, so the priority gap is the
notice/multi-frame path and the error-return semantics of #2 (a regression test that
asserts a module error yields a negative/`< 0` result would have caught it).

---

## What's solid

- Clean build under a strict warning set (`-Werror -Wconversion -pedantic`) on gcc 14.
- Frame checksum/length validation is defensive, and the receive path's resynchronization
  on noise is covered by tests and survived a 2M-case ASan/UBSan fuzz.
- Windows (`_WIN32`) and POSIX serial backends are cleanly separated behind one header.
- The systemd template unit and its README are detailed, and their claims (socket
  activation unsupported, graceful SIGTERM shutdown, per-instance `RuntimeDirectory`)
  match the code.

## Suggested order of work

1. Fix CI (#1) — cheap and unblocks everything else.
2. Normalize error returns so `< 0` reliably means failure (#2) and add a regression test.
3. Remove/parameterize the fixed 75 ms sleep (#3).
4. Harden the read-memory parser to use the declared payload length (#4).
5. Refresh `AGENTS.md` and `README.md` (#5).
6. Clean up the Low items opportunistically (#6–#8).
