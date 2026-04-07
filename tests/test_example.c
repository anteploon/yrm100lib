#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "yrm100/yrm100_command.h"
#include "yrm100/yrm100_error.h"
#include "yrm100/yrm100_frame.h"
#include "yrm100/yrm100_parse.h"
#include "yrm100/yrm100_types.h"

ssize_t yrm100_command_read_response(yrm100_context_t *device_context);
void test_serial_set_read_data(const unsigned char *data, size_t len, const size_t *chunks, size_t chunk_count);
int test_string_functions(void);

static int expect_equal_int(const char *label, int got, int expected)
{
    if (got != expected)
    {
        printf("FAIL: %s expected %d got %d\n", label, expected, got);
        return 1;
    }
    return 0;
}

static int test_fragmented_read(void)
{
    yrm100_context_t ctx;
    memset(&ctx, 0, sizeof(ctx));
    ctx.serial_port_name = "mock";
    ctx.serial_port = (serial_port_t)1;
    ctx.is_initialized = true;

    unsigned char response[] = {0xBB, 0x01, 0x00, 0x00, 0x03, 0x00, 0x00, 0x11, 0x00, 0x7E};
    response[8] = (unsigned char)yrm100_frame_calculate_checksum(response, sizeof(response));
    size_t chunks[] = {4, 6};
    test_serial_set_read_data(response, sizeof(response), chunks, 2);

    ssize_t read_len = yrm100_command_read_response(&ctx);
    int failures = 0;
    failures += expect_equal_int("fragmented read length", (int)read_len, (int)sizeof(response));
    if (read_len == (ssize_t)sizeof(response) &&
        memcmp(ctx.command_response_buf, response, sizeof(response)) != 0)
    {
        printf("FAIL: fragmented read buffer mismatch\n");
        failures++;
    }
    return failures;
}

static int test_payload_end_byte_at_chunk_boundary(void)
{
    yrm100_context_t ctx;
    memset(&ctx, 0, sizeof(ctx));
    ctx.serial_port_name = "mock";
    ctx.serial_port = (serial_port_t)1;
    ctx.is_initialized = true;

    unsigned char response[] = {0xBB, 0x01, 0x00, 0x00, 0x03, 0x12, 0x7E, 0x34, 0x00, 0x7E};
    response[8] = (unsigned char)yrm100_frame_calculate_checksum(response, sizeof(response));
    size_t chunks[] = {5, 2, 3};
    test_serial_set_read_data(response, sizeof(response), chunks, 3);

    ssize_t read_len = yrm100_command_read_response(&ctx);
    int failures = 0;
    failures += expect_equal_int("payload end byte read length", (int)read_len, (int)sizeof(response));
    if (read_len == (ssize_t)sizeof(response) &&
        memcmp(ctx.command_response_buf, response, sizeof(response)) != 0)
    {
        printf("FAIL: payload end byte buffer mismatch\n");
        failures++;
    }
    return failures;
}

static int test_overflow_read(void)
{
    yrm100_context_t ctx;
    memset(&ctx, 0, sizeof(ctx));
    ctx.serial_port_name = "mock";
    ctx.serial_port = (serial_port_t)1;
    ctx.is_initialized = true;

    unsigned char response[] = {0xBB, 0x01, 0x00, 0x04, 0x00};
    size_t chunks[] = {sizeof(response)};
    test_serial_set_read_data(response, sizeof(response), chunks, 1);

    ssize_t read_len = yrm100_command_read_response(&ctx);
    return expect_equal_int("overflow read error", (int)read_len, YRM100_ERROR_SERIAL_INPUT_OVERFLOW);
}

static int test_partial_read_without_end_byte(void)
{
    yrm100_context_t ctx;
    memset(&ctx, 0, sizeof(ctx));
    ctx.serial_port_name = "mock";
    ctx.serial_port = (serial_port_t)1;
    ctx.is_initialized = true;

    unsigned char response[] = {0xBB, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00};
    size_t chunks[] = {sizeof(response)};
    test_serial_set_read_data(response, sizeof(response), chunks, 1);

    ssize_t read_len = yrm100_command_read_response(&ctx);
    return expect_equal_int("partial read missing end byte", (int)read_len, YRM100_ERROR_PARSE_ERROR);
}

static int test_invalid_end_byte_checksum_validation(void)
{
    unsigned char response[] = {0xBB, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00};
    response[6] = (unsigned char)yrm100_frame_calculate_checksum(response, sizeof(response));
    response[7] = 0x00;
    return expect_equal_int("invalid end byte", yrm100_frame_is_valid_response(response, sizeof(response)), 0);
}

static int test_error_response_checksum_validation(void)
{
    int failures = 0;
    unsigned char response[] = {0xBB, 0x01, 0xFF, 0x00, 0x01, YRM100_MODULE_ERROR_READ_FAIL, 0x00, 0x7E};

    response[6] = (unsigned char)yrm100_frame_calculate_checksum(response, sizeof(response));
    failures += expect_equal_int("valid error response", yrm100_frame_is_error_response(response, sizeof(response)), 1);

    response[6]++;
    failures += expect_equal_int("invalid error response checksum", yrm100_frame_is_error_response(response, sizeof(response)), 0);

    return failures;
}

static void set_poll_notice_checksum(unsigned char *response)
{
    response[YRM100_FRAME_POLL_NOTICE_SIZE - 2] = (unsigned char)yrm100_frame_calculate_checksum(response, YRM100_FRAME_POLL_NOTICE_SIZE);
}

