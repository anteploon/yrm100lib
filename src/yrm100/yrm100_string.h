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

#define EMOJI_FAIL "✗"
#define EMOJI_PASS "✓"

/**
 * @brief Formats the PC part of the RFID tag as a hexadecimal string
 * @param tag Pointer to RFID tag
 * @param string_buf Pointer to string buffer
 * @return Pointer to string_buf
 */
char *yrm100_get_tag_pc_string(yrm100_rfid_tag_t *tag, char string_buf[YRM100_TAG_PC_STRING_LENGTH]);

/**
 * @brief Formats the CRC part of the RFID tag as a hexadecimal string
 * @param tag Pointer to RFID tag
 * @param string_buf Pointer to string buffer
 * @return Pointer to string_buf
 */
char *yrm100_get_tag_crc_string(yrm100_rfid_tag_t *tag, char string_buf[YRM100_TAG_CRC_STRING_LENGTH]);

/**
 * @brief Formats the EPC part of the RFID tag as a hexadecimal string
 * @param tag Pointer to RFID tag
 * @param string_buf Pointer to string buffer
 * @return Pointer to string_buf
 */
char *yrm100_get_tag_epc_string(yrm100_rfid_tag_t *tag, char string_buf[YRM100_TAG_EPC_STRING_LENGTH]);

/**
 * @brief Formats the RSSI part of the RFID tag as a string with units
 * @param tag Pointer to RFID tag
 * @param buf Pointer to string buffer
 * @return Pointer to buf
 */
char *yrm100_get_tag_rssi_string(yrm100_rfid_tag_t *tag, char buf[YRM100_TAG_RSSI_STRING_LENGTH]);

/**
 * @brief Formats the RFID tag details as a single descriptive string
 * @param tag Pointer to RFID tag
 * @param string_buf Pointer to string buffer
 * @return Pointer to string_buf
 */
char *yrm100_get_tag_string(yrm100_rfid_tag_t *tag, char string_buf[YRM100_TAG_STRING_LENGTH]);

/**
 * @brief Formats the transmit power value as a string, clamped to device limits
 * @param power Transmit power in centi-dBm
 * @param string_buf Pointer to string buffer
 * @return Pointer to string_buf
 */
char *yrm100_convert_to_tx_power_string(unsigned int power, char string_buf[YRM100_PARAM_TX_POWER_STRING_LENGTH]);

/**
 * @brief Converts a region code to a human-readable string
 * @param region Region code value
 * @return Pointer to a region string
 */
const char *yrm100_convert_to_region_string(uint8_t region);

/**
 * @brief Converts a divide ratio value to a human-readable string
 * @param dr Divide ratio value
 * @return Pointer to a divide ratio string
 */
const char *yrm100_convert_to_divide_ratio_string(uint8_t dr);

/**
 * @brief Converts a backscatter encoding value to a human-readable string
 * @param m Backscatter encoding value
 * @return Pointer to a backscatter encoding string
 */
const char *yrm100_convert_to_backscatter_encoding_string(uint8_t m);

/**
 * @brief Converts a TREXT value to a human-readable string
 * @param trext TREXT value
 * @return Pointer to a TREXT string
 */
const char *yrm100_convert_to_trext_string(uint8_t trext);

/**
 * @brief Converts a SEL value to a human-readable string
 * @param sel SEL value
 * @return Pointer to a SEL string
 */
const char *yrm100_convert_to_sel_string(uint8_t sel);

/**
 * @brief Converts a session value to a human-readable string
 * @param session Session value
 * @return Pointer to a session string
 */
const char *yrm100_convert_to_session_string(uint8_t session);

/**
 * @brief Converts a target value to a human-readable string
 * @param target Target value
 * @return Pointer to a target string
 */
const char *yrm100_convert_to_target_string(uint8_t target);

/**
 * @brief Converts a Q value to a human-readable string
 * @param q Q value
 * @return Pointer to a Q string
 */
const char *yrm100_convert_to_q_string(uint8_t q);

#endif
