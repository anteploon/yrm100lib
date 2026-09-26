# Running the scanner as a systemd service

The `scanner` binary polls one RFID module and exposes the EPCs it reads on a
Unix socket (or on stdout). Because each process talks to exactly one serial
device, running several modules means running several instances. This directory
ships a systemd **template unit**, so one file covers every reader.

| File | Purpose |
| --- | --- |
| `yrm100-scanner@.service` | Template unit, instantiated once per serial device |
| `yrm100-scanner.target` | Optional group so the whole fleet starts/stops together |

The instance identifier (`%i`) is the serial device name without the `/dev/`
prefix. For `/dev/ttyUSB0` the instance is `yrm100-scanner@ttyUSB0`.

## Prerequisites

```sh
make                                  # produces build/scanner
sudo useradd --system --no-create-home --shell /usr/sbin/nologin yrm100
```

The service user must be able to open the serial device. On Debian-based
systems `/dev/ttyUSB*` is owned by `root:dialout`, which the unit handles via
`SupplementaryGroups=dialout`.

## Install

```sh
sudo install -m 0644 deploy/yrm100-scanner@.service /etc/systemd/system/
sudo install -m 0644 deploy/yrm100-scanner.target   /etc/systemd/system/
sudo systemctl daemon-reload
```

Start one instance per connected module:

```sh
sudo systemctl enable yrm100-scanner.target
sudo systemctl enable yrm100-scanner@ttyUSB0 yrm100-scanner@ttyUSB1
sudo systemctl start  yrm100-scanner.target
```

Adding a third module later needs no new files:

```sh
sudo systemctl enable --now yrm100-scanner@ttyUSB2
```

## Operation

```sh
systemctl status 'yrm100-scanner@*'      # whole fleet
systemctl restart yrm100-scanner.target  # restart all instances
journalctl -fu yrm100-scanner@ttyUSB0    # follow one instance's log
```

Each instance writes EPCs to `/run/yrm100-<instance>/scanner.sock`. To read
them, a client must be in the socket's group (the service's `dialout` group, or
override `RuntimeDirectoryMode`/`SocketMode` as needed):

```sh
socat - UNIX-CONNECT:/run/yrm100-ttyUSB0/scanner.sock
```

Pass `-` instead of a socket path to stream EPCs to the journal instead of a
socket (useful for debugging without a client).

## Per-instance configuration

Defaults come from the unit:

| Variable | Default |
| --- | --- |
| `SERIAL_DEVICE` | `/dev/%i` |
| `INTERVAL_MS` | `1000` |
| `SOCKET_PATH` | `/run/yrm100-%i/scanner.sock` |

`EnvironmentFile=-/etc/yrm100/<instance>.conf` is optional (`-` means the file
may be absent). Because it is read *after* `Environment=`, anything it sets
overrides the defaults:

```
# /etc/yrm100/left.conf
SERIAL_DEVICE=/dev/yrm100-left
INTERVAL_MS=250
```

For a one-off tweak without an env file, use a drop-in:

```
/etc/systemd/system/yrm100-scanner@ttyUSB1.service.d/override.conf
```

## Stable device names

`/dev/ttyUSB0`, `/dev/ttyUSB1`, ... are assigned in enumeration order and
change across reboots and replugs, so instance names derived from them are not
stable. Pin each module to a role instead, then name instances after the role.

**Do not key on the serial number.** The CP2102 bridges on the tested hardware
(the YRM100 series modules) all ship with the same serial, `0001`:

```
$ udevadm info -q property -n /dev/ttyUSB0 | grep ID_SERIAL_SHORT
ID_SERIAL_SHORT=0001
$ udevadm info -q property -n /dev/ttyUSB1 | grep ID_SERIAL_SHORT
ID_SERIAL_SHORT=0001
```

That also means `/dev/serial/by-id/` is useless here — both modules collapse to
the same name and only one symlink survives. A rule matching
`ATTRS{serial}=="0001"` would match *both* devices.

The two devices *are* still distinguishable — by **physical USB port**, which
lands in `ID_PATH` (and drives `/dev/serial/by-path/`). On the reference setup:

```
/dev/ttyUSB0  ->  ID_PATH=platform-xhci-hcd.1-usb-0:2:1.0   (USB port 3-2)
/dev/ttyUSB1  ->  ID_PATH=platform-xhci-hcd.0-usb-0:2:1.0   (USB port 1-2)
```

Map those to friendly names:

```
# /etc/udev/rules.d/70-yrm100.rules
SUBSYSTEM=="tty", ENV{ID_PATH}=="platform-xhci-hcd.1-usb-0:2:1.0", SYMLINK+="yrm100-left"
SUBSYSTEM=="tty", ENV{ID_PATH}=="platform-xhci-hcd.0-usb-0:2:1.0", SYMLINK+="yrm100-right"
```

```
# /etc/yrm100/left.conf
SERIAL_DEVICE=/dev/yrm100-left
```

```sh
sudo udevadm control --reload && sudo udevadm trigger --subsystem-match=tty
sudo systemctl enable --now yrm100-scanner@left yrm100-scanner@right
```

Read the actual `ID_PATH` values off your own machine with
`udevadm info -q property -n /dev/ttyUSB0 | grep ID_PATH` — they depend on the
USB topology, not the module. This survives replugs **only into the same
physical port**; moving a module to a different port changes its `ID_PATH`.

If you want names that follow the module rather than the port, reprogram the
CP2102 EEPROM to give each bridge a unique serial (Silicon Labs' `cp210x`
programmer tools). After that, `ID_SERIAL_SHORT` differs and a
`ATTRS{serial}=="..."` rule works as originally described.

## Notes

- **Socket activation (`.socket` units) is not supported** by the current
  binary. `create_server_socket()` refuses to start if the socket path already
  exists, which is precisely what systemd would create first. Supporting it
  would require a code change.
- **Graceful shutdown already works**: the binary handles `SIGTERM`/`SIGINT`,
  disables continuous wave and deinits the module, so no `ExecStop` is needed.
- **`Restart=on-failure`** restarts after hardware/config faults (the binary
  exits non-zero) but not after a clean signal-driven stop (it exits zero).
- **`RuntimeDirectory=yrm100-%i`** is deliberately per-instance: a shared
  directory would be deleted when any one instance stops, taking the other
  instances' sockets with it.
- **`After=/Wants=dev-%i.device`** orders startup after the device node exists.
  Swap `Wants=` for `BindsTo=` if you want the instance to stop when the reader
  is unplugged (can be flaky on USB re-enumeration).
