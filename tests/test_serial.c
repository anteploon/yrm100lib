#include <string.h>
#include "yrm100/yrm100_serial.h"
#include "yrm100/yrm100_error.h"

#define TEST_SERIAL_MAX_READ_DATA 2048
#define TEST_SERIAL_MAX_CHUNKS 16

static unsigned char test_serial_read_data[TEST_SERIAL_MAX_READ_DATA];
static size_t test_serial_read_len;
static size_t test_serial_read_offset;
static size_t test_serial_chunks[TEST_SERIAL_MAX_CHUNKS];
static size_t test_serial_chunk_count;
static size_t test_serial_chunk_index;

void test_serial_set_read_data(const unsigned char *data, size_t len, const size_t *chunks, size_t chunk_count)
{
    if (len > TEST_SERIAL_MAX_READ_DATA || chunk_count > TEST_SERIAL_MAX_CHUNKS)
    {
        test_serial_read_len = 0;
        test_serial_chunk_count = 0;
        test_serial_read_offset = 0;
        test_serial_chunk_index = 0;
        return;
    }

    memcpy(test_serial_read_data, data, len);
    memcpy(test_serial_chunks, chunks, sizeof(test_serial_chunks[0]) * chunk_count);
    test_serial_read_len = len;
    test_serial_read_offset = 0;
    test_serial_chunk_count = chunk_count;
    test_serial_chunk_index = 0;
}

serial_port_t yrm100_serial_open(const char *port_name)
{
    (void)port_name;
    return (serial_port_t)1;
}

ssize_t yrm100_serial_configure(serial_port_t port)
{
    (void)port;
    return 0;
}

ssize_t yrm100_serial_read(serial_port_t port, void *buffer, size_t size)
{
    (void)port;
    if (test_serial_read_offset >= test_serial_read_len)
    {
        return 0;
    }
    if (test_serial_chunk_index >= test_serial_chunk_count)
    {
        return 0;
    }

    size_t remaining = test_serial_read_len - test_serial_read_offset;
    size_t to_read = test_serial_chunks[test_serial_chunk_index];
    if (to_read > remaining)
    {
        to_read = remaining;
    }
    if (to_read > size)
    {
        to_read = size;
    }

    memcpy(buffer, &test_serial_read_data[test_serial_read_offset], to_read);
    test_serial_read_offset += to_read;
    test_serial_chunk_index++;
    return (ssize_t)to_read;
}

ssize_t yrm100_serial_write(serial_port_t port, const void *buffer, size_t size)
{
    (void)port;
    (void)buffer;
    return (ssize_t)size;
}

void yrm100_serial_close(serial_port_t port)
{
    (void)port;
}
