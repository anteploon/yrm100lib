#ifndef YRM100_SERIAL_H
#define YRM100_SERIAL_H

#include <sys/types.h>
#include "yrm100_error.h"

#ifdef _WIN32
#include <windows.h>
typedef HANDLE serial_port_t;
#else
typedef int serial_port_t;
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#include <stddef.h> // for size_t

    /**
     * @brief Opens a serial port
     * @param port_name Serial port path or name (for example in Linux: "/dev/ttyUSB0" or in Windows: "COM3")
     * @return Serial port handle, or an invalid handle on error
     */
serial_port_t yrm100_serial_open(const char *port_name);

    /**
     * @brief Configures a serial port for 115200 8N1, no flow control
     * @param port Serial port
     * @return Value 0 on success otherwise error code
     */
ssize_t yrm100_serial_configure(serial_port_t port);

    /**
     * @brief Reads up to size bytes into buffer
     * @param port Serial port
     * @param buffer Output buffer
     * @param size Maximum number of bytes to read
     * @return Number of bytes read, 0 on timeout, or -1 on error
     */
ssize_t yrm100_serial_read(serial_port_t port, void *buffer, size_t size);

    /**
     * @brief Writes size bytes from buffer to the serial port
     * @param port Serial port
     * @param buffer Input buffer
     * @param size Number of bytes to write
     * @return Number of bytes written or -1 on error
     */
ssize_t yrm100_serial_write(serial_port_t port, const void *buffer, size_t size);

    /**
     * @brief Closes the serial port
     * @param port Serial port
     */
void yrm100_serial_close(serial_port_t port);

#ifdef __cplusplus
}
#endif

#endif // YRM100_SERIAL_H
