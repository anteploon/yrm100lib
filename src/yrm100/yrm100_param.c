#include "yrm100_param.h"

bool yrm100_param_is_valid_region(uint8_t region)
{
    return region == YRM100_PARAM_REGION_CHINA_900 ||
           region == YRM100_PARAM_REGION_CHINA_800 ||
           region == YRM100_PARAM_REGION_USA ||
           region == YRM100_PARAM_REGION_EUROPE ||
           region == YRM100_PARAM_REGION_KOREA;
}

bool yrm100_param_is_valid_memory_bank(uint8_t memory_bank)
{
    return memory_bank == YRM100_PARAM_MEMBANK_RFU ||
           memory_bank == YRM100_PARAM_MEMBANK_EPC ||
           memory_bank == YRM100_PARAM_MEMBANK_TID ||
           memory_bank == YRM100_PARAM_MEMBANK_USER;
}

bool yrm100_param_is_valid_select_mode(uint8_t select_mode)
{
    return select_mode == YRM100_PARAM_SELECT_MODE_DO_SEND_BEFORE_ALL_OPERATIONS ||
           select_mode == YRM100_PARAM_SELECT_MODE_DONT_SEND_BEFORE_ANY_OPERATIONS ||
           select_mode == YRM100_PARAM_SELECT_MODE_DO_SEND_BEFORE_TAG_OPERATIONS;
}

bool yrm100_param_is_valid_q_value(uint8_t q_value)
{
    return q_value <= 15;
}
