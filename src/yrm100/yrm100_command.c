#include <stdio.h>
#include <string.h>
#include "yrm100.h"
#include "yrm100_types.h"
#include "yrm100_util.h"
#include "yrm100_frame.h"
#include "yrm100_param.h"
#include "yrm100_serial.h"
#include "yrm100_parse.h"
#include "yrm100_command.h"
#include "yrm100_error.h"

#define YRM100_COMMAND_FRAME_PREFIX_SIZE 5
#define YRM100_COMMAND_FRAME_OVERHEAD_SIZE 7

int yrm100_set_last_error_code(yrm100_context_t *device_context, ssize_t error_code)
{
    if (device_context != NULL)
    {
        device_context->last_error_code = (int)error_code;
    }
    return (int)error_code;
}

static int yrm100_command_send(yrm100_context_t *device_context, uint8_t *cmd, size_t cmd_size)
{
    if (yrm100_is_device_context_valid(device_context) == false)
    {
        return yrm100_set_last_error_code(device_context, YRM100_ERROR_INVALID_DEVICE_HANDLE);
    }

#ifdef YRM100_COMM_DEBUG
    printf("TX: ");
    for (size_t i = 0; i < cmd_size; i++)
    {
        printf("%02X ", cmd[i]);
    }
    printf("\n");
#endif

    if (yrm100_frame_is_valid_command(cmd, cmd_size))
    {
        size_t total_written = 0;
        while (total_written < cmd_size)
        {
            ssize_t n = yrm100_serial_write(device_context->serial_port, &cmd[total_written], cmd_size - total_written);
            if (n < 0)
            {
                perror("Error writing to serial port");
                return yrm100_set_last_error_code(device_context, YRM100_ERROR_WRITING_TO_SERIAL_PORT_FAILED);
            }
            if (n == 0)
            {
                return yrm100_set_last_error_code(device_context, YRM100_ERROR_WRITING_TO_SERIAL_PORT_FAILED);
            }
            total_written += (size_t)n;
        }
        yrm100_sleep_usec(YRM100_COMMAND_RESPONSE_DELAY_USEC);
    }
    else
    {
        return yrm100_set_last_error_code(device_context, YRM100_ERROR_INVALID_COMMAND);
    }
    return yrm100_set_last_error_code(device_context, YRM100_STATUS_OK);
}

static bool yrm100_command_frame_type_is_supported(uint8_t frame_type)
{
    return frame_type == YRM100_FRAME_TYPE_BYTE_RESPONSE ||
           frame_type == YRM100_FRAME_TYPE_BYTE_NOTICE;
}

static int yrm100_command_frame_expected_size(uint8_t *buf, size_t buf_size, size_t *expected_size)
{
    if (buf == NULL || expected_size == NULL)
    {
        return YRM100_ERROR_BUFFER_NULL;
    }
    if (buf_size < YRM100_COMMAND_FRAME_PREFIX_SIZE)
    {
        return YRM100_ERROR_PARSE_ERROR;
    }
    if (buf[YRM100_FRAME_BYTE_POSITION_HEADER] != YRM100_FRAME_HEADER_BYTE ||
        yrm100_command_frame_type_is_supported(buf[YRM100_FRAME_BYTE_POSITION_TYPE]) == false)
    {
        return YRM100_ERROR_PARSE_ERROR;
    }

    size_t payload_len = ((size_t)buf[3] << 8) | (size_t)buf[4];
    *expected_size = payload_len + YRM100_COMMAND_FRAME_OVERHEAD_SIZE;
    return YRM100_STATUS_OK;
}

static int yrm100_command_validate_complete_frame(uint8_t *buf, size_t frame_size)
{
    int checksum;

    if (buf == NULL)
    {
        return YRM100_ERROR_BUFFER_NULL;
    }
    if (frame_size < YRM100_FRAME_MINIMUM_RESPONSE_SIZE)
    {
        return YRM100_ERROR_PARSE_ERROR;
    }
    if (buf[frame_size - 1] != YRM100_FRAME_END_BYTE)
    {
        return YRM100_ERROR_PARSE_ERROR;
    }
    checksum = yrm100_frame_calculate_checksum(buf, frame_size);
    if (checksum < 0)
    {
        return YRM100_ERROR_CHECKSUM_CALCULATION_FAILURE;
    }
    if (buf[frame_size - 2] != (uint8_t)checksum)
    {
        return YRM100_ERROR_PARSE_ERROR;
    }
    return YRM100_STATUS_OK;
}

