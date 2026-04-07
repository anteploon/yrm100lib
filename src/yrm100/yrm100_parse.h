#ifndef YRM100_PARSE_H
#define YRM100_PARSE_H

#include <stdlib.h>
#include "yrm100_types.h"

/**
 * @brief Extracts the ASCII string from RFID UHF response frame
 * @param response Response frame buffer
 * @param response_len Response buffer size
 * @param string_buf Output string buffer
 * @param string_buf_len Output string buffer size
 * @return Value 0 on success, otherwise error code
 */
int yrm100_parse_ascii_response(unsigned char *response, size_t response_len, char *string_buf, size_t string_buf_len);

/**
 * @brief Extracts RFID UHF tag information from one or more poll notice frames
 * @param response Notice frame buffer
 * @param response_len Response buffer size
 * @param tags Pointer to tag array
 * @param maximum_tag_count Maximum number of tags to parse
 * @return Number of parsed tags on success, otherwise negative error code
 */
int yrm100_parse_poll_response(unsigned char *response, size_t response_len, yrm100_rfid_tag_t *tags, unsigned short maximum_tag_count);

/**
 * @brief Extracts the module error code from an error response frame
 * @param buf Buffer
 * @param buf_size Buffer size
 * @return Module error code or a negative value on parse failure
 */
int yrm100_parse_get_error_code(unsigned char *buf, size_t buf_size);

#endif
