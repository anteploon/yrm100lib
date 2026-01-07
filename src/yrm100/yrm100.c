#include <string.h>
#include <stdlib.h>
#include "yrm100.h"
#include "yrm100_error.h"

yrm100_context_t *yrm100_init(const char *port_name)
{
    yrm100_context_t *device_context = calloc(1, sizeof(*device_context));
    if (device_context != NULL)
    {
        device_context->last_error_code = YRM100_STATUS_OK;
        device_context->serial_port_name = strdup(port_name);
        if (device_context->serial_port_name == NULL)
        {
            free(device_context);
            return NULL;
        }
        device_context->serial_port = yrm100_serial_open(port_name);
#ifdef _WIN32
        if (device_context->serial_port == INVALID_HANDLE_VALUE)
#else
        if (device_context->serial_port < 0)
#endif
        {
            free(device_context->serial_port_name);
            free(device_context);
            return NULL;
        }
        if (yrm100_serial_configure(device_context->serial_port) != YRM100_STATUS_OK)
        {
            yrm100_serial_close(device_context->serial_port);
            free(device_context->serial_port_name);
            free(device_context);
            return NULL;
        }
        device_context->is_multi_poll_running = false;
        device_context->multi_poll_maximum_tag_count = 0;
        device_context->multi_poll_tag_count = 0;
        device_context->multi_poll_target = NULL;
        device_context->is_initialized = true;
    }
    return device_context;
}

int yrm100_deinit(yrm100_context_t *device_context)
{
    if (device_context != NULL)
    {
        if (device_context->is_initialized)
        {
            yrm100_serial_close(device_context->serial_port);
        }
        free(device_context->serial_port_name);
        free(device_context);
    }
    return YRM100_STATUS_OK;
}

bool yrm100_is_device_context_valid(yrm100_context_t *device_context)
{
    if (device_context != NULL &&
        device_context->serial_port_name != NULL &&
        device_context->is_initialized == true)
    {
        return true;
    }
    return false;
}
