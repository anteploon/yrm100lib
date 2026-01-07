#include <stdio.h>
#include "yrm100_command.h"
#include "yrm100_util.h"
#include "yrm100_string.h"

void yrm100_zero_buf(void *buf, size_t buf_size)
{
    if (buf != NULL && buf_size > 0)
    {
        memset(buf, 0, buf_size);
    }
}

void yrm100_clear_tag_data(rfid_tag_t *tags, unsigned short tag_count)
{
    rfid_tag_t *t;
    if (tags==NULL || tag_count==0) {
        return;
    }
    for (unsigned short i = 0; i < tag_count; i++)
    {
        t = &tags[i];
        for (unsigned char j = 0; j < YRM100_TAG_EPC_BYTE_COUNT; j++)
        {
            t->epc[j] = 0x00;
        }
        t->rssi = 0;
        t->pc = 0;
        t->crc = 0;
    }
}

bool yrm100_is_empty_tag(rfid_tag_t *tag)
{
    if (tag->rssi == 0 && tag->pc == 0 && tag->crc == 0)
    {
        return true;
    }
    return false;
}
