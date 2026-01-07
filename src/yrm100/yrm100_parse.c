#include <limits.h>
#include "yrm100.h"
#include "yrm100_error.h"
#include "yrm100_frame.h"
#include "yrm100_util.h"
#include "yrm100_parse.h"

int yrm100_parse_ascii_response(unsigned char *response, size_t response_len, char *string_buf, size_t string_buf_len)
{
    char work_buf[YRM100_ASCII_BUFFER_SIZE];

    if (response == NULL || string_buf == NULL)
    {
        return YRM100_ERROR_BUFFER_NULL;
    }
    if (string_buf_len == 0)
    {
        return YRM100_ERROR_BUFFER_TOO_SMALL;
    }
    if (response_len < YRM100_FRAME_MINIMUM_RESPONSE_SIZE)
    {
        return YRM100_ERROR_PARSE_ERROR;
    }

    yrm100_zero_buf(work_buf, YRM100_ASCII_BUFFER_SIZE);
    yrm100_zero_buf(string_buf, string_buf_len);

    size_t response_str_len = (size_t)((response[3] << 8) | response[4]);

    if (response_len < (7 + response_str_len))
    {
        return YRM100_ERROR_PARSE_ERROR;
    }
    if (response_str_len + 1 > YRM100_ASCII_BUFFER_SIZE)
    {
        return YRM100_ERROR_PARSE_ERROR;
    }
    if (response_str_len > 1)
    {
        response_str_len = response_str_len - 1;
        if (string_buf_len <= response_str_len)
        {
            return YRM100_ERROR_BUFFER_TOO_SMALL;
        }
        memcpy(work_buf, &response[6], response_str_len);
        work_buf[response_str_len] = 0;
        memcpy(string_buf, work_buf, response_str_len + 1);
        return YRM100_STATUS_OK;
    }
    return YRM100_ERROR_UNKNOWN_ERROR;
}

int yrm100_parse_poll_response(unsigned char *response, size_t response_len, rfid_tag_t *tags, unsigned short maximum_tag_count)
{
    size_t pos = 0;

    if (response == NULL || tags == NULL)
    {
        return YRM100_ERROR_BUFFER_NULL;
    }
    if (response_len>(USHRT_MAX/YRM100_FRAME_POLL_NOTICE_SIZE)) {
        return YRM100_ERROR_BUFFER_OVERFLOW;
    }

    unsigned short message_count = (unsigned short)(response_len / YRM100_FRAME_POLL_NOTICE_SIZE);
    if (message_count > maximum_tag_count)
    {
        message_count = maximum_tag_count;
    }
    rfid_tag_t *t;

    for (unsigned short n = 0; n < maximum_tag_count; n++)
    {
        t = &tags[n];
        for (size_t i = 0; i < YRM100_TAG_EPC_BYTE_COUNT; i++)
        {
            t->epc[i] = 0;
        }
        t->rssi = 0;
        t->pc = 0;
        t->crc = 0;
    }

    for (unsigned short n = 0; n < message_count; n++)
    {
        pos = YRM100_FRAME_POLL_NOTICE_SIZE * n;
        t = &tags[n];
        if (response[pos + 0] == 0xBB && response[pos + 1] == 0x02)
        {
            for (size_t i = 0; i < YRM100_TAG_EPC_BYTE_COUNT; i++)
            {
                t->epc[i] = response[pos + 8 + i];
            }
            t->rssi = (signed char)response[pos + 5];
            t->pc = (unsigned short)(((unsigned short)(response[pos + 6]) << 8) | ((unsigned short)response[pos + 7]));
            t->crc = (unsigned short)(((unsigned short)(response[pos + 20]) << 8) | ((unsigned short)response[pos + 21]));
        }
        else
        {
            t->pc = 0;
            t->crc = 0;
            t->rssi = 0;
            for (int i = 0; i < YRM100_TAG_EPC_BYTE_COUNT; i++)
            {
                t->epc[i] = 0;
            }
        }
    }
    return message_count;
}

int yrm100_parse_get_error_code(unsigned char *buf, size_t buf_size)
{
    if (buf_size < YRM100_FRAME_MINIMUM_RESPONSE_SIZE)
    {
        return YRM100_ERROR_PARSE_ERROR;
    }
    if (yrm100_frame_is_error_response(buf, buf_size))
    {
        return buf[YRM100_FRAME_ERROR_RESPONSE_CODE_POSITION];
    }
    return YRM100_ERROR_PARSE_ERROR;
}
