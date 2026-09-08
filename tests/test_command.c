#include <stdio.h>
#include <string.h>
#include "yrm100/yrm100.h"
#include "yrm100/yrm100_command.h"
#include "yrm100/yrm100_error.h"
#include "yrm100/yrm100_frame.h"
#include "test_serial.h"

static int expect_true(const char *label, int condition)
{
    if (!condition)
    {
        printf("FAIL: %s\n", label);
        return 1;
    }
    return 0;
}

static void feed(const unsigned char *bytes, size_t length)
{
    size_t chunks[] = {length};
    test_serial_set_read_data(bytes, length, chunks, 1);
}

static size_t make_response(unsigned char *bytes, unsigned char command, size_t payload_length)
{
    size_t length = payload_length + 7;
    bytes[0] = 0xBB;
    bytes[1] = 1;
    bytes[2] = command;
    bytes[3] = (unsigned char)(payload_length >> 8);
    bytes[4] = (unsigned char)payload_length;
    bytes[length - 2] = (unsigned char)yrm100_frame_calculate_checksum(bytes, length);
    bytes[length - 1] = 0x7E;
    return length;
}

int test_command_functions(void)
{
    int failures = 0;
    unsigned char bytes[1200] = {0};
    yrm100_query_parameters_t query = {0};
    test_serial_reset();
    yrm100_context_t *device = yrm100_init("mock");
    if (device == NULL)
    {
        return 1;
    }

    feed(bytes, 1100);
    failures += expect_true("noise-only stream times out safely",
        yrm100_command_get_tx_power(device) == YRM100_ERROR_READ_TIMEOUT);
    bytes[1105] = 0x07;
    bytes[1106] = 0xD0;
    size_t length = make_response(bytes + 1100, 0xB7, 2);
    feed(bytes, 1100 + length);
    failures += expect_true("valid response after discarded noise",
        yrm100_command_get_tx_power(device) == 2000);

    memset(bytes, 0, sizeof(bytes));
    length = make_response(bytes, 0xB7, 1017);
    feed(bytes, length);
    failures += expect_true("full buffer response reaches payload validation",
        yrm100_command_get_tx_power(device) == YRM100_ERROR_PARSE_ERROR);

    length = make_response(bytes, 0x22, 1015);
    bytes[1] = 2;
    bytes[length - 2] = (unsigned char)yrm100_frame_calculate_checksum(bytes, length);
    length += make_response(bytes + length, 0xB7, 2);
    feed(bytes, length);
    failures += expect_true("following frame prefix cannot overrun remaining space",
        yrm100_command_get_tx_power(device) == YRM100_ERROR_SERIAL_INPUT_OVERFLOW);

    const unsigned char commands[] = {0xB7, 0x08, 0x0D};
    const size_t payload_lengths[] = {2, 1, 2};
    for (size_t i = 0; i < sizeof(commands); i++)
    {
        for (size_t payload = 0; payload <= 3; payload++)
        {
            for (int wrong_command = 0; wrong_command <= 1; wrong_command++)
            {
                memset(bytes, 0, sizeof(bytes));
                bytes[5] = i == 1 ? 3 : 0;
                length = make_response(bytes, wrong_command ? 0x07 : commands[i], payload);
                feed(bytes, length);
                int result;
                if (i == 0)
                {
                    result = yrm100_command_get_tx_power(device);
                }
                else if (i == 1)
                {
                    result = yrm100_command_get_operating_region(device);
                }
                else
                {
                    result = yrm100_command_get_query_parameters(device, &query);
                }
                if (wrong_command || payload != payload_lengths[i])
                {
                    failures += expect_true("getter rejects wrong command or payload length",
                        result == YRM100_ERROR_PARSE_ERROR && device->last_error_code == result);
                }
                else
                {
                    failures += expect_true("getter accepts matching response",
                        result == (i == 1 ? 3 : 0) && device->last_error_code == YRM100_STATUS_OK);
                }
            }
        }
    }
    yrm100_deinit(device);
    return failures;
}
