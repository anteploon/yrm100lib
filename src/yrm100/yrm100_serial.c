#include "yrm100_serial.h"
#include "yrm100_frame.h"
#include <stdio.h>
#include <unistd.h>

#ifdef _WIN32
// ==================== WINDOWS IMPLEMENTATION ====================

serial_port_t serial_open(const char *port_name)
{
    char full_name[64];
    snprintf(full_name, sizeof(full_name), "\\\\.\\%s", port_name);

    HANDLE hSerial = CreateFileA(full_name, GENERIC_READ | GENERIC_WRITE, 0, NULL,
                                 OPEN_EXISTING, 0, NULL);
    if (hSerial == INVALID_HANDLE_VALUE)
    {
        perror("Error opening serial port");
        return INVALID_HANDLE_VALUE;
    }
    return hSerial;
}

ssize_t serial_configure(serial_port_t hSerial)
{
    DCB dcbSerialParams = {0};
    COMMTIMEOUTS timeouts = {0};

    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    if (!GetCommState(hSerial, &dcbSerialParams))
    {
        perror("Error getting state");
        return -1;
    }

    dcbSerialParams.BaudRate = CBR_115200;
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity = NOPARITY;
    dcbSerialParams.fOutxCtsFlow = FALSE;
    dcbSerialParams.fOutxDsrFlow = FALSE;
    dcbSerialParams.fDtrControl = DTR_CONTROL_DISABLE;
    dcbSerialParams.fRtsControl = RTS_CONTROL_DISABLE;
    dcbSerialParams.fOutX = FALSE;
    dcbSerialParams.fInX = FALSE;

    if (!SetCommState(hSerial, &dcbSerialParams))
    {
        perror("Error setting serial params");
        return -1;
    }

    timeouts.ReadIntervalTimeout = 50;
    timeouts.ReadTotalTimeoutConstant = 300;
    timeouts.ReadTotalTimeoutMultiplier = 0;
    timeouts.WriteTotalTimeoutConstant = 0;
    timeouts.WriteTotalTimeoutMultiplier = 0;

    if (!SetCommTimeouts(hSerial, &timeouts))
    {
        perror("Error setting timeouts");
        return -1;
    }

    return 0;
}

ssize_t serial_read(serial_port_t hSerial, void *buffer, size_t size)
{
    DWORD bytesRead = 0;
    if (!ReadFile(hSerial, buffer, (DWORD)size, &bytesRead, NULL))
    {
        perror("Error reading serial port");
        return -1;
    }
    return (int)bytesRead;
}

ssize_t serial_write(serial_port_t hSerial, const void *buffer, size_t size)
{
    DWORD bytesWritten = 0;
    if (!WriteFile(hSerial, buffer, (DWORD)size, &bytesWritten, NULL))
    {
        perror("Error writing to serial port");
        return -1;
    }
    return (int)bytesWritten;
}

void serial_close(serial_port_t hSerial)
{
    CloseHandle(hSerial);
}

#else
// ==================== POSIX (Linux/macOS) IMPLEMENTATION ====================

#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include <errno.h>
#include <stddef.h>

serial_port_t serial_open(const char *port_name)
{
    int fd = open(port_name, O_RDWR | O_NOCTTY | O_SYNC);
    if (fd < 0)
    {
        perror("Error opening serial port");
        return -1;
    }
    return fd;
}

ssize_t serial_configure(serial_port_t port)
{
    struct termios tty;

    if (tcgetattr(port, &tty) != 0)
    {
        perror("Error from tcgetattr");
        return -1;
    }

    cfsetospeed(&tty, B115200);
    cfsetispeed(&tty, B115200);

    tty.c_cflag &= (tcflag_t)~PARENB;
    tty.c_cflag &= (tcflag_t)~CSTOPB;
    tty.c_cflag &= (tcflag_t)~CSIZE;
    tty.c_cflag |= (tcflag_t)CS8;
    tty.c_cflag &= (tcflag_t)~CRTSCTS;
    tty.c_cflag |= CREAD | CLOCAL;

    tty.c_lflag &= (tcflag_t)~(ICANON | ECHO | ECHOE | ISIG);
    tty.c_iflag &= (tcflag_t)~(IXON | IXOFF | IXANY);
    tty.c_iflag &= (tcflag_t)~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL);
    tty.c_oflag &= (tcflag_t)~(OPOST | ONLCR);

    tty.c_cc[VTIME] = 3;
    tty.c_cc[VMIN] = YRM100_FRAME_MINIMUM_NOTICE_SIZE > YRM100_FRAME_MINIMUM_RESPONSE_SIZE ? YRM100_FRAME_MINIMUM_RESPONSE_SIZE : YRM100_FRAME_MINIMUM_NOTICE_SIZE;

    if (tcsetattr(port, TCSANOW, &tty) != 0)
    {
        perror("Error from tcsetattr");
        return -1;
    }

    return 0;
}

ssize_t serial_read(serial_port_t port, void *buffer, size_t size)
{
    ssize_t n = read(port, buffer, size);
    if (n < 0)
        perror("Error reading serial port");
    return n;
}

ssize_t serial_write(serial_port_t port, const void *buffer, size_t size)
{
    tcflush(port, TCIOFLUSH);
    ssize_t n = write(port, buffer, size);
    if (n < 0)
        perror("Error writing serial port");
    return n;
}

void serial_close(serial_port_t port)
{
    close(port);
}
#endif