ssize_t yrm100_command_read_response(yrm100_context_t *device_context)
{
    if (yrm100_is_device_context_valid(device_context) == false)
    {
        return yrm100_set_last_error_code(device_context, YRM100_ERROR_INVALID_DEVICE_HANDLE);
    }

    uint8_t *buf;
    size_t buf_size = sizeof(device_context->command_response_buf);
    size_t cursor = 0;
    size_t total_read = 0;
    size_t frame_start = 0;
    size_t expected_frame_size = 0;
    size_t expected_total = 0;
    ssize_t response_len;

    while (true)
    {
        size_t read_size;

        if (cursor >= buf_size)
        {
            return yrm100_set_last_error_code(device_context, YRM100_ERROR_SERIAL_INPUT_OVERFLOW);
        }

        if (expected_total > 0)
        {
            read_size = expected_total - cursor;
        }
        else if (cursor - frame_start < YRM100_COMMAND_FRAME_PREFIX_SIZE)
        {
            read_size = YRM100_COMMAND_FRAME_PREFIX_SIZE - (cursor - frame_start);
        }
        else
        {
            int result = yrm100_command_frame_expected_size(&device_context->command_response_buf[frame_start], cursor - frame_start, &expected_frame_size);
            if (result != YRM100_STATUS_OK)
            {
                return yrm100_set_last_error_code(device_context, result);
            }
            expected_total = frame_start + expected_frame_size;
            if (expected_total > buf_size)
            {
                return yrm100_set_last_error_code(device_context, YRM100_ERROR_SERIAL_INPUT_OVERFLOW);
            }
            read_size = expected_total - cursor;
        }

        if (read_size == 0)
        {
            int result = yrm100_command_validate_complete_frame(&device_context->command_response_buf[frame_start], expected_frame_size);
            if (result != YRM100_STATUS_OK)
            {
                return yrm100_set_last_error_code(device_context, result);
            }
            if (device_context->command_response_buf[frame_start + YRM100_FRAME_BYTE_POSITION_TYPE] == YRM100_FRAME_TYPE_BYTE_RESPONSE)
            {
                break;
            }

            frame_start = cursor;
            expected_frame_size = 0;
            expected_total = 0;
            continue;
        }

        buf = &device_context->command_response_buf[cursor];
        response_len = yrm100_serial_read(device_context->serial_port, buf, read_size);
        if (response_len < 0)
        {
            return yrm100_set_last_error_code(device_context, YRM100_ERROR_READING_FROM_SERIAL_PORT_FAILED);
        }
        if (response_len == 0)
        {
            if (total_read == 0)
            {
                return yrm100_set_last_error_code(device_context, YRM100_ERROR_READ_TIMEOUT);
            }
            if (cursor - frame_start < YRM100_COMMAND_FRAME_PREFIX_SIZE || (expected_total > 0 && cursor < expected_total))
            {
                return yrm100_set_last_error_code(device_context, YRM100_ERROR_PARSE_ERROR);
            }
            break;
        }
        cursor += (size_t)response_len;
        total_read += (size_t)response_len;

        while (frame_start < cursor &&
               device_context->command_response_buf[frame_start] != YRM100_FRAME_HEADER_BYTE)
        {
            frame_start++;
        }
        if (frame_start > 0)
        {
            if (device_context->command_response_buf[0] == YRM100_FRAME_HEADER_BYTE)
            {
                expected_frame_size = 0;
                expected_total = 0;
            }
            else if (frame_start == cursor)
            {
                cursor = 0;
                frame_start = 0;
            }
            else
            {
                memmove(device_context->command_response_buf, &device_context->command_response_buf[frame_start], cursor - frame_start);
                cursor -= frame_start;
                total_read = cursor;
                frame_start = 0;
            }
            if (device_context->command_response_buf[0] != YRM100_FRAME_HEADER_BYTE)
            {
                expected_frame_size = 0;
                expected_total = 0;
            }
        }

        if (cursor - frame_start >= 2 &&
            yrm100_command_frame_type_is_supported(device_context->command_response_buf[frame_start + YRM100_FRAME_BYTE_POSITION_TYPE]) == false)
        {
            return yrm100_set_last_error_code(device_context, YRM100_ERROR_PARSE_ERROR);
        }
    }

#ifdef YRM100_COMM_DEBUG
    printf("RX: ");
    for (size_t i = 0; i < total_read; i++)
    {
        printf("%02X ", device_context->command_response_buf[i]);
    }
    printf("\n");
#endif

    if (total_read > 0 && device_context->command_response_buf[total_read - 1] != YRM100_FRAME_END_BYTE)
    {
        return yrm100_set_last_error_code(device_context, YRM100_ERROR_PARSE_ERROR);
    }
    yrm100_set_last_error_code(device_context, YRM100_STATUS_OK);
    return (ssize_t)total_read;
}

uint8_t yrm100_pack_select_parameters(yrm100_select_parameters_t *data)
{
    return (uint8_t)((data->target & 0x07) | ((data->action & 0x07) << 3) | ((data->membank & 0x03) << 6));
}

unsigned short yrm100_pack_query_parameters(yrm100_query_parameters_t *data)
{
    if (data == NULL)
    {
        return 0;
    }
    return (unsigned short)((data->dr & 0x01) |
                            ((data->m & 0x03) << 1) |
                            ((data->trext & 0x01) << 3) |
                            ((data->sel & 0x03) << 4) |
                            ((data->session & 0x03) << 6) |
                            ((data->target & 0x01) << 8) |
                            ((data->q & 0x0F) << 9));
}

void unpack_query_parameters(unsigned short packed, yrm100_query_parameters_t *data)
{
    if (data == NULL)
    {
        return;
    }
    data->dr = (uint8_t)(packed & 0x01);
    data->m = (uint8_t)((packed >> 1) & 0x03);
    data->trext = (uint8_t)((packed >> 3) & 0x01);
    data->sel = (uint8_t)((packed >> 4) & 0x03);
    data->session = (uint8_t)((packed >> 6) & 0x03);
    data->target = (uint8_t)((packed >> 8) & 0x01);
    data->q = (uint8_t)((packed >> 9) & 0x0F);
}

int yrm100_command_get_module_manufacturer(yrm100_context_t *device_context, char *string_buf, size_t string_buf_size)
{
    uint8_t bytes[] = {0xBB, 0x00, 0x03, 0x00, 0x01, 0x02, 0x06, 0x7E};

    if (yrm100_is_device_context_valid(device_context) == false)
    {
        return yrm100_set_last_error_code(device_context, YRM100_ERROR_INVALID_DEVICE_HANDLE);
    }
    if (string_buf == NULL)
    {
        return yrm100_set_last_error_code(device_context, YRM100_ERROR_BUFFER_NULL);
    }
    if (string_buf_size == 0)
    {
        return yrm100_set_last_error_code(device_context, YRM100_ERROR_BUFFER_TOO_SMALL);
    }
    yrm100_zero_buf(string_buf, string_buf_size);

    if (yrm100_command_send(device_context, bytes, sizeof(bytes)) == YRM100_STATUS_OK)
    {
        ssize_t response_len = yrm100_command_read_response(device_context);
        if (response_len < 0)
        {
            return yrm100_set_last_error_code(device_context, response_len);
        }
        if (yrm100_frame_is_ok_response(device_context->command_response_buf, (size_t)response_len))
        {
            int parse_result = yrm100_parse_ascii_response(device_context->command_response_buf, (size_t)response_len, string_buf, string_buf_size);
            if (parse_result != YRM100_STATUS_OK)
            {
                return yrm100_set_last_error_code(device_context, parse_result);
            }
            return yrm100_set_last_error_code(device_context, YRM100_STATUS_OK);
        }
        if (yrm100_frame_is_error_response(device_context->command_response_buf, (size_t)response_len))
        {
            return yrm100_set_last_error_code(device_context, yrm100_parse_get_error_code(device_context->command_response_buf, (size_t)response_len));
        }
        return yrm100_set_last_error_code(device_context, YRM100_ERROR_COMMAND_FAILED);
    }
    return yrm100_set_last_error_code(device_context, YRM100_ERROR_UNKNOWN_ERROR);
}

