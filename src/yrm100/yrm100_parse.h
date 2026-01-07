#ifndef YRM100_PARSE_H
#define YRM100_PARSE_H

#include <stdlib.h>

/**
 * @brief Extracts the ASCII string from RFID UHF response frame
 * @param response 
 * @param response_len 
 * @param string_buf 
 * @param string_buf_len 
 * @return 
 */
int yrm100_parse_ascii_response(unsigned char *response, size_t response_len, char *string_buf, size_t string_buf_len);

/**
 * @brief Extracts the RFID UHF tag information from the notice frame
 * @param response 
 * @param response_len 
 * @param tags 
 * @param maximum_tag_count 
 * @return Value 0 or positive on success, otherwise error code
 */
int yrm100_parse_poll_response(unsigned char *response, size_t response_len, rfid_tag_t *tags, unsigned short maximum_tag_count);

/**
 * @brief Extracts the error code from the error response frame
 * @param buf Buffer
 * @param buf_size Buffer size
 * @return Error code or a negative value on parse failure
 */
int yrm100_parse_get_error_code(unsigned char *buf, size_t buf_size);

#endif
