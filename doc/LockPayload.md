# Lock Payload

The lock payload is 20 bits and defines WHAT to lock and HOW to lock.

The Lock Payload is made of 5 lock fields, each 4 bits long:

- Kill (4 bits)
- Access (4 bits)
- EPC (4 bits)
- TID (4 bits)
- User (4 bits)

20 bits in total.

Each 4-bit field controls one memory area.

| Bit | Name             | Meaning                   |
| --- | ---------------- | ------------------------- |
| 3   | **Lock**         | Apply a lock              |
| 2   | **Unlock**       | Remove a lock             |
| 1   | **Permalock**    | Make lock permanent       |
| 0   | **Perma-unlock** | Make permanently unlocked |

- Only one action bit should be set per field
- 00xx means no change to that memory area
- Permanent actions cannot be reversed

Meaning of 4-bit values

| Binary | Hex   | Effect             |
| ------ | ----- | ------------------ |
| `0000` | `0x0` | No change          |
| `0010` | `0x2` | Lock               |
| `0001` | `0x1` | Unlock             |
| `1000` | `0x8` | Permanently lock   |
| `0100` | `0x4` | Permanently unlock |

Other combinations are invalid or ignored by compliant tags.

## Example 1: Non-permanent EPC lock

- Kill    = `0000`
- Access  = `0000`
- EPC     = `0010` (lock)
- TID     = `0000`
- User    = `0000`

Lock Payload bits: `0000 0000 0010 0000 0000`

The same in hex: `0x00200`

## Example 2: Perma-lock user memory

- Kill    = `0000`
- Access  = `0000`
- EPC     = `0000`
- TID     = `0000`
- User    = `1000`  (perma-lock)

Lock Payload bits: `0000 0000 0000 0000 1000`

The same in hex: `0x00008`

Permanent locks are irreversible.

Note that some tags do not support locking TID.
