#include "yrm100_error.h"

const char *yrm100_error_code_to_string(int error_code)
{
    switch (error_code)
    {
    case YRM100_STATUS_OK:
        return "OK";
    case YRM100_ERROR_COMMAND_FAILED:
        return "Command failed";
    case YRM100_ERROR_INITIALIZATION_FAILED:
        return "Device initialization failed";
    case YRM100_ERROR_NOT_INITIALIZED:
        return "Device is not initialized";
    case YRM100_ERROR_WRITING_TO_SERIAL_PORT_FAILED:
        return "Writing to serial port failed";
    case YRM100_ERROR_BUFFER_NULL:
        return "Buffer pointer is NULL";
    case YRM100_ERROR_BUFFER_TOO_SMALL:
        return "Buffer is too small";
    case YRM100_ERROR_SERIAL_INPUT_OVERFLOW:
        return "Serial input overflow";
    case YRM100_ERROR_PARSE_ERROR:
        return "Parse error";
    case YRM100_ERROR_BUFFER_OVERFLOW:
        return "Buffer overflow";
    case YRM100_ERROR_CHECKSUM_CALCULATION_FAILURE:
        return "Checksum calculation failure";
    case YRM100_ERROR_UNKNOWN_REGION:
        return "Unknown region value specified";
    case YRM100_ERROR_UNKNOWN_SELECT_MODE:
        return "Unknown select mode specified";
    case YRM100_ERROR_MULTI_POLL_NOT_RUNNING:
        return "Multi poll is not running";
    case YRM100_ERROR_READ_TIMEOUT:
        return "Read timeout";
    case YRM100_ERROR_READING_FROM_SERIAL_PORT_FAILED:
        return "Reading from serial port failed";
    case YRM100_ERROR_SERIAL_PORT_OPEN_FAILED:
        return "Serial port open failed";
    case YRM100_ERROR_INVALID_COMMAND:
        return "Command byte sequence invalid";
    case YRM100_ERROR_INVALID_DEVICE_HANDLE:
        return "Invalid device handle";
    case YRM100_ERROR_SERIAL_TCGETATTR_FAILED:
        return "Serial port call to tcgetattr() failed";
    case YRM100_ERROR_SERIAL_TCSETATTR_FAILED:
        return "Serial port call to tcsetattr() failed";
    case YRM100_ERROR_SERIAL_GETCOMMSTATE_FAILED:
        return "Serial port GetCommState() failed";
    case YRM100_ERROR_SERIAL_SETCOMMSTATE_FAILED:
        return "Serial port call to SetCommState() failed";
    case YRM100_ERROR_SERIAL_SETCOMMTIMEOUTS_FAILED:
        return "Serial port call to SetCommTimeouts() failed";
    default:
        return "Unknown error";
    }
}

const char *yrm100_module_error_code_to_string(int error_code)
{
    switch (error_code)
    {
    case YRM100_MODULE_ERROR_NO_ERROR:
        return "OK";
    case YRM100_MODULE_ERROR_READ_FAIL:
        return "Tag data memory read failed";
    case YRM100_MODULE_ERROR_WRITE_FAIL:
        return "Tag data memory write failed";
    case YRM100_MODULE_ERROR_KILL_FAIL:
        return "Failed to kill the tag";
    case YRM100_MODULE_ERROR_LOCK_FAIL:
        return "Failed to lock the tag";
    case YRM100_MODULE_ERROR_INVENTORY_FAIL:
        return "Polling operation failed - no tag found";
    case YRM100_MODULE_ERROR_COMMAND_ERROR:
        return "Command error - parameters or CRC not valid";
    case YRM100_MODULE_ERROR_FHSS_FAIL:
        return "Frequency-hopping channel search timed out";
    default:
        return "Unknown error";
    }
}