int yrm100_command_get_module_hardware_version(yrm100_context_t *device_context, char *string_buf, size_t string_buf_size)
{
    uint8_t bytes[] = {0xBB, 0x00, 0x03, 0x00, 0x01, 0x00, 0x04, 0x7E};

    if (yrm100_is_device_context_valid(device_context) == false)
    {
        return yrm100_set_last_error_code(device_context, YRM100_ERROR_INVALID_DEVICE_HANDLE);
    }
    if (string_buf == NULL)
    {
        return yrm100_set_last_error_code(device_context, YRM100_ERROR_BUFFER_NULL);
    }
    if (string_buf_size == 0)
    {
        return yrm100_set_last_error_code(device_context, YRM100_ERROR_BUFFER_TOO_SMALL);
    }
    yrm100_zero_buf(string_buf, string_buf_size);

    if (yrm100_command_send(device_context, bytes, sizeof(bytes)) == YRM100_STATUS_OK)
    {
        ssize_t result = yrm100_command_read_response(device_context);
        if (result < 0)
        {
            return yrm100_set_last_error_code(device_context, result);
        }
        if (result == 0)
        {
            return yrm100_set_last_error_code(device_context, YRM100_ERROR_READ_TIMEOUT);
        }
        if (yrm100_frame_is_ok_response(device_context->command_response_buf, (size_t)result))
        {
            int parse_result = yrm100_parse_ascii_response(device_context->command_response_buf, (size_t)result, string_buf, string_buf_size);
            if (parse_result != YRM100_STATUS_OK)
            {
                return yrm100_set_last_error_code(device_context, parse_result);
            }
            return yrm100_set_last_error_code(device_context, YRM100_STATUS_OK);
        }
        if (yrm100_frame_is_error_response(device_context->command_response_buf, (size_t)result))
        {
            return yrm100_set_last_error_code(device_context, yrm100_parse_get_error_code(device_context->command_response_buf, (size_t)result));
        }
        return yrm100_set_last_error_code(device_context, YRM100_ERROR_COMMAND_FAILED);
    }
    return yrm100_set_last_error_code(device_context, YRM100_ERROR_UNKNOWN_ERROR);
}

int yrm100_command_get_module_software_version(yrm100_context_t *device_context, char *string_buf, size_t string_buf_size)
{
    uint8_t bytes[] = {0xBB, 0x00, 0x03, 0x00, 0x01, 0x01, 0x05, 0x7E};

    if (yrm100_is_device_context_valid(device_context) == false)
    {
        return yrm100_set_last_error_code(device_context, YRM100_ERROR_INVALID_DEVICE_HANDLE);
    }
    if (string_buf == NULL)
    {
        return yrm100_set_last_error_code(device_context, YRM100_ERROR_BUFFER_NULL);
    }
    if (string_buf_size == 0)
    {
        return yrm100_set_last_error_code(device_context, YRM100_ERROR_BUFFER_TOO_SMALL);
    }
    yrm100_zero_buf(string_buf, string_buf_size);

    if (yrm100_command_send(device_context, bytes, sizeof(bytes)) == YRM100_STATUS_OK)
    {
        ssize_t result = yrm100_command_read_response(device_context);
        if (result < 0)
        {
            return yrm100_set_last_error_code(device_context, result);
        }
        if (result == 0)
        {
            return yrm100_set_last_error_code(device_context, YRM100_ERROR_READ_TIMEOUT);
        }
        if (yrm100_frame_is_ok_response(device_context->command_response_buf, (size_t)result))
        {
            int parse_result = yrm100_parse_ascii_response(device_context->command_response_buf, (size_t)result, string_buf, string_buf_size);
            if (parse_result != YRM100_STATUS_OK)
            {
                return yrm100_set_last_error_code(device_context, parse_result);
            }
            return yrm100_set_last_error_code(device_context, YRM100_STATUS_OK);
        }
        if (yrm100_frame_is_error_response(device_context->command_response_buf, (size_t)result))
        {
            return yrm100_set_last_error_code(device_context, yrm100_parse_get_error_code(device_context->command_response_buf, (size_t)result));
        }
        return yrm100_set_last_error_code(device_context, YRM100_ERROR_COMMAND_FAILED);
    }
    return yrm100_set_last_error_code(device_context, YRM100_ERROR_UNKNOWN_ERROR);
}

int yrm100_command_single_poll(yrm100_context_t *device_context, yrm100_rfid_tag_t *tags, unsigned short maximum_tag_count)
{
    uint8_t bytes[] = {0xBB, 0x00, 0x22, 0x00, 0x00, 0x22, 0x7E};
    size_t notice_len = 0;

    if (yrm100_is_device_context_valid(device_context) == false)
    {
        return yrm100_set_last_error_code(device_context, YRM100_ERROR_INVALID_DEVICE_HANDLE);
    }
    int checksum = yrm100_frame_calculate_checksum(bytes, sizeof(bytes));
    if (checksum < 0)
    {
        return yrm100_set_last_error_code(device_context, YRM100_ERROR_CHECKSUM_CALCULATION_FAILURE);
    }
    bytes[sizeof(bytes) - 2] = (uint8_t)checksum;
    if (yrm100_command_send(device_context, bytes, sizeof(bytes)) == YRM100_STATUS_OK)
    {
        ssize_t response_len = yrm100_command_read_response(device_context);
        if (response_len < 0)
        {
            return yrm100_set_last_error_code(device_context, response_len);
        }
        while (notice_len + YRM100_FRAME_POLL_NOTICE_SIZE <= (size_t)response_len &&
               yrm100_frame_is_valid_notice(&device_context->command_response_buf[notice_len], YRM100_FRAME_POLL_NOTICE_SIZE))
        {
            notice_len += YRM100_FRAME_POLL_NOTICE_SIZE;
        }
        if (notice_len < (size_t)response_len)
        {
            size_t trailing_len = (size_t)response_len - notice_len;
            uint8_t *trailing_frame = &device_context->command_response_buf[notice_len];

            if (yrm100_frame_is_error_response(trailing_frame, trailing_len))
            {
                return yrm100_set_last_error_code(device_context, yrm100_parse_get_error_code(trailing_frame, trailing_len));
            }
            if (yrm100_frame_is_ok_response(trailing_frame, trailing_len) == false)
            {
                return yrm100_set_last_error_code(device_context, YRM100_ERROR_PARSE_ERROR);
            }
        }
        else if (yrm100_frame_is_error_response(device_context->command_response_buf, (size_t)response_len))
        {
            return yrm100_set_last_error_code(device_context, yrm100_parse_get_error_code(device_context->command_response_buf, (size_t)response_len));
        }
        if (notice_len > 0)
        {
            int parse_result = yrm100_parse_poll_response(device_context->command_response_buf, notice_len, tags, maximum_tag_count);
            if (parse_result >= 0)
            {
                return yrm100_set_last_error_code(device_context, YRM100_STATUS_OK);
            }
            else
            {
                return yrm100_set_last_error_code(device_context, parse_result);
            }
        }
        return yrm100_set_last_error_code(device_context, YRM100_STATUS_OK);
    }
    return yrm100_set_last_error_code(device_context, YRM100_ERROR_UNKNOWN_ERROR);
}

