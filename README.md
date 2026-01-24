# yrm100lib

RFID UHF library supporting YRM100 series modules (same as MagicRF M100 http://www.magicrf.com/product_en.htm)

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
