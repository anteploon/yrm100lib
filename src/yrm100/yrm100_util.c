#include <stdio.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
#include <stdlib.h>
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

void yrm100_free_tag_data(yrm100_rfid_tag_t *tags, unsigned short tag_count)
{
    yrm100_rfid_tag_t *t;
    if (tags==NULL || tag_count==0) {
        return;
    }
    for (unsigned short i = 0; i < tag_count; i++)
    {
        t = &tags[i];
        if (t->data != NULL)
        {
            free(t->data);
            t->data = NULL;
            t->data_length = 0;
        }
    }
}

void yrm100_reset_tag_buf(yrm100_rfid_tag_t *tags, unsigned short tag_count)
{
    yrm100_rfid_tag_t *t;
    if (tags==NULL || tag_count==0) {
        return;
    }
    for (unsigned short i = 0; i < tag_count; i++)
    {
        t = &tags[i];
        for (uint8_t j = 0; j < YRM100_TAG_EPC_BYTE_COUNT; j++)
        {
            t->epc[j] = 0x00;
        }
        t->rssi = 0;
        t->pc = 0;
        t->crc = 0;
        if (t->data != NULL)
        {
            free(t->data);
        }
        t->data = NULL;
        t->data_length = 0;
    }
}

bool yrm100_is_empty_tag(yrm100_rfid_tag_t *tag)
{
    if (tag == NULL)
    {
        return true;
    }
    if (tag->rssi == 0 && tag->pc == 0 && tag->crc == 0)
    {
        return true;
    }
    return false;
}

void yrm100_sleep_usec(unsigned int usec)
{
    if (usec == 0)
    {
        return;
    }
#ifdef _WIN32
    DWORD msec = (DWORD)((usec + 999U) / 1000U);
    Sleep(msec);
#else
    usleep(usec);
#endif
}
