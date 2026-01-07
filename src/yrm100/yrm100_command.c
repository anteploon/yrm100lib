#include <stdio.h>
#include <unistd.h>
#include "yrm100.h"
#include "yrm100_util.h"
#include "yrm100_frame.h"
#include "yrm100_serial.h"
#include "yrm100_parse.h"
#include "yrm100_command.h"
#include "yrm100_error.h"

int yrm100_set_last_error_code(yrm100_context_t *device_context, ssize_t error_code)
{
    if (device_context != NULL)
    {
        device_context->last_error_code = (int)error_code;
    }
    return (int)error_code;
}

static int yrm100_command_send(yrm100_context_t *device_context, unsigned char *cmd, size_t cmd_size)
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
        ssize_t n = yrm100_serial_write(device_context->serial_port, cmd, cmd_size);
        if (n < 0)
        {
            perror("Error writing to serial port");
            return yrm100_set_last_error_code(device_context, YRM100_ERROR_WRITING_TO_SERIAL_PORT_FAILED);
        }
        usleep(YRM100_COMMAND_RESPONSE_DELAY_USEC);
    }
    else
    {
        return yrm100_set_last_error_code(device_context, YRM100_ERROR_INVALID_COMMAND);
    }
    return yrm100_set_last_error_code(device_context, YRM100_STATUS_OK);
}

ssize_t yrm100_command_read_response(yrm100_context_t *device_context)
{
    if (yrm100_is_device_context_valid(device_context) == false)
    {
        return yrm100_set_last_error_code(device_context, YRM100_ERROR_INVALID_DEVICE_HANDLE);
    }

    unsigned char *buf = device_context->command_response_buf;
    size_t buf_size = sizeof(device_context->command_response_buf);
    unsigned short cursor = 0;
    size_t total_read = 0;
    ssize_t response_len;

    while (true)
    {
        if (cursor >= buf_size)
        {
            return yrm100_set_last_error_code(device_context, YRM100_ERROR_SERIAL_INPUT_OVERFLOW);
        }
        buf = &device_context->command_response_buf[cursor];
        response_len = yrm100_serial_read(device_context->serial_port, buf, buf_size - cursor);
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
            break;
        }
        cursor += (unsigned short)response_len;
        total_read += (size_t)response_len;
        if (device_context->command_response_buf[cursor - 1] == YRM100_FRAME_END_BYTE)
        {
            break;
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

unsigned char yrm100_pack_select_parameters(rfid_select_parameters_t *data)
{
    return (unsigned char)((data->target & 0x07) | ((data->action & 0x07) << 3) | ((data->membank & 0x03) << 6));
}

int yrm100_command_get_module_manufacturer(yrm100_context_t *device_context, char *string_buf, size_t string_buf_size)
{
    unsigned char bytes[] = {0xBB, 0x00, 0x03, 0x00, 0x01, 0x02, 0x06, 0x7E};

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
    }
    return yrm100_set_last_error_code(device_context, YRM100_ERROR_UNKNOWN_ERROR);
}

int yrm100_command_get_module_hardware_version(yrm100_context_t *device_context, char *string_buf, size_t string_buf_size)
{
    unsigned char bytes[] = {0xBB, 0x00, 0x03, 0x00, 0x01, 0x00, 0x04, 0x7E};

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
    unsigned char bytes[] = {0xBB, 0x00, 0x03, 0x00, 0x01, 0x01, 0x05, 0x7E};

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
    }
    return yrm100_set_last_error_code(device_context, YRM100_ERROR_UNKNOWN_ERROR);
}

