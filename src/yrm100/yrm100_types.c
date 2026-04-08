#include "yrm100_types.h"
#include "yrm100_param.h"

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
