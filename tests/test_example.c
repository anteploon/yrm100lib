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
size_t test_serial_get_last_write(unsigned char *buffer, size_t size);
int test_string_functions(void);
static void set_poll_notice_checksum(unsigned char *response);

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

static int test_notice_only_read(void)
{
    yrm100_context_t ctx;
    memset(&ctx, 0, sizeof(ctx));
    ctx.serial_port_name = "mock";
    ctx.serial_port = (serial_port_t)1;
    ctx.is_initialized = true;

    unsigned char response[] = {
        0xBB, 0x02, 0x22, 0x00, 0x11, 0xC8, 0x30, 0x00,
        0xE2, 0x00, 0x00, 0x17, 0x22, 0x11, 0x44, 0x55,
        0x66, 0x77, 0x88, 0x99, 0xAB, 0xCD, 0x00, 0x7E
    };
    size_t chunks[] = {5, sizeof(response) - 5};

    set_poll_notice_checksum(response);
    test_serial_set_read_data(response, sizeof(response), chunks, 2);

    ssize_t read_len = yrm100_command_read_response(&ctx);
    int failures = 0;
    failures += expect_equal_int("notice only read length", (int)read_len, (int)sizeof(response));
    if (read_len == (ssize_t)sizeof(response) &&
        memcmp(ctx.command_response_buf, response, sizeof(response)) != 0)
    {
        printf("FAIL: notice only read buffer mismatch\n");
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

static int test_read_tag_memory_response_validation(void)
{
    int failures = 0;
    unsigned char response[] = {
        0xBB, 0x01, 0x39, 0x00, 0x11, 0x00, 0x30, 0x00,
        0xE2, 0x00, 0x00, 0x17, 0x22, 0x11, 0x44, 0x55,
        0x66, 0x77, 0x88, 0x99, 0x12, 0x34, 0x00, 0x7E
    };

    response[sizeof(response) - 2] = (unsigned char)yrm100_frame_calculate_checksum(response, sizeof(response));
    failures += expect_equal_int("valid read tag memory response", yrm100_frame_is_read_tag_memory_response(response, sizeof(response)), 1);

    response[YRM100_FRAME_BYTE_POSITION_COMMAND] = 0x00;
    response[sizeof(response) - 2] = (unsigned char)yrm100_frame_calculate_checksum(response, sizeof(response));
    failures += expect_equal_int("wrong read tag memory command", yrm100_frame_is_read_tag_memory_response(response, sizeof(response)), 0);

    response[YRM100_FRAME_BYTE_POSITION_COMMAND] = YRM100_FRAME_COMMAND_READ_TAG_MEMORY_AREA;
    response[sizeof(response) - 2]++;
    failures += expect_equal_int("invalid read tag memory checksum", yrm100_frame_is_read_tag_memory_response(response, sizeof(response)), 0);

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

static int test_single_poll_success_response(void)
{
    int failures = 0;
    yrm100_context_t ctx;
    yrm100_rfid_tag_t tags[2];
    unsigned char response[] = {
        0xBB, 0x02, 0x22, 0x00, 0x11, 0xC8, 0x30, 0x00,
        0xE2, 0x00, 0x00, 0x17, 0x22, 0x11, 0x44, 0x55,
        0x66, 0x77, 0x88, 0x99, 0xAB, 0xCD, 0x00, 0x7E,
        0xBB, 0x01, 0x22, 0x00, 0x01, 0x00, 0x23, 0x7E
    };
    size_t chunks[] = {24, 8};

    memset(&ctx, 0, sizeof(ctx));
    ctx.serial_port_name = "mock";
    ctx.serial_port = (serial_port_t)1;
    ctx.is_initialized = true;

    memset(tags, 0, sizeof(tags));
    response[22] = (unsigned char)yrm100_frame_calculate_checksum(response, YRM100_FRAME_POLL_NOTICE_SIZE);
    response[30] = (unsigned char)yrm100_frame_calculate_checksum(&response[YRM100_FRAME_POLL_NOTICE_SIZE], 8);
    test_serial_set_read_data(response, sizeof(response), chunks, 2);

    failures += expect_equal_int(
        "single poll success result",
        yrm100_command_single_poll(&ctx, tags, 2),
        YRM100_STATUS_OK);
    failures += expect_equal_int("single poll success rssi", tags[0].rssi, (signed char)0xC8);
    failures += expect_equal_int("single poll success pc", tags[0].pc, 0x3000);
    failures += expect_equal_int("single poll success crc", tags[0].crc, 0xABCD);
    if (memcmp(tags[0].epc, &response[8], YRM100_TAG_EPC_BYTE_COUNT) != 0)
    {
        printf("FAIL: single poll success EPC mismatch\n");
        failures++;
    }
    failures += expect_equal_int("single poll second tag rssi", tags[1].rssi, 0);
    failures += expect_equal_int("single poll second tag pc", tags[1].pc, 0);
    failures += expect_equal_int("single poll second tag crc", tags[1].crc, 0);

    return failures;
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

static int test_single_poll_notice_only_success(void)
{
    int failures = 0;
    yrm100_context_t ctx;
    yrm100_rfid_tag_t tags[2];
    unsigned char response[] = {
        0xBB, 0x02, 0x22, 0x00, 0x11, 0xC8, 0x30, 0x00,
        0xE2, 0x00, 0x00, 0x17, 0x22, 0x11, 0x44, 0x55,
        0x66, 0x77, 0x88, 0x99, 0xAB, 0xCD, 0x00, 0x7E
    };
    size_t chunks[] = {7, sizeof(response) - 7};

    memset(&ctx, 0, sizeof(ctx));
    ctx.serial_port_name = "mock";
    ctx.serial_port = (serial_port_t)1;
    ctx.is_initialized = true;

    memset(tags, 0, sizeof(tags));
    set_poll_notice_checksum(response);
    test_serial_set_read_data(response, sizeof(response), chunks, 2);

    failures += expect_equal_int(
        "single poll notice only result",
        yrm100_command_single_poll(&ctx, tags, 2),
        YRM100_STATUS_OK);
    failures += expect_equal_int("single poll notice only rssi", tags[0].rssi, (signed char)0xC8);
    failures += expect_equal_int("single poll notice only pc", tags[0].pc, 0x3000);
    failures += expect_equal_int("single poll notice only crc", tags[0].crc, 0xABCD);
    if (memcmp(tags[0].epc, &response[8], YRM100_TAG_EPC_BYTE_COUNT) != 0)
    {
        printf("FAIL: single poll notice only EPC mismatch\n");
        failures++;
    }

    return failures;
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

static int test_get_query_parameters_success(void)
{
    int failures = 0;
    yrm100_context_t ctx;
    yrm100_query_parameters_t query_parameters;
    unsigned char response[] = {0xBB, 0x01, 0x0D, 0x00, 0x02, 0x0B, 0x73, 0x00, 0x7E};
    size_t chunks[] = {sizeof(response)};

    memset(&ctx, 0, sizeof(ctx));
    ctx.serial_port_name = "mock";
    ctx.serial_port = (serial_port_t)1;
    ctx.is_initialized = true;

    memset(&query_parameters, 0, sizeof(query_parameters));
    response[7] = (unsigned char)yrm100_frame_calculate_checksum(response, sizeof(response));
    test_serial_set_read_data(response, sizeof(response), chunks, 1);

    failures += expect_equal_int(
        "get query parameters result",
        yrm100_command_get_query_parameters(&ctx, &query_parameters),
        YRM100_STATUS_OK);
    failures += expect_equal_int("get query parameters dr", query_parameters.dr, 1);
    failures += expect_equal_int("get query parameters m", query_parameters.m, 1);
    failures += expect_equal_int("get query parameters trext", query_parameters.trext, 0);
    failures += expect_equal_int("get query parameters sel", query_parameters.sel, 3);
    failures += expect_equal_int("get query parameters session", query_parameters.session, 1);
    failures += expect_equal_int("get query parameters target", query_parameters.target, 1);
    failures += expect_equal_int("get query parameters q", query_parameters.q, 5);

    return failures;
}

static int test_set_select_parameters_writes_mask(void)
{
    int failures = 0;
    yrm100_context_t ctx;
    yrm100_select_parameters_t select_parameters;
    unsigned char command_buf[32];
    unsigned char response[] = {0xBB, 0x01, 0x0C, 0x00, 0x01, 0x00, 0x00, 0x7E};
    size_t chunks[] = {sizeof(response)};

    memset(&ctx, 0, sizeof(ctx));
    ctx.serial_port_name = "mock";
    ctx.serial_port = (serial_port_t)1;
    ctx.is_initialized = true;

    memset(&select_parameters, 0, sizeof(select_parameters));
    select_parameters.target = 1;
    select_parameters.action = 2;
    select_parameters.membank = YRM100_PARAM_MEMBANK_EPC;
    select_parameters.pointer = 0x00000020;
    select_parameters.length = 0x60;
    select_parameters.truncate = 0x00;
    for (size_t i = 0; i < YRM100_TAG_EPC_BYTE_COUNT; i++)
    {
        select_parameters.mask[i] = (unsigned char)(0xA0U + i);
    }

    response[6] = (unsigned char)yrm100_frame_calculate_checksum(response, sizeof(response));
    test_serial_set_read_data(response, sizeof(response), chunks, 1);

    failures += expect_equal_int(
        "set select parameters result",
        yrm100_command_set_select_parameters(&ctx, &select_parameters),
        YRM100_STATUS_OK);
    failures += expect_equal_int(
        "set select parameters write length",
        (int)test_serial_get_last_write(command_buf, sizeof(command_buf)),
        26);
    if (memcmp(&command_buf[12], select_parameters.mask, YRM100_TAG_EPC_BYTE_COUNT) != 0)
    {
        printf("FAIL: set select parameters mask mismatch\n");
        failures++;
    }
    failures += expect_equal_int(
        "set select parameters checksum",
        command_buf[24],
        yrm100_frame_calculate_checksum(command_buf, 26));
    return failures;
}

static int test_get_select_parameters_reads_mask(void)
{
    int failures = 0;
    yrm100_context_t ctx;
    yrm100_select_parameters_t select_parameters;
    unsigned char response[] = {
        0xBB, 0x01, 0x0B, 0x00, 0x13, 0x51, 0x12, 0x34,
        0x56, 0x78, 0x60, 0x01, 0xA0, 0xA1, 0xA2, 0xA3,
        0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB,
        0x00, 0x7E
    };
    size_t chunks[] = {sizeof(response)};
    unsigned char expected_mask[YRM100_TAG_EPC_BYTE_COUNT] = {
        0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5,
        0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB
    };

    memset(&ctx, 0, sizeof(ctx));
    ctx.serial_port_name = "mock";
    ctx.serial_port = (serial_port_t)1;
    ctx.is_initialized = true;

    memset(&select_parameters, 0xCC, sizeof(select_parameters));

    response[24] = (unsigned char)yrm100_frame_calculate_checksum(response, sizeof(response));
    test_serial_set_read_data(response, sizeof(response), chunks, 1);

    failures += expect_equal_int(
        "get select parameters result",
        yrm100_command_get_select_parameters(&ctx, &select_parameters),
        YRM100_STATUS_OK);
    failures += expect_equal_int("get select parameters target", select_parameters.target, 1);
    failures += expect_equal_int("get select parameters action", select_parameters.action, 2);
    failures += expect_equal_int("get select parameters membank", select_parameters.membank, YRM100_PARAM_MEMBANK_EPC);
    failures += expect_equal_int("get select parameters pointer", (int)select_parameters.pointer, 0x12345678);
    failures += expect_equal_int("get select parameters length", select_parameters.length, 0x60);
    failures += expect_equal_int("get select parameters truncate", select_parameters.truncate, 0x01);
    if (memcmp(select_parameters.mask, expected_mask, sizeof(expected_mask)) != 0)
    {
        printf("FAIL: get select parameters mask mismatch\n");
        failures++;
    }

    return failures;
}

int main(void)
{
    int failures = 0;
    failures += test_fragmented_read();
    failures += test_payload_end_byte_at_chunk_boundary();
    failures += test_notice_only_read();
    failures += test_overflow_read();
    failures += test_partial_read_without_end_byte();
    failures += test_invalid_end_byte_checksum_validation();
    failures += test_error_response_checksum_validation();
    failures += test_read_tag_memory_response_validation();
    failures += test_poll_response_validation();
    failures += test_single_poll_error_response();
    failures += test_single_poll_success_response();
    failures += test_single_poll_nonmultiple_notice_response();
    failures += test_single_poll_notice_only_success();
    failures += test_get_tx_power_error_response();
    failures += test_get_query_parameters_success();
    failures += test_set_select_parameters_writes_mask();
    failures += test_get_select_parameters_reads_mask();
    failures += test_string_functions();
    if (failures == 0)
    {
        printf("OK\n");
        return 0;
    }
    return 1;
}
