#ifndef YRM100_ERROR_H
#define YRM100_ERROR_H

/** @defgroup yrm100_status_codes Library status and error codes
 *  @{
 */

/** Operation completed successfully. */
#define YRM100_STATUS_OK 0
/** Command execution failed. */
#define YRM100_ERROR_COMMAND_FAILED -1
/** Device initialization failed. */
#define YRM100_ERROR_INITIALIZATION_FAILED -2
/** Device context has not been initialized. */
#define YRM100_ERROR_NOT_INITIALIZED -3
/** Writing to the serial port failed. */
#define YRM100_ERROR_WRITING_TO_SERIAL_PORT_FAILED -4
/** A required buffer pointer was NULL. */
#define YRM100_ERROR_BUFFER_NULL -5
/** Provided buffer is too small for the operation. */
#define YRM100_ERROR_BUFFER_TOO_SMALL -6
/** Incoming serial data exceeded the command buffer. */
#define YRM100_ERROR_SERIAL_INPUT_OVERFLOW -7
/** Response parsing failed. */
#define YRM100_ERROR_PARSE_ERROR -8
/** Operation would overflow a destination buffer. */
#define YRM100_ERROR_BUFFER_OVERFLOW -9
/** Frame checksum calculation failed. */
#define YRM100_ERROR_CHECKSUM_CALCULATION_FAILURE -10
/** Region value is not recognized. */
#define YRM100_ERROR_UNKNOWN_REGION -11
/** Select mode value is not recognized. */
#define YRM100_ERROR_UNKNOWN_SELECT_MODE -12
/** Multi-poll operation is not running. */
#define YRM100_ERROR_MULTI_POLL_NOT_RUNNING -13
/** Serial read timed out. */
#define YRM100_ERROR_READ_TIMEOUT -14
/** Reading from the serial port failed. */
#define YRM100_ERROR_READING_FROM_SERIAL_PORT_FAILED -15
/** Opening the serial port failed. */
#define YRM100_ERROR_SERIAL_PORT_OPEN_FAILED -16
/** Command identifier is invalid. */
#define YRM100_ERROR_INVALID_COMMAND -17
/** Device handle is invalid. */
#define YRM100_ERROR_INVALID_DEVICE_HANDLE -18
/** POSIX `tcgetattr` call failed. */
#define YRM100_ERROR_SERIAL_TCGETATTR_FAILED -19
/** POSIX `tcsetattr` call failed. */
#define YRM100_ERROR_SERIAL_TCSETATTR_FAILED -20
/** Windows `GetCommState` call failed. */
#define YRM100_ERROR_SERIAL_GETCOMMSTATE_FAILED -21
/** Windows `SetCommState` call failed. */
#define YRM100_ERROR_SERIAL_SETCOMMSTATE_FAILED -22
/** Windows `SetCommTimeouts` call failed. */
#define YRM100_ERROR_SERIAL_SETCOMMTIMEOUTS_FAILED -23
/** Memory allocation failed. */
#define YRM100_ERROR_MEMORY_ALLOCATION_FAILURE -24
/** Requested data length is invalid. */
#define YRM100_ERROR_INVALID_DATA_LENGTH -25
/** Memory bank value is not recognized. */
#define YRM100_ERROR_UNKNOWN_MEMORY_BANK -26

/** Feature is not implemented. */
#define YRM100_ERROR_NOT_IMPLEMENTED -98
/** Unspecified error fallback. */
#define YRM100_ERROR_UNKNOWN_ERROR -99

/** @} */

/** @defgroup yrm100_module_error_codes Device-reported module error codes
 *  @{
 */

/** No module error was reported. */
#define YRM100_MODULE_ERROR_NO_ERROR 0x00
/** Tag read operation failed on the module. */
#define YRM100_MODULE_ERROR_READ_FAIL 0x09
/** Tag write operation failed on the module. */
#define YRM100_MODULE_ERROR_WRITE_FAIL 0x10
/** Tag kill operation failed on the module. */
#define YRM100_MODULE_ERROR_KILL_FAIL 0x12
/** Tag lock operation failed on the module. */
#define YRM100_MODULE_ERROR_LOCK_FAIL 0x13
/** Inventory command failed on the module. */
#define YRM100_MODULE_ERROR_INVENTORY_FAIL 0x15
/** Command was rejected by the module. */
#define YRM100_MODULE_ERROR_COMMAND_ERROR 0x17
/** Frequency hopping operation failed on the module. */
#define YRM100_MODULE_ERROR_FHSS_FAIL 0x20

/** @} */

/**
 * @brief Converts a library error code to a human-readable string
 * @param error_code Library error code
 * @return Error message string
 */
const char *yrm100_error_code_to_string(int error_code);

/**
 * @brief Converts a module error code to a human-readable string
 * @param error_code Module error code
 * @return Module error message string
 */
const char *yrm100_module_error_code_to_string(int error_code);

#endif
