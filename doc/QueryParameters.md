# Query Parameters

See **yrm100_query_parameters_t** in **yrm100_types.h**

The QUERY command answers three questions:
- Who should talk? → Sel, Session, Target
- How should they talk? → DR, M, TRext
- When should they talk? → Q (slots)

## DR - Divide Ratio (1 bit)
Controls the tag backscatter data rate (uplink)
| Value | Meaning            |
| ----- | ------------------ |
| 0     | DR = 8             |
| 1     | DR = 64/3 (~21.33) |

### What it does
- Determines how fast the tag → reader reply is
- Affects range vs. speed trade-off

### Used when
- High-speed conveyor → DR = 64/3
- Long-range or noisy RF → DR = 8

## M – Backscatter Encoding (2 bits)
Controls how the tag encodes its reply
| Bits | Encoding    |
| ---- | ----------- |
| 00   | M = 1 (FM0) |
| 01   | M = 2       |
| 10   | M = 4       |
| 11   | M = 8       |

### What it does
- Defines how many subcarrier cycles per bit
- Higher M = more redundancy

### Practical impact
- M=1 (FM0) → Fastest, least robust
- M=8 → Slowest, most robust

### Typical usage
- Short range, fast reading → M=1 or 2
- Long range, difficult RF → M=4 or 8

## TRext – Tag Response Extension (1 bit)
Indicates presence of a pilot tone
| Value | Meaning            |
| ----- | ------------------ |
| 0     | No pilot tone      |
| 1     | Pilot tone present |

### What it does
- Adds a short unmodulated CW tone before tag reply
- Helps the reader synchronize to the tag signal

### Practical impact
- TRext=1 improves reliability
- Slightly reduces throughput

### Usually
- Required for some combinations of DR and M
- Often automatically chosen by the reader

## Sel – Tag Selection (2 bits)
Selects which tag population participates
| Bits    | Meaning            |
| ------- | ------------------ |
| 00 / 01 | All tags           |
| 10      | ~SL (non-selected) |
| 11      | SL (selected)      |

### What it does
- Filters tags based on the SL flag
- Used with SELECT commands

### Practical impact
- Enables reading subsets of tags
- Useful in complex tag populations

## Session (2 bits)
Defines the inventory session (persistence of tag state)
| Bits | Session |
| ---- | ------- |
| 00   | S0      |
| 01   | S1      |
| 10   | S2      |
| 11   | S3      |

### What it does
- Determines how long a tag “remembers” it was inventoried
- Controls how tags reset their flags

### Typical behavior
- S0 → resets quickly (fast repeated reads)
- S1 → common default
- S2 / S3 → long persistence (portal, dock doors)

## Target (1 bit)
Selects which inventory flag to target
| Value | Meaning  |
| ----- | -------- |
| 0     | Target A |
| 1     | Target B |

### What it does
- Tags toggle between A and B states
- Reader alternates target to avoid rereading same tags

### Practical impact
- Helps manage repeated inventories
- Used with session to control tag participation

## Q – Slot Count (4 bits)
Controls anti-collision slot count
| Q  | Slots       |
| -- | ----------- |
| 0  | 1 slot      |
| 1  | 2 slots     |
| 4  | 16 slots    |
| 15 | 32768 slots |
Slots = 2^Q

### What it does
- Defines how many time slots tags randomly choose from
- Reduces collisions

### Practical impact
- Low tag count → small Q
- High tag density → large Q

### Most readers
- Use dynamic Q adjustment (Q-algorithm)

### Typical Default Example
`
DR=8
M=2
TRext=1
Sel=All
Session=S1
Target=A
Q=4 (16 slots)
`
This balances speed, reliability, and collision avoidance for most environments.
