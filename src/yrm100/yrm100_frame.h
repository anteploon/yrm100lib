#ifndef YRM100_FRAME_H
#define YRM100_FRAME_H

#include <sys/types.h>
#include <stdbool.h>

/** @defgroup frame_byte_position Byte positions in the RFID frame
 *  @{
 */

#define YRM100_FRAME_BYTE_POSITION_HEADER 0
#define YRM100_FRAME_BYTE_POSITION_TYPE 1
#define YRM100_FRAME_BYTE_POSITION_COMMAND 2

/** @} */

#define YRM100_FRAME_ERROR_RESPONSE_CODE_POSITION 5

#define YRM100_FRAME_POLL_NOTICE_SIZE 24

#define YRM100_FRAME_MINIMUM_COMMAND_SIZE 7
#define YRM100_FRAME_MINIMUM_RESPONSE_SIZE 7
#define YRM100_FRAME_MINIMUM_NOTICE_SIZE 24

#define YRM100_FRAME_HEADER_BYTE 0xBB
#define YRM100_FRAME_TYPE_BYTE_COMMAND 0x00
#define YRM100_FRAME_TYPE_BYTE_RESPONSE 0x01
#define YRM100_FRAME_TYPE_BYTE_NOTICE 0x02
#define YRM100_FRAME_END_BYTE 0x7E

/**
 * @brief Checks if the RFID UHF frame looks like a command frame (does not validate the frame)
 * @param buf Buffer
 * @param buf_size Buffer size
 * @return True if the frame looks like a command frame
 */
bool yrm100_frame_is_command(unsigned char *buf, size_t buf_size);

/**
 * @brief Checks if the RFID UHF frame looks like a response frame (does not validate the frame)
 * @param buf Buffer
 * @param buf_size Buffer size
 * @return True if the frame looks like a response frame
 */
bool yrm100_frame_is_response(unsigned char *buf, size_t buf_size);

/**
 * @brief Checks if the RFID UHF frame looks like a notice frame (does not validate the frame)
 * @param buf Buffer
 * @param buf_size Buffer size
 * @return True if the frame looks like a notice frame
 */
bool yrm100_frame_is_notice(unsigned char *buf, size_t buf_size);

/**
 * @brief Checks if the RFID UHF frame looks like a valid command frame (also verifies the frame checksum)
 * @param buf Buffer
 * @param buf_size Buffer size
 * @return True if the frame is a valid command frame
 */
bool yrm100_frame_is_valid_command(unsigned char *buf, size_t buf_size);

/**
 * @brief 
 * @param buf Buffer
 * @param buf_size Buffer size
 * @return True if the frame is a valid response frame
 */
bool yrm100_frame_is_valid_response(unsigned char *buf, size_t buf_size);

/**
 * @brief 
 * @param buf Buffer
 * @param buf_size Buffer size
 * @return True if the frame is a error response
 */
bool yrm100_frame_is_error_response(unsigned char *buf, size_t buf_size);

/**
 * @brief 
 * @param buf Buffer
 * @param buf_size Buffer size
 * @return True if the frame is a non-error response frame
 */
bool yrm100_frame_is_ok_response(unsigned char *buf, size_t buf_size);

/**
 * @brief Calculates the checksum of the frame
 * @param buf Buffer
 * @param buf_size Buffer size
 * @return Checksum or a negative value on error (buf_size must be at least 2)
 */
int yrm100_frame_calculate_checksum(unsigned char *buf, size_t buf_size);

#endif
