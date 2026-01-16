#ifndef YRM100_COMMAND_H
#define YRM100_COMMAND_H

#include "yrm100.h"
#include "yrm100_serial.h"
#include "yrm100_string.h"

// #define YRM100_COMM_DEBUG
#define YRM100_COMMAND_RESPONSE_DELAY_USEC 75000

/**
 * @brief Gets the RFID module manufacturer string
 * @param device_context Handle to the RFID UHF device
 * @param string_buf Pointer to string buffer
 * @param string_buf_size String buffer size
 * @return Value 0 on success otherwise error code
 */
int yrm100_command_get_module_manufacturer(yrm100_context_t *device_context, char *string_buf, size_t string_buf_size);

/**
 * @brief Gets a string that represents the RFID module hardware version
 * @param device_context Handle to the RFID UHF device
 * @param string_buf Pointer to string buffer
 * @param string_buf_size String buffer size
 * @return Value 0 on success otherwise error code
 */
int yrm100_command_get_module_hardware_version(yrm100_context_t *device_context, char *string_buf, size_t string_buf_size);

/**
 * @brief Gets a string that represents the RFID module software version
 * @param device_context Handle to the RFID UHF device
 * @param string_buf Pointer to string buffer
 * @param string_buf_size String buffer size
 * @return Value 0 on success otherwise error code
 */
int yrm100_command_get_module_software_version(yrm100_context_t *device_context, char *string_buf, size_t string_buf_size);

/**
 * @brief Executes "single poll" command on the RFID module
 * @param device_context Handle to the RFID UHF device
 * @param tags Pointer where to store tags
 * @param maximum_tag_count Maximum number of tags to read
 * @return Value 0 on success otherwise error code
 */
int yrm100_command_single_poll(yrm100_context_t *device_context, rfid_tag_t *tags, unsigned short maximum_tag_count);

/**
 * @brief Starts the multi poll process
 * @param device_context Handle to the RFID UHF device
 * @param tags Pointer to memory are that will hold the RFID tag data
 * @param maximum_tag_count Maximum number of tags to read
 * @return Value 0 on success otherwise error code
 */
// int yrm100_command_multi_poll_start(yrm100_context_t *device_context, rfid_tag_t *tags, unsigned short maximum_tag_count);

/**
 * @brief Should be called when the multi poll process is active, for the data in buffers to get processed
 * @param device_context Handle to the RFID UHF device
 * @return Value 0 on success otherwise error code
 */
// int yrm100_command_multi_poll_continue(yrm100_context_t *device_context);

/**
 * @brief Stops the multi poll process
 * @param device_context Handle to the RFID UHF device
 * @return Value 0 on success otherwise error code
 */
// int yrm100_command_multi_poll_stop(yrm100_context_t *device_context);

/**
 * @brief Executes "Set idle sleep time" command on the RFID module
 * @param device_context Handle to the RFID UHF device
 * @param minutes After how many minutes to sleep (0 means never)
 * @return Value 0 on success otherwise error code
 */
int yrm100_command_set_idle_sleep_time(yrm100_context_t *device_context, unsigned char minutes);

/**
 * @brief Sets select parameters on the RFID module
 * @param device_context Handle to the RFID UHF device
 * @param select_parameters Pointer to select parameters
 * @return Value 0 on success otherwise error code
 */
int yrm100_command_set_select_parameters(yrm100_context_t *device_context, rfid_select_parameters_t *select_parameters);

/**
 * @brief Sets select mode on the RFID module
 * @param device_context Handle to the RFID UHF device
 * @param select_mode One of the values from @ref yrm100_select_mode
 * @return Value 0 on success otherwise error code
 */
int yrm100_command_set_select_mode(yrm100_context_t *device_context, unsigned char select_mode);

/**
 * @brief Disables idle sleep on RFID module
 * @param device_context Handle to the RFID UHF device
 * @return Value 0 on success otherwise error code
 */
int yrm100_command_disable_idle_sleep(yrm100_context_t *device_context);

/**
 * @brief Sets RFID module operating region setting
 * @param device_context Handle to the RFID UHF device
 * @param region See YRM100_PARAM_REGION_* constants
 * @return Value 0 on success otherwise error code
 */
int yrm100_command_set_operating_region(yrm100_context_t *device_context, unsigned char region);

/**
 * @brief Gets RFID module operating region setting
 * @param device_context Handle to the RFID UHF device
 * @return Positive value indicating the operating region or negative value indicating an error
 */
int yrm100_command_get_operating_region(yrm100_context_t *device_context);

/**
 * @brief Sets RFID module TX power setting
 * @param device_context Handle to the RFID UHF device
 * @param power Value where for instance 2600 means 26dBm
 * @return Value 0 on success otherwise error code
 */
int yrm100_command_set_tx_power(yrm100_context_t *device_context, unsigned short power);

/**
 * @brief Gets RFID module TX power setting
 * @param device_context Handle to the RFID UHF device
 * @return Positive value indicating the power setting (where 2000 means for example 20dBm) and negative value indicating an error
 */
int yrm100_command_get_tx_power(yrm100_context_t *device_context);

/**
 * @brief Turns the continous wave mode on or of
 * @param device_context Handle to the RFID UHF device
 * @param on_or_off Use either YRM100_PARAM_CONTINOUS_WAVE_OFF (0x00) or YRM100_MODULE_CONTINOUS_WAVE_ON (0xFF)
 * @return Value 0 on success otherwise error code
 */
int yrm100_command_set_continous_wave(yrm100_context_t *device_context, unsigned char on_or_off);

/**
 * @brief Turns continous wave mode on
 * @param device_context Handle to the RFID UHF device
 * @return Value 0 on success otherwise error code
 */
int yrm100_command_enable_continous_wave(yrm100_context_t *device_context);

/**
 * @brief Turns continous wave mode off
 * @param device_context Handle to the RFID UHF device
 * @return Value 0 on success otherwise error code
 */
int yrm100_command_disable_continous_wave(yrm100_context_t *device_context);

/**
 * @brief
 * @param device_context Handle to the RFID UHF device
 * @param string_buf Pointer to string buffer
 * @return Value 0 on success otherwise error code
 */
char *yrm100_command_get_tx_power_string(yrm100_context_t *device_context, char string_buf[YRM100_PARAM_TX_POWER_STRING_LENGTH]);

/**
 * @brief Wraps three separate RFID UHF function calls into one, that formats the resulting string into human readable one
 * @param device_context Handle to the RFID UHF device
 * @param string_buf Pointer to string buffer
 * @return Value 0 on success otherwise error code
 */
char *yrm100_command_get_module_info_string(yrm100_context_t *device_context, char string_buf[YRM100_MODULE_INFO_STRING_LENGTH]);

#endif
