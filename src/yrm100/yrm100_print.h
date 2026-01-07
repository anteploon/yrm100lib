#ifndef YRM100_PRINT_H
#define YRM100_PRINT_H

#include "yrm100.h"

/**
 * @brief Prints RFID tag
 * @param tag RFID tag
 */
void yrm100_print_tag_info(rfid_tag_t *tag);

/**
 * @brief Prints RFID module info
 * @param device_context
 */
void yrm100_print_module_info(yrm100_context_t *device_context);

#endif
