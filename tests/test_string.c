#include <stdio.h>
#include <string.h>
#include "yrm100/yrm100_string.h"

static int expect_equal_string(const char *label, const char *got, const char *expected)
{
    if (strcmp(got, expected) != 0)
    {
        printf("FAIL: %s expected \"%s\" got \"%s\"\n", label, expected, got);
        return 1;
    }
    return 0;
}

int test_string_functions(void)
{
    int failures = 0;
    char tx_power[YRM100_PARAM_TX_POWER_STRING_LENGTH];
    yrm100_rfid_tag_t tag;
    char tag_buf[YRM100_TAG_STRING_LENGTH];

    memset(&tag, 0, sizeof(tag));
    tag.pc = 0x1234;
    tag.crc = 0xABCD;
    tag.rssi = -42;
    tag.epc[0] = 0xDE;
    tag.epc[1] = 0xAD;
    tag.epc[2] = 0xBE;
    tag.epc[3] = 0xEF;
    tag.epc[4] = 0x01;
    tag.epc[5] = 0x23;
    tag.epc[6] = 0x45;
    tag.epc[7] = 0x67;
    tag.epc[8] = 0x89;
    tag.epc[9] = 0xAB;
    tag.epc[10] = 0xCD;
    tag.epc[11] = 0xEF;

    failures += expect_equal_string(
        "tx power clamp low",
        yrm100_convert_to_tx_power_string(900U, tx_power),
        "10dBm");
    failures += expect_equal_string(
        "tx power clamp high",
        yrm100_convert_to_tx_power_string(2700U, tx_power),
        "26dBm");
    failures += expect_equal_string(
        "region string",
        yrm100_convert_to_region_string(YRM100_PARAM_REGION_USA),
        "USA");
    failures += expect_equal_string(
        "tag string",
        yrm100_get_tag_string(&tag, tag_buf),
        "EPC: DEADBEEF0123456789ABCDEF, PC: 1234, RSSI:  -42dBm, CRC: ABCD");

    return failures;
}