int yrm100_command_single_poll(yrm100_context_t *device_context, rfid_tag_t *tags, unsigned short maximum_tag_count)
{
    unsigned char bytes[] = {0xBB, 0x00, 0x22, 0x00, 0x00, 0x22, 0x7E};

    if (yrm100_is_device_context_valid(device_context) == false)
    {
        return yrm100_set_last_error_code(device_context, YRM100_ERROR_INVALID_DEVICE_HANDLE);
    }
    int checksum = yrm100_frame_calculate_checksum(bytes, sizeof(bytes));
    if (checksum < 0)
    {
        return yrm100_set_last_error_code(device_context, YRM100_ERROR_CHECKSUM_CALCULATION_FAILURE);
    }
    bytes[sizeof(bytes) - 2] = (unsigned char)checksum;
    if (yrm100_command_send(device_context, bytes, sizeof(bytes)) == YRM100_STATUS_OK)
    {
        ssize_t response_len = yrm100_command_read_response(device_context);
        if (response_len < 0)
        {
            return yrm100_set_last_error_code(device_context, response_len);
        }
        if (response_len > 1)
        {
            int parse_result = yrm100_parse_poll_response(device_context->command_response_buf, (size_t)response_len, tags, maximum_tag_count);
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

/*
int yrm100_command_multi_poll_start(yrm100_context_t *device_context, rfid_tag_t *tags, unsigned short maximum_tag_count)
{
    unsigned char bytes[] = {0xBB, 0x00, 0x27, 0x00, 0x03, 0x22, 0x27, 0x10, 0x83, 0x7E};

    if (yrm100_is_device_context_valid(device_context) == false)
    {
        return yrm100_set_last_error_code(device_context, YRM100_ERROR_INVALID_DEVICE_HANDLE);
    }
    bytes[6] = (unsigned char)(maximum_tag_count >> 8);
    bytes[7] = (unsigned char)(maximum_tag_count & 0xFF);

    int checksum = yrm100_frame_calculate_checksum(bytes, sizeof(bytes));
    if (checksum < 0)
    {
        return yrm100_set_last_error_code(device_context, YRM100_ERROR_CHECKSUM_CALCULATION_FAILURE);
    }
    bytes[sizeof(bytes) - 2] = (unsigned char)checksum;

    device_context->multi_poll_tag_count = 0;
    if (yrm100_command_send(device_context, bytes, sizeof(bytes)) == YRM100_STATUS_OK)
    {
        device_context->is_multi_poll_running = true;
        device_context->multi_poll_maximum_tag_count = maximum_tag_count;
        device_context->multi_poll_target = tags;
        ssize_t response_len = yrm100_command_read_response(device_context);
        if (response_len < 0)
        {
            return yrm100_set_last_error_code(device_context, response_len;
        }
        if (response_len > 1)
        {
            int parse_result = yrm100_parse_poll_response(device_context->command_response_buf, (size_t)response_len, tags, maximum_tag_count);
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

int yrm100_command_multi_poll_continue(yrm100_context_t *device_context)
{
    if (yrm100_is_device_context_valid(device_context) == false)
    {
        return yrm100_set_last_error_code(device_context, YRM100_ERROR_INVALID_DEVICE_HANDLE);
    }
    if (device_context->is_multi_poll_running == false)
    {
        return yrm100_set_last_error_code(device_context, YRM100_ERROR_MULTI_POLL_NOT_RUNNING);
    }
    if (device_context->multi_poll_target == NULL)
    {
        return yrm100_set_last_error_code(device_context, YRM100_ERROR_BUFFER_NULL);
    }
    unsigned short maximum_tag_count = device_context->multi_poll_maximum_tag_count;
    unsigned short tag_count = device_context->multi_poll_tag_count;
    if (maximum_tag_count <= tag_count)
    {
        return yrm100_set_last_error_code(device_context, YRM100_ERROR_BUFFER_OVERFLOW);
    }
    rfid_tag_t *tags = &device_context->multi_poll_target[tag_count];

    ssize_t response_len = yrm100_command_read_response(device_context);
    if (response_len < 0)
    {
        return yrm100_set_last_error_code(device_context, response_len);
    }
    if (response_len > 1)
    {
        int parse_result = yrm100_parse_poll_response(device_context->command_response_buf, (size_t)response_len, tags, maximum_tag_count - tag_count);
        if (parse_result >= 0)
        {
            return yrm100_set_last_error_code(device_context, YRM100_STATUS_OK);
        }
        else
        {
            return yrm100_set_last_error_code(device_context, parse_result);
        }
    }
    return yrm100_set_last_error_code(device_context, YRM100_ERROR_UNKNOWN_ERROR);
}

int yrm100_command_multi_poll_stop(yrm100_context_t *device_context)
{
    unsigned char bytes[] = {0xBB, 0x00, 0x28, 0x00, 0x00, 0x28, 0x7E};

    if (yrm100_is_device_context_valid(device_context) == false)
    {
        return yrm100_set_last_error_code(device_context, YRM100_ERROR_INVALID_DEVICE_HANDLE);
    }
    if (device_context->is_multi_poll_running == false)
    {
        return yrm100_set_last_error_code(device_context, YRM100_ERROR_MULTI_POLL_NOT_RUNNING);
    }
    int checksum = yrm100_frame_calculate_checksum(bytes, sizeof(bytes));
    if (checksum < 0)
    {
        return yrm100_set_last_error_code(device_context, YRM100_ERROR_CHECKSUM_CALCULATION_FAILURE);
    }
    bytes[sizeof(bytes) - 2] = (unsigned char)checksum;

    if (yrm100_command_send(device_context, bytes, sizeof(bytes)) == YRM100_STATUS_OK)
    {
        ssize_t response_len = yrm100_command_read_response(device_context);
        if (response_len < 0)
        {
            return yrm100_set_last_error_code(device_context, response_len);
        }
        if (yrm100_frame_is_ok_response(device_context->command_response_buf, (size_t)response_len))
        {
            device_context->is_multi_poll_running = false;
            return yrm100_set_last_error_code(device_context, YRM100_STATUS_OK);
        }
    }
    return yrm100_set_last_error_code(device_context, YRM100_ERROR_UNKNOWN_ERROR);
}
*/
/*
int yrm100_command_set_select_parameters(yrm100_context_t *device_context)
{
    unsigned char bytes[] = {0xBB, 0x00, 0x0C, 0x00, 0x13, 0x01, 0x00, 0x00, 0x00, 0x20, 0x60, 0x00, 0x30, 0x75, 0x1F, 0xEB, 0x70, 0x5C, 0x59, 0x04, 0xE3, 0xD5, 0x0D, 0x70, 0xAD, 0x7E};
    return yrm100_set_last_error_code(device_context, YRM100_ERROR_UNKNOWN_ERROR);
}

int yrm100_command_get_select_parameters(yrm100_context_t *device_context)
{
    unsigned char bytes[] = {0xBB, 0x00, 0x0B, 0x00, 0x00, 0x0B, 0x7E};
    return yrm100_set_last_error_code(device_context, YRM100_ERROR_UNKNOWN_ERROR);
}
*/

int yrm100_command_set_select_mode(yrm100_context_t *device_context, unsigned char select_mode)
{
    unsigned char bytes[] = {0xBB, 0x00, 0x12, 0x00, 0x01, 0x01, 0x14, 0x7E};

    if (select_mode != YRM100_PARAM_SELECT_MODE_DO_SEND_BEFORE_ALL_OPERATIONS &&
        select_mode != YRM100_PARAM_SELECT_MODE_DONT_SEND_BEFORE_ANY_OPERATIONS &&
        select_mode != YRM100_PARAM_SELECT_MODE_DO_SEND_BEFORE_TAG_OPERATIONS)
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
    bytes[sizeof(bytes) - 2] = (unsigned char)checksum;

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

int yrm100_command_set_idle_sleep_time(yrm100_context_t *device_context, unsigned char minutes)
{
    unsigned char bytes[] = {0xBB, 0x00, 0x1D, 0x00, 0x01, 0x02, 0x20, 0x7E};

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
    bytes[sizeof(bytes) - 2] = (unsigned char)checksum;

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

int yrm100_command_set_operating_region(yrm100_context_t *device_context, unsigned char region)
{
    unsigned char bytes[] = {0xBB, 0x00, 0x07, 0x00, 0x01, 0x01, 0x09, 0x7E};

    if (yrm100_is_device_context_valid(device_context) == false)
    {
        return yrm100_set_last_error_code(device_context, YRM100_ERROR_INVALID_DEVICE_HANDLE);
    }
    if (region != YRM100_PARAM_REGION_CHINA_900 &&
        region != YRM100_PARAM_REGION_CHINA_800 &&
        region != YRM100_PARAM_REGION_EUROPE &&
        region != YRM100_PARAM_REGION_KOREA &&
        region != YRM100_PARAM_REGION_USA)
    {
        return yrm100_set_last_error_code(device_context, YRM100_ERROR_UNKNOWN_REGION);
    }

    bytes[5] = region;
    int checksum = yrm100_frame_calculate_checksum(bytes, sizeof(bytes));
    if (checksum < 0)
    {
        return yrm100_set_last_error_code(device_context, YRM100_ERROR_CHECKSUM_CALCULATION_FAILURE);
    }
    bytes[sizeof(bytes) - 2] = (unsigned char)checksum;
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
    unsigned char bytes[] = {0xBB, 0x00, 0x08, 0x00, 0x00, 0x08, 0x7E};

    if (yrm100_is_device_context_valid(device_context) == false)
    {
        return yrm100_set_last_error_code(device_context, YRM100_ERROR_INVALID_DEVICE_HANDLE);
    }
    int checksum = yrm100_frame_calculate_checksum(bytes, sizeof(bytes));
    if (checksum < 0)
    {
        return yrm100_set_last_error_code(device_context, YRM100_ERROR_CHECKSUM_CALCULATION_FAILURE);
    }
    bytes[sizeof(bytes) - 2] = (unsigned char)checksum;
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
        return yrm100_set_last_error_code(device_context, YRM100_ERROR_COMMAND_FAILED);
    }
    return yrm100_set_last_error_code(device_context, YRM100_ERROR_UNKNOWN_ERROR);
}

int yrm100_command_set_tx_power(yrm100_context_t *device_context, unsigned short power)
{
    unsigned char bytes[] = {0xBB, 0x00, 0xB6, 0x00, 0x02, 0x07, 0xD0, 0x8F, 0x7E};

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
    bytes[5] = (unsigned char)(power >> 8);
    bytes[6] = (unsigned char)(power & 0xFF);
    int checksum = yrm100_frame_calculate_checksum(bytes, sizeof(bytes));
    if (checksum < 0)
    {
        return yrm100_set_last_error_code(device_context, YRM100_ERROR_CHECKSUM_CALCULATION_FAILURE);
    }
    bytes[sizeof(bytes) - 2] = (unsigned char)checksum;
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
    unsigned char bytes[] = {0xBB, 0x00, 0xB7, 0x00, 0x00, 0xB7, 0x7E};

    int checksum = yrm100_frame_calculate_checksum(bytes, sizeof(bytes));
    if (checksum < 0)
    {
        return yrm100_set_last_error_code(device_context, YRM100_ERROR_CHECKSUM_CALCULATION_FAILURE);
    }
    bytes[sizeof(bytes) - 2] = (unsigned char)checksum;
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
        return yrm100_set_last_error_code(device_context, YRM100_ERROR_COMMAND_FAILED);
    }
    return yrm100_set_last_error_code(device_context, YRM100_ERROR_UNKNOWN_ERROR);
}

int yrm100_command_set_continous_wave(yrm100_context_t *device_context, unsigned char on_or_off)
{
    unsigned char bytes[] = {0xBB, 0x00, 0xB0, 0x00, 0x01, 0xFF, 0xB0, 0x7E};

    bytes[5] = on_or_off;
    int checksum = yrm100_frame_calculate_checksum(bytes, sizeof(bytes));
    if (checksum < 0)
    {
        return yrm100_set_last_error_code(device_context, YRM100_ERROR_CHECKSUM_CALCULATION_FAILURE);
    }
    bytes[sizeof(bytes) - 2] = (unsigned char)checksum;
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

    yrm100_command_get_module_manufacturer(device_context, manufacturer_buf, sizeof(manufacturer_buf));
    yrm100_command_get_module_hardware_version(device_context, hardware_version_buf, sizeof(hardware_version_buf));
    yrm100_command_get_module_software_version(device_context, software_version_buf, sizeof(software_version_buf));

    snprintf(string_buf, YRM100_MODULE_INFO_STRING_LENGTH, "%s %s / FW: %s", manufacturer_buf, hardware_version_buf, software_version_buf);

    return string_buf;
}
