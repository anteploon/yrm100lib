#include <stdio.h>
#include "yrm100_command.h"
#include "yrm100_util.h"
#include "yrm100_string.h"

char *yrm100_get_tag_pc_string(rfid_tag_t *tag, char string_buf[YRM100_TAG_PC_STRING_LENGTH])
{
    snprintf(string_buf, YRM100_TAG_PC_STRING_LENGTH, "%04X", tag->pc);
    return string_buf;
}

char *yrm100_get_tag_crc_string(rfid_tag_t *tag, char string_buf[YRM100_TAG_CRC_STRING_LENGTH])
{
    snprintf(string_buf, YRM100_TAG_CRC_STRING_LENGTH, "%04X", tag->crc);
    return string_buf;
}

char *yrm100_get_tag_epc_string(rfid_tag_t *tag, char string_buf[YRM100_TAG_EPC_STRING_LENGTH])
{
    char hex_buf[3];

    yrm100_zero_buf(string_buf, YRM100_TAG_EPC_STRING_LENGTH);
    for (short i = 0; i < YRM100_TAG_EPC_BYTE_COUNT; i++)
    {
        snprintf(hex_buf, 3, "%X", tag->epc[i] >> 4);
        string_buf[i * 2] = hex_buf[0];
        snprintf(hex_buf, 3, "%X", tag->epc[i] & 0x0F);
        string_buf[(i * 2) + 1] = hex_buf[0];
    }
    return string_buf;
}

char *yrm100_get_tag_rssi_string(rfid_tag_t *tag, char buf[YRM100_TAG_RSSI_STRING_LENGTH])
{
    snprintf(buf, YRM100_TAG_RSSI_STRING_LENGTH, "%4idBm", tag->rssi);
    return buf;
}

char *yrm100_get_tag_string(rfid_tag_t *tag, char string_buf[YRM100_TAG_STRING_LENGTH])
{
    char pc[YRM100_TAG_PC_STRING_LENGTH];
    char crc[YRM100_TAG_CRC_STRING_LENGTH];
    char epc[YRM100_TAG_EPC_STRING_LENGTH];
    char rssi[YRM100_TAG_RSSI_STRING_LENGTH];

    yrm100_zero_buf(string_buf, YRM100_TAG_STRING_LENGTH);

    yrm100_get_tag_epc_string(tag, epc);
    yrm100_get_tag_pc_string(tag, pc);
    yrm100_get_tag_crc_string(tag, crc);
    yrm100_get_tag_rssi_string(tag, rssi);

    snprintf(string_buf, YRM100_TAG_STRING_LENGTH, "EPC: %s, PC: %s, RSSI: %s, CRC: %s", epc, pc, rssi, crc);

    return string_buf;
}

char *yrm100_convert_to_tx_power_string(unsigned int power, char string_buf[YRM100_PARAM_TX_POWER_STRING_LENGTH])
{
    unsigned int value;
    if (power < 1000)
    {
        value = 1000;
    }
    else if (power > 2600)
    {
        value = 2600;
    }
    else
    {
        value = power;
    }
    value = value / 100;
    snprintf(string_buf, YRM100_PARAM_TX_POWER_STRING_LENGTH, "%idBm", value);
    return string_buf;
}

const char *yrm100_convert_to_region_string(unsigned int region_code)
{
    switch (region_code)
    {
    case YRM100_PARAM_REGION_CHINA_900:
        return "China 900MHz";
    case YRM100_PARAM_REGION_USA:
        return "USA";
    case YRM100_PARAM_REGION_EUROPE:
        return "Europe";
    case YRM100_PARAM_REGION_CHINA_800:
        return "China 800MHz";
    case YRM100_PARAM_REGION_KOREA:
        return "Korea";
    }
    return "Unknown region";
}
