#ifndef YRM100_PARAM_H
#define YRM100_PARAM_H

#include <stdbool.h>
#include <stdint.h>

/** @defgroup yrm100_region RFID operating region values
 *  @{
 */

 /** China 900MHz */
#define YRM100_PARAM_REGION_CHINA_900 1

/** China 800MHz */
#define YRM100_PARAM_REGION_CHINA_800 4

/** USA */
#define YRM100_PARAM_REGION_USA 2

/** Europe */
#define YRM100_PARAM_REGION_EUROPE 3

/** Korea */
#define YRM100_PARAM_REGION_KOREA 6

/** @} */

/** @defgroup yrm100_select_mode RFID select modes
 *  @{
 */

/** Send select before all operations */
#define YRM100_PARAM_SELECT_MODE_DO_SEND_BEFORE_ALL_OPERATIONS 0x00

/** Do not send select before any operations */
#define YRM100_PARAM_SELECT_MODE_DONT_SEND_BEFORE_ANY_OPERATIONS 0x01

/** Send select before tag operations only */
#define YRM100_PARAM_SELECT_MODE_DO_SEND_BEFORE_TAG_OPERATIONS 0x02

/** @} */

/** @defgroup yrm100_select_membank RFID select memory bank values
 *  @{
 */

/** Reserved for future use */
#define YRM100_PARAM_MEMBANK_RFU 0

/** Target EPC bank */
#define YRM100_PARAM_MEMBANK_EPC 1

/** Target Tag ID bank */
#define YRM100_PARAM_MEMBANK_TID 2

/** Target user bank */
#define YRM100_PARAM_MEMBANK_USER 3

/** @} */


/** @defgroup yrm100_tx_power Defines the limits of TX power value
 *  @{
 */

/** Minimum TX power value that can be set (1000 = 10dBm) */
#define YRM100_PARAM_TX_POWER_MINIMUM 1000

/** Maximum TX power value that can be set (2600 = 26dBm) */
#define YRM100_PARAM_TX_POWER_MAXIMUM 2600

/** @} */

/** @defgroup yrm100_continous_wave_toggle Values to use to turn continuous wave on and off
 *  @{
 */

/** Continuous wave off */
#define YRM100_PARAM_CONTINOUS_WAVE_OFF 0x00

/** Continuous wave on */
#define YRM100_PARAM_CONTINOUS_WAVE_ON 0xFF

/** @} */

/**
 * @brief Checks if a value is a valid RFID tag memory bank
 * @param memory_bank Memory bank value
 * @return True if memory_bank is one of the YRM100_PARAM_MEMBANK_* constants
 */
bool yrm100_param_is_valid_memory_bank(uint8_t memory_bank);

/**
 * @brief Checks if a value is a valid RFID operating region
 * @param region Region value
 * @return True if region is one of the YRM100_PARAM_REGION_* constants
 */
bool yrm100_param_is_valid_region(uint8_t region);

/**
 * @brief Checks if a value is a valid RFID select mode
 * @param select_mode Select mode value
 * @return True if select_mode is one of the YRM100_PARAM_SELECT_MODE_* constants
 */
bool yrm100_param_is_valid_select_mode(uint8_t select_mode);

/**
 * @brief Checks if a value is a valid RFID query Q value
 * @param q_value Query Q value
 * @return True if q_value can fit in the 4-bit Q field
 */
bool yrm100_param_is_valid_q_value(uint8_t q_value);

#endif
