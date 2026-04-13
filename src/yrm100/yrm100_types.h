#ifndef YRM100_TYPES_H
#define YRM100_TYPES_H

#include <stdbool.h>
#include <stdint.h>
#include "yrm100_serial.h"

/** Size of the internal command/response byte buffer. */
#define YRM100_COMMAND_RESPONSE_BUFFER_SIZE 1024
/** Size of the generic ASCII scratch buffer. */
#define YRM100_ASCII_BUFFER_SIZE 127
/** Number of EPC bytes stored per tag. */
#define YRM100_TAG_EPC_BYTE_COUNT 12

/** Query session `S0`. */
#define YRM100_QUERY_SESSION_S0 0
/** Query session `S1`. */
#define YRM100_QUERY_SESSION_S1 1
/** Query session `S2`. */
#define YRM100_QUERY_SESSION_S2 2
/** Query session `S3`. */
#define YRM100_QUERY_SESSION_S3 3

/** Query target `A`. */
#define YRM100_QUERY_TARGET_A 0
/** Query target `B`. */
#define YRM100_QUERY_TARGET_B 1

/** Miller sub-carrier encoding `M=1`. */
#define YRM100_QUERY_M1 0
/** Miller sub-carrier encoding `M=2`. */
#define YRM100_QUERY_M2 1
/** Miller sub-carrier encoding `M=4`. */
#define YRM100_QUERY_M4 2
/** Miller sub-carrier encoding `M=8`. */
#define YRM100_QUERY_M8 3

/**
 * @brief RFID UHF tag
 * The `data` field is used for additional data read from the tag, such as TID or user memory. It is allocated and freed by the library, and the caller should not modify it directly.
 */
typedef struct yrm100_rfid_tag_t
{
    /** Received signal strength indicator in dBm. */
    signed char rssi;
    /** Protocol control word. */
    uint16_t pc;
    /** EPC bytes returned by the reader. */
    uint8_t epc[YRM100_TAG_EPC_BYTE_COUNT];
    /** CRC word returned by the reader. */
    uint16_t crc;
    /** Optional dynamically allocated memory-bank payload. */
    uint8_t *data;
    /** Number of bytes available in `data`. */
    size_t data_length;
} yrm100_rfid_tag_t;

/**
 * @brief RFID UHF device context
 */
typedef struct yrm100_context_t
{
    /** Duplicated serial port name owned by the context. */
    char *serial_port_name;
    /** Open serial port handle. */
    serial_port_t serial_port;
    /** Internal receive buffer for commands and responses. */
    uint8_t command_response_buf[YRM100_COMMAND_RESPONSE_BUFFER_SIZE];
    /** Most recent library error code. */
    int last_error_code;
    /** True after successful initialization. */
    bool is_initialized;
    /** True while a multi-poll operation is active. */
    bool is_multi_poll_running;
    /** Caller-provided destination buffer for multi-poll tags. */
    yrm100_rfid_tag_t *multi_poll_target;
    /** Capacity of `multi_poll_target`. */
    uint16_t multi_poll_maximum_tag_count;
    /** Number of tags written during multi-poll. */
    uint16_t multi_poll_tag_count;
} yrm100_context_t;

/**
 * @brief RFID UHF select parameters.
 * Depending on the select mode, the select is sent before other operations.
 */
typedef struct yrm100_select_parameters_t
{
    /** Target flag field. */
    unsigned char target;  // 3 bits
    /** Select action field. */
    unsigned char action;  // 3 bits
    /** Memory bank used for the select mask. */
    unsigned char membank; // 2 bits
    /** Bit pointer within the selected memory bank. */
    unsigned int pointer;  // 32-bit bit pointer
    /** Select mask length in bits. */
    unsigned char length;  // Mask length in bits
    /** Select mask bytes. */
    unsigned char mask[YRM100_TAG_EPC_BYTE_COUNT];
    /** Truncate flag. */
    unsigned char truncate;
} yrm100_select_parameters_t;

/**
 * @brief RFID UHF query parameters.
 */
typedef struct yrm100_query_parameters_t
{
    /** Divide ratio flag. */
    unsigned char dr;      // 1 bit
    /** Miller encoding setting. */
    unsigned char m;       // 2 bits
    /** Pilot tone enable flag. */
    unsigned char trext;   // 1 bit
    /** Select flag. */
    unsigned char sel;     // 2 bits
    /** Query session value. */
    unsigned char session; // 2 bits
    /** Query target flag. */
    unsigned char target;  // 1 bit
    /** Query slot count exponent. */
    unsigned char q;       // 4 bits
} yrm100_query_parameters_t;

/**
 * @brief Checks if the RFID UHF device context looks like a valid one
 * @param device_context Device context
 * @return True if the device context is valid
 */
bool yrm100_is_device_context_valid(yrm100_context_t *device_context);

#endif
