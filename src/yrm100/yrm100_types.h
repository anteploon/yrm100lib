#ifndef YRM100_TYPES_H
#define YRM100_TYPES_H

#include <stdbool.h>
#include <stdint.h>
#include "yrm100_serial.h"

#define YRM100_COMMAND_RESPONSE_BUFFER_SIZE 1024
#define YRM100_ASCII_BUFFER_SIZE 127
#define YRM100_TAG_EPC_BYTE_COUNT 12

#define YRM100_QUERY_SESSION_S0 0
#define YRM100_QUERY_SESSION_S1 1
#define YRM100_QUERY_SESSION_S2 2
#define YRM100_QUERY_SESSION_S3 3

#define YRM100_QUERY_TARGET_A 0
#define YRM100_QUERY_TARGET_B 1

#define YRM100_QUERY_M1 0
#define YRM100_QUERY_M2 1
#define YRM100_QUERY_M4 2
#define YRM100_QUERY_M8 3

/**
 * @brief RFID UHF tag
 * The `data` field is used for additional data read from the tag, such as TID or user memory. It is allocated and freed by the library, and the caller should not modify it directly.
 */
typedef struct yrm100_rfid_tag_t
{
    signed char rssi;
    uint16_t pc;
    uint8_t epc[YRM100_TAG_EPC_BYTE_COUNT];
    uint16_t crc;
    uint8_t *data;
    size_t data_length;
} yrm100_rfid_tag_t;

/**
 * @brief RFID UHF device context
 */
typedef struct yrm100_context_t
{
    char *serial_port_name;
    serial_port_t serial_port;
    uint8_t command_response_buf[YRM100_COMMAND_RESPONSE_BUFFER_SIZE];
    int last_error_code;
    bool is_initialized;
    bool is_multi_poll_running;
    yrm100_rfid_tag_t *multi_poll_target;
    uint16_t multi_poll_maximum_tag_count;
    uint16_t multi_poll_tag_count;
} yrm100_context_t;

/**
 * @brief RFID UHF select parameters.
 * Depending on the select mode, the select is sent before other operations.
 */
typedef struct yrm100_select_parameters_t
{
    unsigned char target;  // 3 bits
    unsigned char action;  // 3 bits
    unsigned char membank; // 2 bits
    unsigned int pointer;  // 32-bit bit pointer
    unsigned char length;  // Mask length in bits
    unsigned char mask[YRM100_TAG_EPC_BYTE_COUNT];
    unsigned char truncate;
} yrm100_select_parameters_t;

/**
 * @brief RFID UHF query parameters.
 */
typedef struct yrm100_query_parameters_t
{
    unsigned char dr;      // 1 bit
    unsigned char m;       // 2 bits
    unsigned char trext;   // 1 bit
    unsigned char sel;     // 2 bits
    unsigned char session; // 2 bits
    unsigned char target;  // 1 bit
    unsigned char q;       // 4 bits
} yrm100_query_parameters_t;

/**
 * @brief Checks if the RFID UHF device context looks like a valid one
 * @param device_context Device context
 * @return True if the device context is valid
 */
bool yrm100_is_device_context_valid(yrm100_context_t *device_context);

#endif
