#ifndef YRM100_STRING_H
#define YRM100_STRING_H

#include <string.h>
#include <sys/types.h>
#include "yrm100.h"
#include "yrm100_serial.h"

#define YRM100_MODULE_INFO_STRING_LENGTH 255
#define YRM100_PARAM_TX_POWER_STRING_LENGTH 6
#define YRM100_TAG_STRING_LENGTH 66
#define YRM100_TAG_CRC_STRING_LENGTH 5
#define YRM100_TAG_PC_STRING_LENGTH 5
#define YRM100_TAG_RSSI_STRING_LENGTH 8
#define YRM100_TAG_EPC_STRING_LENGTH 25
#define YRM100_MODULE_REGION_STRING_LENGTH 100

/**
 * @brief Formats the PC part of the RFID tag as a hexadecimal string
 * @param tag Pointer to RFID tag
 * @param string_buf Pointer to string buffer
 * @return Pointer to the string_buf 
 */
char *yrm100_get_tag_pc_string(rfid_tag_t *tag, char string_buf[YRM100_TAG_PC_STRING_LENGTH]);

/**
 * @brief Formats the CRC part of the RFID tag as a hexadecimal string
 * @param tag Pointer to RFID tag
 * @param string_buf Pointer to string buffer
 * @return Pointer to the string_buf
 */
char *yrm100_get_tag_crc_string(rfid_tag_t *tag, char string_buf[YRM100_TAG_CRC_STRING_LENGTH]);

/**
 * @brief Formats the EPC part of the RFID tag as a hexadecimal string
 * @param tag Pointer to RFID tag
 * @param string_buf Pointer to string buffer
 * @return Pointer to the string_buf
 */
char *yrm100_get_tag_epc_string(rfid_tag_t *tag, char string_buf[YRM100_TAG_EPC_STRING_LENGTH]);

/**
 * @brief Formats the RSSI part of the RFID tag as a string with units
 * @param tag Pointer to RFID tag
 * @param buf Pointer to string buffer
 * @return Pointer to the buf
 */
char *yrm100_get_tag_rssi_string(rfid_tag_t *tag, char buf[YRM100_TAG_RSSI_STRING_LENGTH]);

/**
 * @brief Formats the RFID tag details as a single descriptive string
 * @param tag Pointer to RFID tag
 * @param string_buf Pointer to string buffer
 * @return Pointer to the string_buf
 */
char *yrm100_get_tag_string(rfid_tag_t *tag, char string_buf[YRM100_TAG_STRING_LENGTH]);

/**
 * @brief Formats the transmit power value as a string, clamped to device limits
 * @param power Transmit power in centi-dBm
 * @param string_buf Pointer to string buffer
 * @return Pointer to the string_buf
 */
char *yrm100_convert_to_tx_power_string(unsigned int power, char string_buf[YRM100_PARAM_TX_POWER_STRING_LENGTH]);

/**
 * @brief Converts a region code to a human-readable string
 * @param region_code Region code value
 * @return Pointer to a region string
 */
const char *yrm100_convert_to_region_string(unsigned int region_code);

#endif