int yrm100_command_get_select_parameters(yrm100_context_t *device_context, yrm100_select_parameters_t *select_parameters)
{
    uint8_t bytes[] = {0xBB, 0x00, 0x0B, 0x00, 0x00, 0x0B, 0x7E};
    size_t payload_length;

    if (yrm100_is_device_context_valid(device_context) == false)
    {
        return yrm100_set_last_error_code(device_context, YRM100_ERROR_INVALID_DEVICE_HANDLE);
    }
    if (select_parameters == NULL)
    {
        return yrm100_set_last_error_code(device_context, YRM100_ERROR_BUFFER_NULL);
    }
    int checksum = yrm100_frame_calculate_checksum(bytes, sizeof(bytes));
    if (checksum < 0)
    {
        return yrm100_set_last_error_code(device_context, YRM100_ERROR_CHECKSUM_CALCULATION_FAILURE);
    }
    bytes[sizeof(bytes) - 2] = (uint8_t)checksum;
    if (yrm100_command_send(device_context, bytes, sizeof(bytes)) == YRM100_STATUS_OK)
    {
        ssize_t response_len = yrm100_command_read_response(device_context);
        if (response_len < 0)
        {
            return yrm100_set_last_error_code(device_context, response_len);
        }
        if (yrm100_frame_is_ok_response(device_context->command_response_buf, (size_t)response_len))
        {
            payload_length = ((size_t)device_context->command_response_buf[3] << 8) |
                             (size_t)device_context->command_response_buf[4];
            if (device_context->command_response_buf[YRM100_FRAME_BYTE_POSITION_COMMAND] != 0x0B ||
                payload_length < 19)
            {
                return yrm100_set_last_error_code(device_context, YRM100_ERROR_PARSE_ERROR);
            }
            // byte 5 -> SelParam
            // byte 6, 7, 8, 9 -> Pointer
            // byte 10 -> Length
            // byte 11 -> Truncate
            // byte 12..23 -> Mask
            select_parameters->target = device_context->command_response_buf[5] & 0x07;
            select_parameters->action = (device_context->command_response_buf[5] >> 3) & 0x07;
            select_parameters->membank = (device_context->command_response_buf[5] >> 6) & 0x03;
            select_parameters->length = device_context->command_response_buf[10];
            select_parameters->pointer = (unsigned int)(((unsigned int)device_context->command_response_buf[6] << 24) |
                                                        ((unsigned int)device_context->command_response_buf[7] << 16) |
                                                        ((unsigned int)device_context->command_response_buf[8] << 8) |
                                                        ((unsigned int)device_context->command_response_buf[9]));
            select_parameters->truncate = device_context->command_response_buf[11];
            memcpy(select_parameters->mask, &device_context->command_response_buf[12], YRM100_TAG_EPC_BYTE_COUNT);

            return yrm100_set_last_error_code(device_context, YRM100_STATUS_OK);
        }
    }
    return yrm100_set_last_error_code(device_context, YRM100_ERROR_UNKNOWN_ERROR);
}

int yrm100_command_set_select_parameters(yrm100_context_t *device_context, yrm100_select_parameters_t *select_parameters)
{
    uint8_t bytes[] = {0xBB, 0x00, 0x0C, 0x00, 0x13, 0x01, 0x00, 0x00, 0x00, 0x20, 0x60, 0x00, 0x30, 0x75, 0x1F, 0xEB, 0x70, 0x5C, 0x59, 0x04, 0xE3, 0xD5, 0x0D, 0x70, 0xAD, 0x7E};

    if (yrm100_is_device_context_valid(device_context) == false)
    {
        return yrm100_set_last_error_code(device_context, YRM100_ERROR_INVALID_DEVICE_HANDLE);
    }
    if (select_parameters == NULL)
    {
        return yrm100_set_last_error_code(device_context, YRM100_ERROR_BUFFER_NULL);
    }
    bytes[5] = yrm100_pack_select_parameters(select_parameters);
    bytes[6] = (uint8_t)((select_parameters->pointer >> 24) & 0xFF);
    bytes[7] = (uint8_t)((select_parameters->pointer >> 16) & 0xFF);
    bytes[8] = (uint8_t)((select_parameters->pointer >> 8) & 0xFF);
    bytes[9] = (uint8_t)((select_parameters->pointer >> 0) & 0xFF);
    bytes[10] = select_parameters->length;
    bytes[11] = select_parameters->truncate;
    memcpy(&bytes[12], select_parameters->mask, YRM100_TAG_EPC_BYTE_COUNT);

    int checksum = yrm100_frame_calculate_checksum(bytes, sizeof(bytes));
    if (checksum < 0)
    {
        return yrm100_set_last_error_code(device_context, YRM100_ERROR_CHECKSUM_CALCULATION_FAILURE);
    }
    bytes[sizeof(bytes) - 2] = (uint8_t)checksum;
    if (yrm100_command_send(device_context, bytes, sizeof(bytes)) == YRM100_STATUS_OK)
    {
        ssize_t response_len = yrm100_command_read_response(device_context);
        if (response_len < 0)
        {
            return yrm100_set_last_error_code(device_context, response_len);
        }
        if (yrm100_frame_is_ok_response(device_context->command_response_buf, (size_t)response_len))
        {
            return yrm100_set_last_error_code(device_context, YRM100_STATUS_OK);
        }
    }
    return yrm100_set_last_error_code(device_context, YRM100_ERROR_UNKNOWN_ERROR);
}

