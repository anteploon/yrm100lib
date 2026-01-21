#include <stdio.h>
#include "yrm100_print.h"
#include "yrm100_string.h"
#include "yrm100_command.h"

void yrm100_print_tag_info(rfid_tag_t *tag)
{
    if (tag == NULL)
    {
        return;
    }
    char buf[YRM100_TAG_STRING_LENGTH];

    yrm100_get_tag_string(tag, buf);
    printf("%s\n", buf);
}

void yrm100_print_module_info(yrm100_context_t *device_context)
{
    char buf[YRM100_MODULE_INFO_STRING_LENGTH];

    yrm100_command_get_module_info_string(device_context, buf);
    printf("%s\n", buf);
}

void yrm100_print_query_parameters(yrm100_query_parameters_t *query_parameters)
{
    if (query_parameters == NULL)
    {
        return;
    }
    printf("Query Parameters:\n");
    printf(" DR: %u\n", query_parameters->dr);
    printf(" M: %u\n", query_parameters->m);
    printf(" TReXt: %u\n", query_parameters->trext);
    printf(" Sel: %u\n", query_parameters->sel);
    printf(" Session: %u\n", query_parameters->session);
    printf(" Target: %u\n", query_parameters->target);
    printf(" Q: %u\n", query_parameters->q);
}
