#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "yrm100/yrm100_error.h"
#include "yrm100/yrm100_frame.h"
#include "yrm100/yrm100_parse.h"

static int expect_true(const char *label, int condition)
{
    if (!condition)
    {
        printf("FAIL: %s\n", label);
        return 1;
    }
    return 0;
}

static void finalize_frame(uint8_t *frame, size_t frame_size)
{
    frame[frame_size - 2] = (uint8_t)yrm100_frame_calculate_checksum(frame, frame_size);
    frame[frame_size - 1] = YRM100_FRAME_END_BYTE;
}

int test_parse_functions(void)
{
    int failures = 0;
    char string_buf[16];
    yrm100_rfid_tag_t tags[2];
    yrm100_rfid_tag_t tag;
    uint8_t ascii_response[] = {
        0xBB, 0x01, 0x03, 0x00, 0x06, 0x00, 'H', 'E', 'L', 'L', 'O', 0x00, 0x00
    };
    uint8_t poll_notice[YRM100_FRAME_POLL_NOTICE_SIZE] = {
        0xBB, 0x02, 0x22, 0x00, 0x13, 0xD6, 0x12, 0x34,
        0xDE, 0xAD, 0xBE, 0xEF, 0x01, 0x23, 0x45, 0x67,
        0x89, 0xAB, 0xCD, 0xEF, 0xAB, 0xCD, 0x00, 0x00
    };
    uint8_t read_response[] = {
        0xBB, 0x01, 0x39, 0x00, 0x11, 0x00, 0x12, 0x34,
        0xDE, 0xAD, 0xBE, 0xEF, 0x01, 0x23, 0x45, 0x67,
        0x89, 0xAB, 0xCD, 0xEF, 0x11, 0x22, 0x33, 0x44, 0x00
    };

    memset(tags, 0, sizeof(tags));
    finalize_frame(ascii_response, sizeof(ascii_response));
    failures += expect_true(
        "parse ascii response",
        yrm100_parse_ascii_response(ascii_response, sizeof(ascii_response), string_buf, sizeof(string_buf)) == YRM100_STATUS_OK);
    failures += expect_true("ascii payload copied", strcmp(string_buf, "HELLO") == 0);
    failures += expect_true(
        "ascii rejects small output buffer",
        yrm100_parse_ascii_response(ascii_response, sizeof(ascii_response), string_buf, 5) == YRM100_ERROR_BUFFER_TOO_SMALL);

    finalize_frame(poll_notice, sizeof(poll_notice));
    failures += expect_true(
        "parse poll response",
        yrm100_parse_poll_response(poll_notice, sizeof(poll_notice), tags, 2) == 1);
    failures += expect_true("poll tag rssi parsed", tags[0].rssi == -42);
    failures += expect_true("poll tag pc parsed", tags[0].pc == 0x1234);
    failures += expect_true("poll tag crc parsed", tags[0].crc == 0xABCD);
    failures += expect_true(
        "poll tag epc parsed",
        memcmp(tags[0].epc, &poll_notice[8], YRM100_TAG_EPC_BYTE_COUNT) == 0);

    memset(&tag, 0, sizeof(tag));
    failures += expect_true(
        "parse read tag memory response",
        yrm100_parse_read_tag_memory_response(read_response, sizeof(read_response), &tag, 2) == YRM100_STATUS_OK);
    failures += expect_true("read tag pc parsed", tag.pc == 0x1234);
    failures += expect_true("read tag data length parsed", tag.data_length == 4);
    failures += expect_true("read tag data allocated", tag.data != NULL);
    if (tag.data != NULL)
    {
        failures += expect_true("read tag data copied", memcmp(tag.data, &read_response[20], 4) == 0);
        free(tag.data);
    }

    return failures;
}
