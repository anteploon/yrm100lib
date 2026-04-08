#include "yrm100_types.h"

bool yrm100_is_valid_memory_bank(unsigned char memory_bank)
{
    return memory_bank == YRM100_MEMORY_BANK_RESERVED ||
           memory_bank == YRM100_MEMORY_BANK_EPC ||
           memory_bank == YRM100_MEMORY_BANK_TID ||
           memory_bank == YRM100_MEMORY_BANK_USER;
}
