#ifndef TEST_SERIAL_H
#define TEST_SERIAL_H

#include <stddef.h>
#include "yrm100/yrm100_serial.h"

void test_serial_reset(void);
void test_serial_set_open_result(serial_port_t port);
void test_serial_set_configure_result(ssize_t result);
void test_serial_set_read_data(const unsigned char *data, size_t len, const size_t *chunks, size_t chunk_count);
size_t test_serial_get_last_write(unsigned char *buffer, size_t size);

#endif