int yrm100_command_set_select_mode(yrm100_context_t *device_context, uint8_t select_mode)
{
    uint8_t bytes[] = {0xBB, 0x00, 0x12, 0x00, 0x01, 0x01, 0x14, 0x7E};

    if (yrm100_param_is_valid_select_mode(select_mode) == false)
    {
        return yrm100_set_last_error_code(device_context, YRM100_ERROR_UNKNOWN_SELECT_MODE);
    }
    if (yrm100_is_device_context_valid(device_context) == false)
    {
        return yrm100_set_last_error_code(device_context, YRM100_ERROR_INVALID_DEVICE_HANDLE);
    }

    bytes[5] = select_mode;
    int checksum = yrm100_frame_calculate_checksum(bytes, sizeof(bytes));
    if (checksum < 0)
    {
        return yrm100_set_last_error_code(device_context, YRM100_ERROR_CHECKSUM_CALCULATION_FAILURE);
    }
    bytes[sizeof(bytes) - 2] = (uint8_t)checksum;

    if (yrm100_command_send(device_context, bytes, sizeof(bytes)) == YRM100_STATUS_OK)
    {
        ssize_t response_len = yrm100_command_read_response(device_context);
        if (response_len < 0)
        {
            return yrm100_set_last_error_code(device_context, response_len);
        }
        if (yrm100_frame_is_ok_response(device_context->command_response_buf, (size_t)response_len))
        {
            return yrm100_set_last_error_code(device_context, YRM100_STATUS_OK);
        }
    }
    return yrm100_set_last_error_code(device_context, YRM100_ERROR_UNKNOWN_ERROR);
}

int yrm100_command_get_query_parameters(yrm100_context_t *device_context, yrm100_query_parameters_t *query_parameters)
{
    uint8_t bytes[] = {0xBB, 0x00, 0x0D, 0x00, 0x00, 0x0D, 0x7E};
    if (yrm100_is_device_context_valid(device_context) == false)
    {
        return yrm100_set_last_error_code(device_context, YRM100_ERROR_INVALID_DEVICE_HANDLE);
    }
    if (query_parameters == NULL)
    {
        return yrm100_set_last_error_code(device_context, YRM100_ERROR_BUFFER_NULL);
    }
    int checksum = yrm100_frame_calculate_checksum(bytes, sizeof(bytes));
    if (checksum < 0)
    {
        return yrm100_set_last_error_code(device_context, YRM100_ERROR_CHECKSUM_CALCULATION_FAILURE);
    }
    bytes[sizeof(bytes) - 2] = (uint8_t)checksum;
    if (yrm100_command_send(device_context, bytes, sizeof(bytes)) == YRM100_STATUS_OK)
    {
        ssize_t response_len = yrm100_command_read_response(device_context);
        if (response_len < 0)
        {
            return yrm100_set_last_error_code(device_context, response_len);
        }
        if (yrm100_frame_is_ok_response(device_context->command_response_buf, (size_t)response_len))
        {
            unsigned short packed_query_params = (unsigned short)(((unsigned short)device_context->command_response_buf[5] << 8) |
                                                                  ((unsigned short)device_context->command_response_buf[6]));
            unpack_query_parameters(packed_query_params, query_parameters);
            return yrm100_set_last_error_code(device_context, YRM100_STATUS_OK);
        }
    }
    return yrm100_set_last_error_code(device_context, YRM100_ERROR_UNKNOWN_ERROR);
}

int yrm100_command_set_idle_sleep_time(yrm100_context_t *device_context, uint8_t minutes)
{
    uint8_t bytes[] = {0xBB, 0x00, 0x1D, 0x00, 0x01, 0x02, 0x20, 0x7E};

    if (yrm100_is_device_context_valid(device_context) == false)
    {
        return yrm100_set_last_error_code(device_context, YRM100_ERROR_INVALID_DEVICE_HANDLE);
    }
    bytes[5] = minutes;
    int checksum = yrm100_frame_calculate_checksum(bytes, sizeof(bytes));
    if (checksum < 0)
    {
        return yrm100_set_last_error_code(device_context, YRM100_ERROR_CHECKSUM_CALCULATION_FAILURE);
    }
    bytes[sizeof(bytes) - 2] = (uint8_t)checksum;

    if (yrm100_command_send(device_context, bytes, sizeof(bytes)) == YRM100_STATUS_OK)
    {
        ssize_t response_len = yrm100_command_read_response(device_context);
        if (response_len < 0)
        {
            return yrm100_set_last_error_code(device_context, response_len);
        }
        if (yrm100_frame_is_ok_response(device_context->command_response_buf, (size_t)response_len))
        {
            return yrm100_set_last_error_code(device_context, YRM100_STATUS_OK);
        }
    }
    return yrm100_set_last_error_code(device_context, YRM100_ERROR_UNKNOWN_ERROR);
}

int yrm100_command_disable_idle_sleep(yrm100_context_t *device_context)
{
    return yrm100_command_set_idle_sleep_time(device_context, 0);
}

int yrm100_command_set_operating_region(yrm100_context_t *device_context, uint8_t region)
{
    uint8_t bytes[] = {0xBB, 0x00, 0x07, 0x00, 0x01, 0x01, 0x09, 0x7E};

    if (yrm100_is_device_context_valid(device_context) == false)
    {
        return yrm100_set_last_error_code(device_context, YRM100_ERROR_INVALID_DEVICE_HANDLE);
    }
    if (yrm100_param_is_valid_region(region) == false)
    {
        return yrm100_set_last_error_code(device_context, YRM100_ERROR_UNKNOWN_REGION);
    }

    bytes[5] = region;
    int checksum = yrm100_frame_calculate_checksum(bytes, sizeof(bytes));
    if (checksum < 0)
    {
        return yrm100_set_last_error_code(device_context, YRM100_ERROR_CHECKSUM_CALCULATION_FAILURE);
    }
    bytes[sizeof(bytes) - 2] = (uint8_t)checksum;
    if (yrm100_command_send(device_context, bytes, sizeof(bytes)) == YRM100_STATUS_OK)
    {
        ssize_t response_len = yrm100_command_read_response(device_context);
        if (response_len < 0)
        {
            return yrm100_set_last_error_code(device_context, response_len);
        }
        if (yrm100_frame_is_ok_response(device_context->command_response_buf, (size_t)response_len))
        {
            return yrm100_set_last_error_code(device_context, YRM100_STATUS_OK);
        }
        if (yrm100_frame_is_error_response(device_context->command_response_buf, (size_t)response_len))
        {
            return yrm100_set_last_error_code(device_context, yrm100_parse_get_error_code(device_context->command_response_buf, (size_t)response_len));
        }
        return yrm100_set_last_error_code(device_context, YRM100_ERROR_COMMAND_FAILED);
    }
    return yrm100_set_last_error_code(device_context, YRM100_ERROR_UNKNOWN_ERROR);
}

