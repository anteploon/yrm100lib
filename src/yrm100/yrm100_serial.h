#ifndef YRM100_SERIAL_H
#define YRM100_SERIAL_H

#include <sys/types.h>

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
     * Opens a serial port.
     * Example:
     *   - Linux:  "/dev/ttyUSB0"
     *   - Windows: "COM3"
     */
serial_port_t yrm100_serial_open(const char *port_name);

    /**
     * @brief Configures a serial port for 115200 8N1, no flow control
     * @param port Serial port
     * @return On success 0
     */
ssize_t yrm100_serial_configure(serial_port_t port);

    /**
     * Reads up to `size` bytes into `buffer`.
     * Returns number of bytes read, 0 on timeout, or -1 on error.
     */
ssize_t yrm100_serial_read(serial_port_t port, void *buffer, size_t size);

    /**
     * Writes `size` bytes from `buffer` to the serial port.
     * Returns number of bytes written or -1 on error.
     */
ssize_t yrm100_serial_write(serial_port_t port, const void *buffer, size_t size);

    /**
     * Closes the serial port.
     */
void yrm100_serial_close(serial_port_t port);

#ifdef __cplusplus
}
#endif

#endif // YRM100_SERIAL_H
