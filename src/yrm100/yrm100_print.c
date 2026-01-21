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
    printf("------------------------------------------------\n");
    printf("                                Query Parameters\n");
    printf("------------------------------------------------\n");
    printf("             Divide Ratio (DR): %s\n", yrm100_convert_to_divide_ratio_string(query_parameters->dr));
    printf("      Backscatter Encoding (M): %s\n", yrm100_convert_to_backscatter_encoding_string(query_parameters->m));
    printf("Tag Response Extension (TReXt): %s\n", yrm100_convert_to_trext_string(query_parameters->trext));
    printf("           Tag Selection (Sel): %s\n", yrm100_convert_to_sel_string(query_parameters->sel));
    printf("                       Session: %s\n", yrm100_convert_to_session_string(query_parameters->session));
    printf("                        Target: %s\n", yrm100_convert_to_target_string(query_parameters->target));
    printf("                Slot Count (Q): %s\n", yrm100_convert_to_q_string(query_parameters->q));
    printf("------------------------------------------------\n");
}
