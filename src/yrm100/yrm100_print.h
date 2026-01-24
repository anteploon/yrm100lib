#ifndef YRM100_PRINT_H
#define YRM100_PRINT_H

#include "yrm100.h"

/**
 * @brief Prints RFID tag
 * @param tag RFID tag
 */
void yrm100_print_tag_info(yrm100_rfid_tag_t *tag);

/**
 * @brief Prints RFID module info
 * @param device_context Handle to the RFID UHF device
 */
void yrm100_print_module_info(yrm100_context_t *device_context);

/**
 * @brief Prints query parameters
 * @param query_parameters Query parameters
 */
void yrm100_print_query_parameters(yrm100_query_parameters_t *query_parameters);

#endif