int yrm100_command_get_operating_region(yrm100_context_t *device_context)
{
    uint8_t bytes[] = {0xBB, 0x00, 0x08, 0x00, 0x00, 0x08, 0x7E};

    if (yrm100_is_device_context_valid(device_context) == false)
    {
        return yrm100_set_last_error_code(device_context, YRM100_ERROR_INVALID_DEVICE_HANDLE);
    }
    int checksum = yrm100_frame_calculate_checksum(bytes, sizeof(bytes));
    if (checksum < 0)
    {
        return yrm100_set_last_error_code(device_context, YRM100_ERROR_CHECKSUM_CALCULATION_FAILURE);
    }
    bytes[sizeof(bytes) - 2] = (uint8_t)checksum;
    if (yrm100_command_send(device_context, bytes, sizeof(bytes)) == YRM100_STATUS_OK)
    {
        ssize_t response_len = yrm100_command_read_response(device_context);
        if (response_len < 0)
        {
            return yrm100_set_last_error_code(device_context, response_len);
        }
        if (yrm100_frame_is_ok_response(device_context->command_response_buf, (size_t)response_len))
        {
            yrm100_set_last_error_code(device_context, YRM100_STATUS_OK);
            return device_context->command_response_buf[5];
        }
        if (yrm100_frame_is_error_response(device_context->command_response_buf, (size_t)response_len))
        {
            return yrm100_set_last_error_code(device_context, yrm100_parse_get_error_code(device_context->command_response_buf, (size_t)response_len));
        }
        return yrm100_set_last_error_code(device_context, YRM100_ERROR_COMMAND_FAILED);
    }
    return yrm100_set_last_error_code(device_context, YRM100_ERROR_UNKNOWN_ERROR);
}

int yrm100_command_set_tx_power(yrm100_context_t *device_context, unsigned short power)
{
    uint8_t bytes[] = {0xBB, 0x00, 0xB6, 0x00, 0x02, 0x07, 0xD0, 0x8F, 0x7E};

    if (yrm100_is_device_context_valid(device_context) == false)
    {
        return yrm100_set_last_error_code(device_context, YRM100_ERROR_INVALID_DEVICE_HANDLE);
    }
    if (power < YRM100_PARAM_TX_POWER_MINIMUM)
    {
        power = YRM100_PARAM_TX_POWER_MINIMUM;
    }
    if (power > YRM100_PARAM_TX_POWER_MAXIMUM)
    {
        power = YRM100_PARAM_TX_POWER_MAXIMUM;
    }
    bytes[5] = (uint8_t)(power >> 8);
    bytes[6] = (uint8_t)(power & 0xFF);
    int checksum = yrm100_frame_calculate_checksum(bytes, sizeof(bytes));
    if (checksum < 0)
    {
        return yrm100_set_last_error_code(device_context, YRM100_ERROR_CHECKSUM_CALCULATION_FAILURE);
    }
    bytes[sizeof(bytes) - 2] = (uint8_t)checksum;
    if (yrm100_command_send(device_context, bytes, sizeof(bytes)) == YRM100_STATUS_OK)
    {
        ssize_t response_len = yrm100_command_read_response(device_context);
        if (response_len < 0)
        {
            return yrm100_set_last_error_code(device_context, response_len);
        }
        if (yrm100_frame_is_ok_response(device_context->command_response_buf, (size_t)response_len))
        {
            return yrm100_set_last_error_code(device_context, YRM100_STATUS_OK);
        }
        if (yrm100_frame_is_error_response(device_context->command_response_buf, (size_t)response_len))
        {
            return yrm100_set_last_error_code(device_context, yrm100_parse_get_error_code(device_context->command_response_buf, (size_t)response_len));
        }
        return yrm100_set_last_error_code(device_context, YRM100_ERROR_COMMAND_FAILED);
    }
    return yrm100_set_last_error_code(device_context, YRM100_ERROR_UNKNOWN_ERROR);
}

int yrm100_command_get_tx_power(yrm100_context_t *device_context)
{
    uint8_t bytes[] = {0xBB, 0x00, 0xB7, 0x00, 0x00, 0xB7, 0x7E};

    int checksum = yrm100_frame_calculate_checksum(bytes, sizeof(bytes));
    if (checksum < 0)
    {
        return yrm100_set_last_error_code(device_context, YRM100_ERROR_CHECKSUM_CALCULATION_FAILURE);
    }
    bytes[sizeof(bytes) - 2] = (uint8_t)checksum;
    if (yrm100_is_device_context_valid(device_context) == false)
    {
        return yrm100_set_last_error_code(device_context, YRM100_ERROR_INVALID_DEVICE_HANDLE);
    }
    if (yrm100_command_send(device_context, bytes, sizeof(bytes)) == YRM100_STATUS_OK)
    {
        ssize_t response_len = yrm100_command_read_response(device_context);
        if (response_len < 0)
        {
            return yrm100_set_last_error_code(device_context, response_len);
        }
        if (yrm100_frame_is_ok_response(device_context->command_response_buf, (size_t)response_len))
        {
            yrm100_set_last_error_code(device_context, YRM100_STATUS_OK);
            return (device_context->command_response_buf[5] << 8) | (unsigned short)device_context->command_response_buf[6];
        }
        if (yrm100_frame_is_error_response(device_context->command_response_buf, (size_t)response_len))
        {
            return yrm100_set_last_error_code(device_context, yrm100_parse_get_error_code(device_context->command_response_buf, (size_t)response_len));
        }
        return yrm100_set_last_error_code(device_context, YRM100_ERROR_COMMAND_FAILED);
    }
    return yrm100_set_last_error_code(device_context, YRM100_ERROR_UNKNOWN_ERROR);
}

