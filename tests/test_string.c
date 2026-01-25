#include <stdio.h>
#include <string.h>
#include "yrm100/yrm100_param.h"
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

static int test_tag_strings(void)
{
    yrm100_rfid_tag_t tag;
    memset(&tag, 0, sizeof(tag));
    tag.rssi = (signed char)-70;
    tag.pc = (unsigned short)0x1234;
    tag.crc = (unsigned short)0xBEEF;

    unsigned char epc_bytes[YRM100_TAG_EPC_BYTE_COUNT] = {
        0x00, 0x01, 0xAB, 0xCD, 0xEF, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70};
    memcpy(tag.epc, epc_bytes, sizeof(epc_bytes));

    char pc[YRM100_TAG_PC_STRING_LENGTH];
    char crc[YRM100_TAG_CRC_STRING_LENGTH];
    char epc[YRM100_TAG_EPC_STRING_LENGTH];
    char rssi[YRM100_TAG_RSSI_STRING_LENGTH];
    char tag_string[YRM100_TAG_STRING_LENGTH];

    int failures = 0;
    failures += expect_equal_string("tag pc string",
        yrm100_get_tag_pc_string(&tag, pc),
        "1234");
    failures += expect_equal_string("tag crc string",
        yrm100_get_tag_crc_string(&tag, crc),
        "BEEF");
    failures += expect_equal_string("tag epc string",
        yrm100_get_tag_epc_string(&tag, epc),
        "0001ABCDEF10203040506070");
    failures += expect_equal_string("tag rssi string",
        yrm100_get_tag_rssi_string(&tag, rssi),
        " -70dBm");
    failures += expect_equal_string("tag summary string",
        yrm100_get_tag_string(&tag, tag_string),
        "EPC: 0001ABCDEF10203040506070, PC: 1234, RSSI:  -70dBm, CRC: BEEF");
    return failures;
}

static int test_tx_power_string(void)
{
    char buf[YRM100_PARAM_TX_POWER_STRING_LENGTH];
    int failures = 0;

    failures += expect_equal_string("tx power clamp low",
        yrm100_convert_to_tx_power_string(900, buf),
        "10dBm");
    failures += expect_equal_string("tx power in range",
        yrm100_convert_to_tx_power_string(2300, buf),
        "23dBm");
    failures += expect_equal_string("tx power clamp high",
        yrm100_convert_to_tx_power_string(2700, buf),
        "26dBm");
    return failures;
}

static int test_region_string(void)
{
    int failures = 0;
    failures += expect_equal_string("region china 900",
        yrm100_convert_to_region_string(YRM100_PARAM_REGION_CHINA_900),
        "China 900MHz");
    failures += expect_equal_string("region usa",
        yrm100_convert_to_region_string(YRM100_PARAM_REGION_USA),
        "USA");
    failures += expect_equal_string("region europe",
        yrm100_convert_to_region_string(YRM100_PARAM_REGION_EUROPE),
        "Europe");
    failures += expect_equal_string("region china 800",
        yrm100_convert_to_region_string(YRM100_PARAM_REGION_CHINA_800),
        "China 800MHz");
    failures += expect_equal_string("region korea",
        yrm100_convert_to_region_string(YRM100_PARAM_REGION_KOREA),
        "Korea");
    failures += expect_equal_string("region unknown",
        yrm100_convert_to_region_string(99U),
        "Unknown region");
    return failures;
}

static int test_query_string_helpers(void)
{
    int failures = 0;

    failures += expect_equal_string("divide ratio 0",
        yrm100_convert_to_divide_ratio_string(0U),
        "8");
    failures += expect_equal_string("divide ratio 1",
        yrm100_convert_to_divide_ratio_string(1U),
        "64/3");
    failures += expect_equal_string("divide ratio unknown",
        yrm100_convert_to_divide_ratio_string(2U),
        "-");

    failures += expect_equal_string("backscatter m0",
        yrm100_convert_to_backscatter_encoding_string(0U),
        "1 (FM0)");
    failures += expect_equal_string("backscatter m1",
        yrm100_convert_to_backscatter_encoding_string(1U),
        "2");
    failures += expect_equal_string("backscatter m2",
        yrm100_convert_to_backscatter_encoding_string(2U),
        "4");
    failures += expect_equal_string("backscatter m3",
        yrm100_convert_to_backscatter_encoding_string(3U),
        "8");
    failures += expect_equal_string("backscatter unknown",
        yrm100_convert_to_backscatter_encoding_string(4U),
        "-");

    failures += expect_equal_string("trext 0",
        yrm100_convert_to_trext_string(0U),
        "No pilot tone");
    failures += expect_equal_string("trext 1",
        yrm100_convert_to_trext_string(1U),
        "Pilot tone present");
    failures += expect_equal_string("trext unknown",
        yrm100_convert_to_trext_string(2U),
        "-");

    failures += expect_equal_string("sel 0",
        yrm100_convert_to_sel_string(0U),
        "All");
    failures += expect_equal_string("sel 1",
        yrm100_convert_to_sel_string(1U),
        "All");
    failures += expect_equal_string("sel 2",
        yrm100_convert_to_sel_string(2U),
        "Non-selected");
    failures += expect_equal_string("sel 3",
        yrm100_convert_to_sel_string(3U),
        "Selected");
    failures += expect_equal_string("sel unknown",
        yrm100_convert_to_sel_string(4U),
        "-");

    failures += expect_equal_string("session 0",
        yrm100_convert_to_session_string(0U),
        "S0");
    failures += expect_equal_string("session 1",
        yrm100_convert_to_session_string(1U),
        "S1");
    failures += expect_equal_string("session 2",
        yrm100_convert_to_session_string(2U),
        "S2");
    failures += expect_equal_string("session 3",
        yrm100_convert_to_session_string(3U),
        "S3");
    failures += expect_equal_string("session unknown",
        yrm100_convert_to_session_string(4U),
        "-");

    failures += expect_equal_string("target 0",
        yrm100_convert_to_target_string(0U),
        "A");
    failures += expect_equal_string("target 1",
        yrm100_convert_to_target_string(1U),
        "B");
    failures += expect_equal_string("target unknown",
        yrm100_convert_to_target_string(2U),
        "-");

    failures += expect_equal_string("q 0",
        yrm100_convert_to_q_string(0U),
        "1 slot");
    failures += expect_equal_string("q 1",
        yrm100_convert_to_q_string(1U),
        "2 slots");
    failures += expect_equal_string("q 4",
        yrm100_convert_to_q_string(4U),
        "16 slots");
    failures += expect_equal_string("q 15",
        yrm100_convert_to_q_string(15U),
        "32768 slots");
    failures += expect_equal_string("q unknown",
        yrm100_convert_to_q_string(2U),
        "-");

    return failures;
}

int test_string_functions(void)
{
    int failures = 0;
    failures += test_tag_strings();
    failures += test_tx_power_string();
    failures += test_region_string();
    failures += test_query_string_helpers();
    return failures;
}
