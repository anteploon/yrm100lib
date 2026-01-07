#ifndef YRM100_TYPES_H
#define YRM100_TYPES_H

#include "yrm100_serial.h"

#define YRM100_COMMAND_RESPONSE_BUFFER_SIZE 1024
#define YRM100_ASCII_BUFFER_SIZE 127
#define YRM100_TAG_EPC_BYTE_COUNT 12

/**
 * @brief RFID UHF tag
 */
typedef struct rfid_tag_t
{
    signed char rssi;
    unsigned short pc;
    unsigned char epc[YRM100_TAG_EPC_BYTE_COUNT];
    unsigned short crc;
} rfid_tag_t;

/**
 * @brief RFID UHF device context
 */
typedef struct yrm100_context_t
{
    char *serial_port_name;
    serial_port_t serial_port;
    unsigned char command_response_buf[YRM100_COMMAND_RESPONSE_BUFFER_SIZE];
    int last_error_code;
    bool is_initialized;
    bool is_multi_poll_running;
    rfid_tag_t *multi_poll_target;    
    unsigned short multi_poll_maximum_tag_count;
    unsigned short multi_poll_tag_count;
} yrm100_context_t;

/**
 * @brief RFID UHF select parameters.
 * Depending on the select mode, the select is sent prior other operations.
 */
typedef struct rfid_select_parameters_t
{
    unsigned char target;   // 3 bits
    unsigned char action;   // 3 bits
    unsigned char membank;  // 2 bits
    unsigned short pointer;
    unsigned short length;    
    unsigned char mask[YRM100_TAG_EPC_BYTE_COUNT];
    unsigned char truncate;    
} rfid_select_parameters_t;

#endif