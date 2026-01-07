#ifndef YRM100_H
#define YRM100_H

#include <sys/types.h>
#include <stdbool.h>
#include "yrm100_types.h"
#include "yrm100_serial.h"
#include "yrm100_param.h"

/**
 * @brief Initializes the RFID UHF device context
 * @param port_name Serial port path or name (for example in Linux: "/dev/ttyUSB0" or in Windows: "COM7")
 * @return RFID UHF device context on success or NULL on error
 */
yrm100_context_t *rfid_uhf_init(const char *port_name);

/**
 * @brief Frees RFID UHF device context resources and closes the serial port
 * @param device_context Device context
 * @return Negative value on failure
 */
int yrm100_deinit(rfid_uhf_context_t *device_context);

/**
 * @brief Checks if the RFID UHF device context looks like a valid one
 * @param device_context Device context
 * @return True if the device context is valid
 */
bool yrm100_is_device_context_valid(rfid_uhf_context_t *device_context);

#endif
