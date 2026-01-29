#ifndef YRM100_UTIL_H
#define YRM100_UTIL_H

#include <string.h>
#include "yrm100.h"
#include "yrm100_serial.h"

/**
 * @brief Sets buf_size number of bytes in buf to zero
 * @param buf String buffer
 * @param buf_size Buffer size
 */
void yrm100_zero_buf(void *buf, size_t buf_size);

/**
 * @brief Frees the memory allocated for tag data
 * @param tags Pointer to array of tags
 * @param tag_count Number of tags in the array
 */
void yrm100_free_tag_data(yrm100_rfid_tag_t *tags, unsigned short tag_count);

/**
 * @brief Zeros out the RFID tag structure in memory
 * @param tags Pointer to tags
 * @param tag_count Number of how many tags to zero
 */
void yrm100_clear_tag_buf(yrm100_rfid_tag_t *tags, unsigned short tag_count);

/**
 * @brief Checks if specified RFID tag structure is zeroed out
 * @param tag Pointer to tag
 * @return True if the tag is NULL or zeroed out
 */
bool yrm100_is_empty_tag(yrm100_rfid_tag_t *tag);

/**
 * @brief Sleeps for the specified number of microseconds
 * @param usec Microseconds to sleep
 */
void yrm100_sleep_usec(unsigned int usec);

#endif