int yrm100_command_set_continous_wave(yrm100_context_t *device_context, uint8_t on_or_off)
{
    uint8_t bytes[] = {0xBB, 0x00, 0xB0, 0x00, 0x01, 0xFF, 0xB0, 0x7E};

    bytes[5] = on_or_off;
    int checksum = yrm100_frame_calculate_checksum(bytes, sizeof(bytes));
    if (checksum < 0)
    {
        return yrm100_set_last_error_code(device_context, YRM100_ERROR_CHECKSUM_CALCULATION_FAILURE);
    }
    bytes[sizeof(bytes) - 2] = (uint8_t)checksum;
    if (yrm100_is_device_context_valid(device_context) == false)
    {
        return yrm100_set_last_error_code(device_context, YRM100_ERROR_INVALID_DEVICE_HANDLE);
    }
    if (yrm100_command_send(device_context, bytes, sizeof(bytes)) == YRM100_STATUS_OK)
    {
        ssize_t response_len = yrm100_command_read_response(device_context);
        if (response_len < 0)
        {
            return yrm100_set_last_error_code(device_context, response_len);
        }
        if (yrm100_frame_is_ok_response(device_context->command_response_buf, (size_t)response_len))
        {
            return yrm100_set_last_error_code(device_context, YRM100_STATUS_OK);
        }
        if (yrm100_frame_is_error_response(device_context->command_response_buf, (size_t)response_len))
        {
            return yrm100_set_last_error_code(device_context, yrm100_parse_get_error_code(device_context->command_response_buf, (size_t)response_len));
        }
        return yrm100_set_last_error_code(device_context, YRM100_ERROR_COMMAND_FAILED);
    }
    return yrm100_set_last_error_code(device_context, YRM100_ERROR_UNKNOWN_ERROR);
}

int yrm100_command_enable_continous_wave(yrm100_context_t *device_context)
{
    return yrm100_command_set_continous_wave(device_context, YRM100_PARAM_CONTINOUS_WAVE_ON);
}

int yrm100_command_disable_continous_wave(yrm100_context_t *device_context)
{
    return yrm100_command_set_continous_wave(device_context, YRM100_PARAM_CONTINOUS_WAVE_OFF);
}

int yrm100_command_sleep(yrm100_context_t *device_context)
{
    uint8_t bytes[] = {0xBB, 0x00, 0x17, 0x00, 0x00, 0x17, 0x7E};
    if (yrm100_is_device_context_valid(device_context) == false)
    {
        return yrm100_set_last_error_code(device_context, YRM100_ERROR_INVALID_DEVICE_HANDLE);
    }
    int checksum = yrm100_frame_calculate_checksum(bytes, sizeof(bytes));
    if (checksum < 0)
    {
        return yrm100_set_last_error_code(device_context, YRM100_ERROR_CHECKSUM_CALCULATION_FAILURE);
    }
    bytes[sizeof(bytes) - 2] = (uint8_t)checksum;
    if (yrm100_command_send(device_context, bytes, sizeof(bytes)) == YRM100_STATUS_OK)
    {
        ssize_t response_len = yrm100_command_read_response(device_context);
        if (response_len < 0)
        {
            return yrm100_set_last_error_code(device_context, response_len);
        }
        if (yrm100_frame_is_ok_response(device_context->command_response_buf, (size_t)response_len))
        {
            return yrm100_set_last_error_code(device_context, YRM100_STATUS_OK);
        }
        if (yrm100_frame_is_error_response(device_context->command_response_buf, (size_t)response_len))
        {
            return yrm100_set_last_error_code(device_context, yrm100_parse_get_error_code(device_context->command_response_buf, (size_t)response_len));
        }
        return yrm100_set_last_error_code(device_context, YRM100_ERROR_COMMAND_FAILED);
    }
    return yrm100_set_last_error_code(device_context, YRM100_ERROR_UNKNOWN_ERROR);
}

int yrm100_command_read_tag_memory_area(yrm100_context_t *device_context, yrm100_rfid_tag_t *tag, uint8_t memory_bank, unsigned short segment_address, unsigned short data_length, uint32_t password)
{
    uint8_t bytes[] = {0xBB, 0x00, 0x39, 0x00, 0x09, 0x00, 0x00, 0xFF, 0xFF, 0x03, 0x00, 0x00, 0x00, 0x02, 0x45, 0x7E};

    if (yrm100_is_device_context_valid(device_context) == false)
    {
        return yrm100_set_last_error_code(device_context, YRM100_ERROR_INVALID_DEVICE_HANDLE);
    }
    if (tag == NULL)
    {
        return yrm100_set_last_error_code(device_context, YRM100_ERROR_BUFFER_NULL);
    }
    if (data_length == 0)
    {
        return yrm100_set_last_error_code(device_context, YRM100_ERROR_INVALID_DATA_LENGTH);
    }
    if (yrm100_param_is_valid_memory_bank(memory_bank) == false)
    {
        return yrm100_set_last_error_code(device_context, YRM100_ERROR_UNKNOWN_MEMORY_BANK);
    }

    bytes[5] = (uint8_t)((password >> 24) & 0xFF);
    bytes[6] = (uint8_t)((password >> 16) & 0xFF);
    bytes[7] = (uint8_t)((password >> 8) & 0xFF);
    bytes[8] = (uint8_t)((password >> 0) & 0xFF);
    bytes[9] = memory_bank;
    bytes[10] = (uint8_t)((segment_address >> 8) & 0xFF);
    bytes[11] = (uint8_t)((segment_address >> 0) & 0xFF);
    bytes[12] = (uint8_t)((data_length >> 8) & 0xFF);
    bytes[13] = (uint8_t)((data_length >> 0) & 0xFF);
    int checksum = yrm100_frame_calculate_checksum(bytes, sizeof(bytes));
    if (checksum < 0)
    {
        return yrm100_set_last_error_code(device_context, YRM100_ERROR_CHECKSUM_CALCULATION_FAILURE);
    }
    bytes[sizeof(bytes) - 2] = (uint8_t)checksum;
    if (yrm100_command_send(device_context, bytes, sizeof(bytes)) == YRM100_STATUS_OK)
    {
        ssize_t response_len = yrm100_command_read_response(device_context);
        if (response_len < 0)
        {
            return yrm100_set_last_error_code(device_context, response_len);
        }
        if (yrm100_frame_is_read_tag_memory_response(device_context->command_response_buf, (size_t)response_len))
        {
            int parse_result = yrm100_parse_read_tag_memory_response(
                device_context->command_response_buf,
                (size_t)response_len,
                tag,
                data_length);
            if (parse_result != YRM100_STATUS_OK)
            {
                return yrm100_set_last_error_code(device_context, parse_result);
            }
            return yrm100_set_last_error_code(device_context, YRM100_STATUS_OK);
        }
        if (yrm100_frame_is_error_response(device_context->command_response_buf, (size_t)response_len))
        {
            return yrm100_set_last_error_code(device_context, yrm100_parse_get_error_code(device_context->command_response_buf, (size_t)response_len));
        }
        return yrm100_set_last_error_code(device_context, YRM100_ERROR_COMMAND_FAILED);
    }
    return yrm100_set_last_error_code(device_context, YRM100_ERROR_UNKNOWN_ERROR);
}

