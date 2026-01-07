
#ifndef YRM100_ERROR_H
#define YRM100_ERROR_H

#define YRM100_STATUS_OK 0
#define YRM100_ERROR_COMMAND_FAILED -1
#define YRM100_ERROR_INITIALIZATION_FAILED -2
#define YRM100_ERROR_NOT_INITIALIZED -3
#define YRM100_ERROR_WRITING_TO_SERIAL_PORT_FAILED -4
#define YRM100_ERROR_BUFFER_NULL -5
#define YRM100_ERROR_BUFFER_TOO_SMALL -6
#define YRM100_ERROR_SERIAL_INPUT_OVERFLOW -7
#define YRM100_ERROR_PARSE_ERROR -8
#define YRM100_ERROR_BUFFER_OVERFLOW -9
#define YRM100_ERROR_CHECKSUM_CALCULATION_FAILURE -10
#define YRM100_ERROR_UNKNOWN_REGION -11
#define YRM100_ERROR_UNKNOWN_SELECT_MODE -12
#define YRM100_ERROR_MULTI_POLL_NOT_RUNNING -13
#define YRM100_ERROR_INVALID_COMMAND -17
#define YRM100_ERROR_INVALID_DEVICE_HANDLE -18
#define YRM100_ERROR_UNKNOWN_ERROR -99

#define YRM100_MODULE_ERROR_NO_ERROR 0x00
#define YRM100_MODULE_ERROR_READ_FAIL 0x09
#define YRM100_MODULE_ERROR_WRITE_FAIL 0x10
#define YRM100_MODULE_ERROR_KILL_FAIL 0x12
#define YRM100_MODULE_ERROR_LOCK_FAIL 0x13
#define YRM100_MODULE_ERROR_INVENTORY_FAIL 0x15
#define YRM100_MODULE_ERROR_COMMAND_ERROR 0x17
#define YRM100_MODULE_ERROR_FHSS_FAIL 0x20

/**
 * @brief Converts the error code to human readable string
 * @param error_code 
 * @return Error message string
 */
const char *yrm100_error_code_to_string(int error_code);

/**
 * @brief Converts the error code to human readable string
 * @param error_code 
 * @return Error message string
 */
const char *yrm100_module_error_code_to_string(int error_code);

#endif
