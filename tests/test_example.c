#include <stdio.h>
#include <string.h>
#include "yrm100/yrm100_command.h"
#include "yrm100/yrm100_error.h"
#include "yrm100/yrm100_types.h"

ssize_t yrm100_command_read_response(yrm100_context_t *device_context);
void test_serial_set_read_data(const unsigned char *data, size_t len, const size_t *chunks, size_t chunk_count);

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

    unsigned char response[] = {0xBB, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x22, 0x7E};
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

static int test_overflow_read(void)
{
    yrm100_context_t ctx;
    memset(&ctx, 0, sizeof(ctx));
    ctx.serial_port_name = "mock";
    ctx.serial_port = (serial_port_t)1;
    ctx.is_initialized = true;

    unsigned char response[YRM100_COMMAND_RESPONSE_BUFFER_SIZE + 1];
    memset(response, 0xAA, sizeof(response));
    size_t chunks[] = {YRM100_COMMAND_RESPONSE_BUFFER_SIZE, 1};
    test_serial_set_read_data(response, sizeof(response), chunks, 2);

    ssize_t read_len = yrm100_command_read_response(&ctx);
    return expect_equal_int("overflow read error", (int)read_len, YRM100_ERROR_SERIAL_INPUT_OVERFLOW);
}

int main(void)
{
    int failures = 0;
    failures += test_fragmented_read();
    failures += test_overflow_read();
    if (failures == 0)
    {
        printf("OK\n");
        return 0;
    }
    return 1;
}