int yrm100_command_kill(yrm100_context_t *device_context, uint32_t password)
{
    uint8_t bytes[] = {0xBB, 0x00, 0x65, 0x00, 0x04, 0x00, 0x00, 0xFF, 0x70, 0x67, 0x7E};
    bytes[5] = (uint8_t)((password >> 24) & 0xFF);
    bytes[6] = (uint8_t)((password >> 16) & 0xFF);
    bytes[7] = (uint8_t)((password >> 8) & 0xFF);
    bytes[8] = (uint8_t)((password >> 0) & 0xFF);
    int checksum = yrm100_frame_calculate_checksum(bytes, sizeof(bytes));
    if (checksum < 0)
    {
        return yrm100_set_last_error_code(device_context, YRM100_ERROR_CHECKSUM_CALCULATION_FAILURE);
    }
    bytes[sizeof(bytes) - 2] = (uint8_t)checksum;
    if (yrm100_command_send(device_context, bytes, sizeof(bytes)) == YRM100_STATUS_OK)
    {
        ssize_t response_len = yrm100_command_read_response(device_context);
        if (response_len < 0)
        {
            return yrm100_set_last_error_code(device_context, response_len);
        }
        if (yrm100_frame_is_ok_response(device_context->command_response_buf, (size_t)response_len))
        {
            return yrm100_set_last_error_code(device_context, YRM100_STATUS_OK);
        }
        if (yrm100_frame_is_error_response(device_context->command_response_buf, (size_t)response_len))
        {
            return yrm100_set_last_error_code(device_context, yrm100_parse_get_error_code(device_context->command_response_buf, (size_t)response_len));
        }
        return yrm100_set_last_error_code(device_context, YRM100_ERROR_COMMAND_FAILED);
    }
    return yrm100_set_last_error_code(device_context, YRM100_ERROR_UNKNOWN_ERROR);
}

int yrm100_command_lock(yrm100_context_t *device_context, uint32_t password)
{
    uint8_t bytes[] = {0xBB, 0x00, 0x82, 0x00, 0x07, 0x00, 0x00, 0xFF, 0xFF, 0x02, 0x00, 0x80, 0x09, 0x7E};
    bytes[5] = (uint8_t)((password >> 24) & 0xFF);
    bytes[6] = (uint8_t)((password >> 16) & 0xFF);
    bytes[7] = (uint8_t)((password >> 8) & 0xFF);
    bytes[8] = (uint8_t)((password >> 0) & 0xFF);
    int checksum = yrm100_frame_calculate_checksum(bytes, sizeof(bytes));
    if (checksum < 0)
    {
        return yrm100_set_last_error_code(device_context, YRM100_ERROR_CHECKSUM_CALCULATION_FAILURE);
    }
    bytes[sizeof(bytes) - 2] = (uint8_t)checksum;
    if (yrm100_command_send(device_context, bytes, sizeof(bytes)) == YRM100_STATUS_OK)
    {
        ssize_t response_len = yrm100_command_read_response(device_context);
        if (response_len < 0)
        {
            return yrm100_set_last_error_code(device_context, response_len);
        }
        if (yrm100_frame_is_ok_response(device_context->command_response_buf, (size_t)response_len))
        {
            return yrm100_set_last_error_code(device_context, YRM100_STATUS_OK);
        }
        if (yrm100_frame_is_error_response(device_context->command_response_buf, (size_t)response_len))
        {
            return yrm100_set_last_error_code(device_context, yrm100_parse_get_error_code(device_context->command_response_buf, (size_t)response_len));
        }
        return yrm100_set_last_error_code(device_context, YRM100_ERROR_COMMAND_FAILED);
    }
    return yrm100_set_last_error_code(device_context, YRM100_ERROR_UNKNOWN_ERROR);
}

char *yrm100_command_get_tx_power_string(yrm100_context_t *device_context, char string_buf[YRM100_PARAM_TX_POWER_STRING_LENGTH])
{
    yrm100_zero_buf(string_buf, YRM100_PARAM_TX_POWER_STRING_LENGTH);
    int result = yrm100_command_get_tx_power(device_context);
    if (result < 0)
    {
        return string_buf;
    }
    return yrm100_convert_to_tx_power_string((unsigned int)result, string_buf);
}

char *yrm100_command_get_module_info_string(yrm100_context_t *device_context, char string_buf[YRM100_MODULE_INFO_STRING_LENGTH])
{
    char manufacturer_buf[50];
    char software_version_buf[50];
    char hardware_version_buf[50];

    yrm100_zero_buf(string_buf, YRM100_MODULE_INFO_STRING_LENGTH);
    yrm100_zero_buf(manufacturer_buf, sizeof(manufacturer_buf));
    yrm100_zero_buf(software_version_buf, sizeof(software_version_buf));
    yrm100_zero_buf(hardware_version_buf, sizeof(hardware_version_buf));

    if (yrm100_command_get_module_manufacturer(device_context, manufacturer_buf, sizeof(manufacturer_buf)) != YRM100_STATUS_OK)
    {
        return string_buf;
    }
    if (yrm100_command_get_module_hardware_version(device_context, hardware_version_buf, sizeof(hardware_version_buf)) != YRM100_STATUS_OK)
    {
        return string_buf;
    }
    if (yrm100_command_get_module_software_version(device_context, software_version_buf, sizeof(software_version_buf)) != YRM100_STATUS_OK)
    {
        return string_buf;
    }

    snprintf(string_buf, YRM100_MODULE_INFO_STRING_LENGTH, "%s %s / FW: %s", manufacturer_buf, hardware_version_buf, software_version_buf);

    return string_buf;
}
