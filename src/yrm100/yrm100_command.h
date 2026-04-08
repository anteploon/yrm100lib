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
 * @return Value 0 on success, otherwise error code
 */
int yrm100_command_get_module_manufacturer(yrm100_context_t *device_context, char *string_buf, size_t string_buf_size);

/**
 * @brief Gets a string that represents the RFID module hardware version
 * @param device_context Handle to the RFID UHF device
 * @param string_buf Pointer to string buffer
 * @param string_buf_size String buffer size
 * @return Value 0 on success, otherwise error code
 */
int yrm100_command_get_module_hardware_version(yrm100_context_t *device_context, char *string_buf, size_t string_buf_size);

/**
 * @brief Gets a string that represents the RFID module software version
 * @param device_context Handle to the RFID UHF device
 * @param string_buf Pointer to string buffer
 * @param string_buf_size String buffer size
 * @return Value 0 on success, otherwise error code
 */
int yrm100_command_get_module_software_version(yrm100_context_t *device_context, char *string_buf, size_t string_buf_size);

/**
 * @brief Executes "single poll" command on the RFID module
 * @param device_context Handle to the RFID UHF device
 * @param tags Pointer where to store tags
 * @param maximum_tag_count Maximum number of tags to read
 * @return Value 0 on success, otherwise error code
 */
int yrm100_command_single_poll(yrm100_context_t *device_context, yrm100_rfid_tag_t *tags, unsigned short maximum_tag_count);

/**
 * @brief Executes "Set idle sleep time" command on the RFID module
 * @param device_context Handle to the RFID UHF device
 * @param minutes After how many minutes to sleep (0 means never)
 * @return Value 0 on success, otherwise error code
 */
int yrm100_command_set_idle_sleep_time(yrm100_context_t *device_context, uint8_t minutes);

/**
 * @brief Gets select parameters from the RFID module
 * @param device_context Handle to the RFID UHF device
 * @param select_parameters Pointer to where to store the select parameters
 * @return Value 0 on success, otherwise error code
 */
int yrm100_command_get_select_parameters(yrm100_context_t *device_context, yrm100_select_parameters_t *select_parameters);

/**
 * @brief Sets select parameters on the RFID module
 * @param device_context Handle to the RFID UHF device
 * @param select_parameters Pointer to select parameters
 * @return Value 0 on success, otherwise error code
 */
int yrm100_command_set_select_parameters(yrm100_context_t *device_context, yrm100_select_parameters_t *select_parameters);

/**
 * @brief Sets select mode on the RFID module
 * @param device_context Handle to the RFID UHF device
 * @param select_mode One of the values from @ref yrm100_select_mode
 * @return Value 0 on success, otherwise error code
 */
int yrm100_command_set_select_mode(yrm100_context_t *device_context, uint8_t select_mode);

/**
 * @brief Gets query parameters from the RFID module
 * @param device_context Handle to the RFID UHF device
 * @param query_parameters Pointer to where to store the query parameters
 * @return Value 0 on success, otherwise error code
 */
int yrm100_command_get_query_parameters(yrm100_context_t *device_context, yrm100_query_parameters_t *query_parameters);

/**
 * @brief Disables idle sleep on RFID module
 * @param device_context Handle to the RFID UHF device
 * @return Value 0 on success, otherwise error code
 */
int yrm100_command_disable_idle_sleep(yrm100_context_t *device_context);

/**
 * @brief Reads data from a specific RFID tag memory bank
 * @param device_context Handle to the RFID UHF device
 * @param tag Pointer where to store the returned tag data
 * @param memory_bank One of the YRM100_PARAM_MEMBANK_* constants
 * @param segment_address Starting word address in the memory bank
 * @param data_length Number of words to read
 * @param password Access password to use when reading the memory area
 * @return Value 0 on success, otherwise error code
 */
int yrm100_command_read_tag_memory_area(yrm100_context_t *device_context, yrm100_rfid_tag_t *tag, uint8_t memory_bank, unsigned short segment_address, unsigned short data_length, uint32_t password);

/**
 * @brief Sets RFID module operating region setting
 * @param device_context Handle to the RFID UHF device
 * @param region See YRM100_PARAM_REGION_* constants
 * @return Value 0 on success, otherwise error code
 */
int yrm100_command_set_operating_region(yrm100_context_t *device_context, uint8_t region);

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
 * @return Value 0 on success, otherwise error code
 */
int yrm100_command_set_tx_power(yrm100_context_t *device_context, unsigned short power);

/**
 * @brief Gets RFID module TX power setting
 * @param device_context Handle to the RFID UHF device
 * @return Positive value indicating the power setting (where 2000 means 20dBm) or negative error code
 */
int yrm100_command_get_tx_power(yrm100_context_t *device_context);

/**
 * @brief Turns the continuous wave mode on or off
 * @param device_context Handle to the RFID UHF device
 * @param on_or_off Use either YRM100_PARAM_CONTINOUS_WAVE_OFF (0x00) or YRM100_PARAM_CONTINOUS_WAVE_ON (0xFF)
 * @return Value 0 on success, otherwise error code
 */
int yrm100_command_set_continous_wave(yrm100_context_t *device_context, uint8_t on_or_off);

/**
 * @brief Turns continuous wave mode on
 * @param device_context Handle to the RFID UHF device
 * @return Value 0 on success, otherwise error code
 */
int yrm100_command_enable_continous_wave(yrm100_context_t *device_context);

/**
 * @brief Turns continuous wave mode off
 * @param device_context Handle to the RFID UHF device
 * @return Value 0 on success, otherwise error code
 */
int yrm100_command_disable_continous_wave(yrm100_context_t *device_context);

/**
 * @brief Puts the RFID module to sleep
 * @param device_context Handle to the RFID UHF device
 * @return Value 0 on success, otherwise error code
 */
int yrm100_command_sleep(yrm100_context_t *device_context);

/**
 * @brief Sends the lock command to a specific RFID tag; before this command, a set select parameter command should be sent first
 * @param device_context Handle to the RFID UHF device
 * @param password Password to use when locking the tag
 * @return Value 0 on success, otherwise error code
 */
int yrm100_command_lock(yrm100_context_t *device_context, uint32_t password);

/**
 * @brief Sends the kill command to a specific RFID tag; before this command, a set select parameter command should be sent first
 * @param device_context Handle to the RFID UHF device
 * @param password Password to use when killing the tag
 * @return Value 0 on success, otherwise error code
 */
int yrm100_command_kill(yrm100_context_t *device_context, uint32_t password);

/**
 * @brief Formats the current TX power setting as a string
 * @param device_context Handle to the RFID UHF device
 * @param string_buf Pointer to string buffer
 * @return Pointer to string_buf
 */
char *yrm100_command_get_tx_power_string(yrm100_context_t *device_context, char string_buf[YRM100_PARAM_TX_POWER_STRING_LENGTH]);

/**
 * @brief Gets manufacturer, hardware version, and software version as one human-readable string
 * @param device_context Handle to the RFID UHF device
 * @param string_buf Pointer to string buffer
 * @return Pointer to string_buf
 */
char *yrm100_command_get_module_info_string(yrm100_context_t *device_context, char string_buf[YRM100_MODULE_INFO_STRING_LENGTH]);

#endif
