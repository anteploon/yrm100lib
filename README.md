# yrm100lib

RFID UHF library supporting YRM100 series modules (same as [MagicRF](http://www.magicrf.com/product_en.htm) M100)

Tested with [Invelion](http://www.invelion.net/) products YRM1005 and YRM1001

There exists a variation of the protocol, where the **end byte** and  the **cheksum byte** are swapped. The plan is that once I get my hands on one of these devices, there will be a support for them added also - should be pretty simple.

Multi poll and setting the select parameters are **not yet implemented**

## Features

- [x] Get module manufacturer string
- [x] Get module hardware version
- [x] Get module software version
- [x] Single poll tag(s)
- [ ] Multi poll tags
- [x] Set module idle sleep timer
- [ ] Sleep
- [ ] Get and set select parameters
- [x] Set select mode
- [x] Get and set operating region (frequency)
- [ ] Get and set operating channel
- [x] Get and set TX power
- [x] Enable / disable continous wave mode
- [ ] Get and set query parameters
- [ ] Get and set receiver demodulator parameters
- [ ] Test RF input blocking signal
- [ ] Test channel RSSI
- [ ] Lock tag
- [ ] Kill tag
- [ ] Read tag memory area
- [ ] Write tag memory area
- [ ] Enable automatic frequency hopping

# Example application

The following is the output from the example.c:

    Serial port: /dev/ttyUSB0
    MagicRf M100 26dBm V1.0 / FW: V2.3.5
    ✓ Disabled idle sleep timer
    ✓ Enabled continuous wave
    ✓ Set operating region to China 900MHz
    ------------------------------------------------
                                    Query Parameters
    ------------------------------------------------
                 Divide Ratio (DR): 8
          Backscatter Encoding (M): 1 (FM0)
    Tag Response Extension (TReXt): No pilot tone
               Tag Selection (Sel): Non-selected
                           Session: S0
                            Target: A
                    Slot Count (Q): -
    ------------------------------------------------
    EPC: E28011700000021AC6B3A1F9, PC: 3400, RSSI:  -52dBm, CRC: 9C5E
    EPC: 000000000000000000000000, PC: 0000, RSSI:    0dBm, CRC: 0000
    EPC: 000000000000000000000000, PC: 0000, RSSI:    0dBm, CRC: 0000
    EPC: 000000000000000000000000, PC: 0000, RSSI:    0dBm, CRC: 0000
    EPC: 000000000000000000000000, PC: 0000, RSSI:    0dBm, CRC: 0000