static int test_poll_response_validation(void)
{
    int failures = 0;
    yrm100_rfid_tag_t tags[2];
    unsigned char two_responses[YRM100_FRAME_POLL_NOTICE_SIZE * 2];
    unsigned char response[] = {
        0xBB, 0x02, 0x22, 0x00, 0x11, 0xC8, 0x30, 0x00,
        0xE2, 0x00, 0x00, 0x17, 0x22, 0x11, 0x44, 0x55,
        0x66, 0x77, 0x88, 0x99, 0xAB, 0xCD, 0x00, 0x7E
    };

    memset(tags, 0, sizeof(tags));
    tags[0].data = malloc(1);
    if (tags[0].data == NULL)
    {
        return expect_equal_int("poll response data allocation", 0, 1);
    }
    tags[0].data_length = 1;
    set_poll_notice_checksum(response);
    failures += expect_equal_int("valid poll response", yrm100_parse_poll_response(response, sizeof(response), tags, 2), 1);
    failures += expect_equal_int("poll response rssi", tags[0].rssi, (signed char)0xC8);
    failures += expect_equal_int("poll response pc", tags[0].pc, 0x3000);
    failures += expect_equal_int("poll response crc", tags[0].crc, 0xABCD);
    failures += expect_equal_int("poll response data reset", tags[0].data == NULL, 1);
    failures += expect_equal_int("poll response data length reset", (int)tags[0].data_length, 0);

    failures += expect_equal_int("short poll response", yrm100_parse_poll_response(response, sizeof(response) - 1, tags, 2), YRM100_ERROR_PARSE_ERROR);

    response[YRM100_FRAME_POLL_NOTICE_SIZE - 2]++;
    failures += expect_equal_int("invalid poll response checksum", yrm100_parse_poll_response(response, sizeof(response), tags, 2), YRM100_ERROR_PARSE_ERROR);

    set_poll_notice_checksum(response);
    memcpy(two_responses, response, sizeof(response));
    memcpy(&two_responses[YRM100_FRAME_POLL_NOTICE_SIZE], response, sizeof(response));
    two_responses[(YRM100_FRAME_POLL_NOTICE_SIZE * 2) - 2]++;
    failures += expect_equal_int("invalid capped poll response", yrm100_parse_poll_response(two_responses, sizeof(two_responses), tags, 1), YRM100_ERROR_PARSE_ERROR);

    return failures;
}

static int test_single_poll_error_response(void)
{
    yrm100_context_t ctx;
    memset(&ctx, 0, sizeof(ctx));
    ctx.serial_port_name = "mock";
    ctx.serial_port = (serial_port_t)1;
    ctx.is_initialized = true;

    unsigned char response[] = {0xBB, 0x01, 0xFF, 0x00, 0x01, YRM100_MODULE_ERROR_READ_FAIL, 0x00, 0x7E};
    response[6] = (unsigned char)yrm100_frame_calculate_checksum(response, sizeof(response));
    size_t chunks[] = {sizeof(response)};
    test_serial_set_read_data(response, sizeof(response), chunks, 1);

    yrm100_rfid_tag_t tags[1];
    memset(tags, 0, sizeof(tags));

    int result = yrm100_command_single_poll(&ctx, tags, 1);
    return expect_equal_int("single poll error response", result, YRM100_MODULE_ERROR_READ_FAIL);
}

static int test_single_poll_nonmultiple_notice_response(void)
{
    yrm100_context_t ctx;
    memset(&ctx, 0, sizeof(ctx));
    ctx.serial_port_name = "mock";
    ctx.serial_port = (serial_port_t)1;
    ctx.is_initialized = true;

    unsigned char response[] = {
        0xBB, 0x02, 0x22, 0x00, 0x12, 0xC8, 0x30, 0x00,
        0xE2, 0x00, 0x00, 0x17, 0x22, 0x11, 0x44, 0x55,
        0x66, 0x77, 0x88, 0x99, 0xAB, 0xCD, 0x00, 0x00, 0x7E
    };
    response[sizeof(response) - 2] = (unsigned char)yrm100_frame_calculate_checksum(response, sizeof(response));
    size_t chunks[] = {sizeof(response)};
    test_serial_set_read_data(response, sizeof(response), chunks, 1);

    yrm100_rfid_tag_t tags[1];
    memset(tags, 0, sizeof(tags));

    int result = yrm100_command_single_poll(&ctx, tags, 1);
    return expect_equal_int("single poll nonmultiple notice", result, YRM100_ERROR_PARSE_ERROR);
}

static int test_get_tx_power_error_response(void)
{
    yrm100_context_t ctx;
    memset(&ctx, 0, sizeof(ctx));
    ctx.serial_port_name = "mock";
    ctx.serial_port = (serial_port_t)1;
    ctx.is_initialized = true;

    unsigned char response[] = {0xBB, 0x01, 0xFF, 0x00, 0x01, YRM100_MODULE_ERROR_READ_FAIL, 0x00, 0x7E};
    response[6] = (unsigned char)yrm100_frame_calculate_checksum(response, sizeof(response));
    size_t chunks[] = {sizeof(response)};
    test_serial_set_read_data(response, sizeof(response), chunks, 1);

    int result = yrm100_command_get_tx_power(&ctx);
    return expect_equal_int("get tx power error response", result, YRM100_MODULE_ERROR_READ_FAIL);
}

int main(void)
{
    int failures = 0;
    failures += test_fragmented_read();
    failures += test_payload_end_byte_at_chunk_boundary();
    failures += test_overflow_read();
    failures += test_partial_read_without_end_byte();
    failures += test_invalid_end_byte_checksum_validation();
    failures += test_error_response_checksum_validation();
    failures += test_poll_response_validation();
    failures += test_single_poll_error_response();
    failures += test_single_poll_nonmultiple_notice_response();
    failures += test_get_tx_power_error_response();
    failures += test_string_functions();
    if (failures == 0)
    {
        printf("OK\n");
        return 0;
    }
    return 1;
}
