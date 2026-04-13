#include <stdio.h>
#include <string.h>
#include "yrm100/yrm100.h"
#include "yrm100/yrm100_error.h"
#include "yrm100/yrm100_types.h"
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

int test_context_functions(void)
{
    int failures = 0;
    yrm100_context_t *device_context;

    test_serial_reset();

    failures += expect_true("init rejects null port", yrm100_init(NULL) == NULL);

    device_context = yrm100_init("/dev/ttyUSB0");
    failures += expect_true("init succeeds with stub serial", device_context != NULL);
    if (device_context != NULL)
    {
        failures += expect_true("device context marked initialized", device_context->is_initialized);
        failures += expect_true("device context has copied serial port name", strcmp(device_context->serial_port_name, "/dev/ttyUSB0") == 0);
        failures += expect_true("device context starts with OK status", device_context->last_error_code == YRM100_STATUS_OK);
        failures += expect_true("device context validates", yrm100_is_device_context_valid(device_context));
        failures += expect_true("deinit returns OK", yrm100_deinit(device_context) == YRM100_STATUS_OK);
    }

    test_serial_reset();
    test_serial_set_open_result((serial_port_t)-1);
    failures += expect_true("init fails when open fails", yrm100_init("/dev/ttyUSB0") == NULL);

    test_serial_reset();
    test_serial_set_configure_result(YRM100_ERROR_SERIAL_TCSETATTR_FAILED);
    failures += expect_true("init fails when configure fails", yrm100_init("/dev/ttyUSB0") == NULL);

    return failures;
}
