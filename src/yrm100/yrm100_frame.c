#include <stdio.h>
#include <unistd.h>
#include "yrm100_frame.h"
#include "yrm100_error.h"

bool yrm100_frame_is_command(unsigned char *buf, size_t buf_size)
{
    if (buf == NULL)
    {
        return false;
    }
    if (buf_size < YRM100_FRAME_MINIMUM_COMMAND_SIZE)
    {
        return false;
    }
    if (buf[YRM100_FRAME_BYTE_POSITION_HEADER] != RFID_UHF_FRAME_HEADER_BYTE || buf[RFID_UHF_FRAME_BYTE_POSITION_TYPE] != RFID_UHF_FRAME_TYPE_BYTE_COMMAND)
    {
        return false;
    }
    return true;
}

bool yrm100_frame_is_response(unsigned char *buf, size_t buf_size)
{
    if (buf == NULL)
    {
        return false;
    }
    if (buf_size < YRM100_FRAME_MINIMUM_RESPONSE_SIZE)
    {
        return false;
    }
    if (buf[YRM100_FRAME_BYTE_POSITION_HEADER] != RFID_UHF_FRAME_HEADER_BYTE || buf[RFID_UHF_FRAME_BYTE_POSITION_TYPE] != RFID_UHF_FRAME_TYPE_BYTE_RESPONSE)
    {
        return false;
    }
    return true;
}

bool yrm100_frame_is_notice(unsigned char *buf, size_t buf_size)
{
    if (buf == NULL)
    {
        return false;
    }
    if (buf_size < YRM100_FRAME_MINIMUM_NOTICE_SIZE)
    {
        return false;
    }
    if (buf[YRM100_FRAME_BYTE_POSITION_HEADER] != RFID_UHF_FRAME_HEADER_BYTE || buf[RFID_UHF_FRAME_BYTE_POSITION_TYPE] != RFID_UHF_FRAME_TYPE_BYTE_NOTICE)
    {
        return false;
    }
    return true;
}

bool yrm100_frame_is_valid_command(unsigned char *buf, size_t buf_size)
{
    if (buf == NULL)
    {
        return false;
    }
    if (yrm100_frame_is_command(buf, buf_size) == false)
    {
        return false;
    }
    if (buf[buf_size - 2] != yrm100_frame_calculate_checksum(buf, buf_size))
    {
        return false;
    }
    return true;
}

bool yrm100_frame_is_valid_response(unsigned char *buf, size_t buf_size)
{
    if (buf == NULL)
    {
        return false;
    }
    if (yrm100_frame_is_response(buf, buf_size) == false)
    {
        return false;
    }

    if (buf[buf_size - 2] != yrm100_frame_calculate_checksum(buf, buf_size))
    {
        return false;
    }
    return true;
}

bool yrm100_frame_is_error_response(unsigned char *buf, size_t buf_size)
{
    if (buf == NULL)
    {
        return false;
    }
    if (buf_size < YRM100_FRAME_MINIMUM_RESPONSE_SIZE)
    {
        return false;
    }
    if (buf[YRM100_FRAME_BYTE_POSITION_HEADER] == RFID_UHF_FRAME_HEADER_BYTE &&
        buf[YRM100_FRAME_BYTE_POSITION_TYPE] == RFID_UHF_FRAME_TYPE_BYTE_RESPONSE &&
        buf[YRM100_FRAME_BYTE_POSITION_COMMAND] == 0xFF)
    {
        return true;
    }
    return false;
}

bool yrm100_frame_is_ok_response(unsigned char *buf, size_t buf_size)
{
    if (buf == NULL)
    {
        return false;
    }
    if (buf_size < YRM100_FRAME_MINIMUM_RESPONSE_SIZE)
    {
        return false;
    }
    if (yrm100_frame_is_valid_response(buf, buf_size))
    {
        return yrm100_frame_is_error_response(buf, buf_size) == false;
    }
    return false;
}

int yrm100_frame_calculate_checksum(unsigned char *buf, size_t buf_size)
{
    if (buf == NULL)
    {
        return YRM100_ERROR_BUFFER_NULL;
    }

    // buf is command frame? lets also check the size then
    if (buf[YRM100_FRAME_BYTE_POSITION_HEADER] == RFID_UHF_FRAME_HEADER_BYTE &&
        buf[YRM100_FRAME_BYTE_POSITION_TYPE] == RFID_UHF_FRAME_TYPE_BYTE_COMMAND &&
        buf_size < YRM100_FRAME_MINIMUM_COMMAND_SIZE)
    {
        return YRM100_ERROR_PARSE_ERROR;
    }

    // buf is response frame? lets also check the size then
    if (buf[YRM100_FRAME_BYTE_POSITION_HEADER] == RFID_UHF_FRAME_HEADER_BYTE &&
        buf[YRM100_FRAME_BYTE_POSITION_TYPE] == RFID_UHF_FRAME_TYPE_BYTE_RESPONSE &&
        buf_size < YRM100_FRAME_MINIMUM_RESPONSE_SIZE)
    {
        return YRM100_ERROR_PARSE_ERROR;
    }

    // buf is notice frame? lets also check the size then
    if (buf[YRM100_FRAME_BYTE_POSITION_HEADER] == RFID_UHF_FRAME_HEADER_BYTE &&
        buf[YRM100_FRAME_BYTE_POSITION_TYPE] == RFID_UHF_FRAME_TYPE_BYTE_NOTICE &&
        buf_size < YRM100_FRAME_MINIMUM_NOTICE_SIZE)
    {
        return YRM100_ERROR_PARSE_ERROR;
    }

    unsigned int sum = 0;
    for (size_t i = 1; i < buf_size - 2; i++)
    {
        sum += buf[i];
    }
    return (unsigned char)(sum & 0xFF);
}
