#ifndef YRM100_PARAM_H
#define YRM100_PARAM_H

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

/** @defgroup yrm100_select_membank RFID select membank - see https://www.atlasrfidstore.com/rfid-insider/types-of-memory-in-gen-2-uhf-rfid-tags/
 *  @{
 */

#define YRM100_PARAM_MEMBANK_RFU 0b00

/** Target EPC bank */
#define YRM100_PARAM_MEMBANK_EPC 0b01

/** Target Tag ID bank */
#define YRM100_PARAM_MEMBANK_TID 0b10

/** Target user bank */
#define YRM100_PARAM_MEMBANK_USER 0b11

/** @} */


/** @defgroup yrm100_tx_power Defines the limits of TX power value
 *  @{
 */

/** Minimum TX power value that can be set (1000 = 10dBm) */
#define YRM100_PARAM_TX_POWER_MINIMUM 1000

/** Maximum TX power value that can be set (2600 = 26dBm) */
#define YRM100_PARAM_TX_POWER_MAXIMUM 2600

/** @} */

/** @defgroup yrm100_continous_wave_toggle Values to use to turn continous wave on and off
 *  @{
 */

/** Continous wave off */
#define YRM100_PARAM_CONTINOUS_WAVE_OFF 0x00

/** Continous wave on */
#define YRM100_PARAM_CONTINOUS_WAVE_ON 0xFF

/** @} */

#endif
