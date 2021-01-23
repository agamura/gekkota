/******************************************************************************
 * @file    gekkota_crc32.c
 * @date    29-Dec-2006
 * @author  <a href="mailto:giuseppe.greco@agamura.com">Giuseppe Greco</a>
 *
 * Copyright (C) 2006 Agamura, Inc. - http://www.agamura.com
 * All right reserved.
 ******************************************************************************/

#include "gekkota.h"
#include "gekkota_crc32.h"

#define LOOKUP_TABLE_LENGTH     256
#define CRC32_POLYNOM           0x04C11DB7
#define CRC32_INITIAL           0xFFFFFFFF

static uint32_t lookupTable[LOOKUP_TABLE_LENGTH];
static bool_t   isLookupTableInitialized = FALSE;

static void_t
_gekkota_crc32_initialize_lookup_table()
{
    int32_t byte;

    for (byte = 0; byte < LOOKUP_TABLE_LENGTH; byte++)
    {
        register uint32_t crc = byte << 24;
        int32_t offset;

        for (offset = 0; offset < 8; offset++)
            if (crc & 0x80000000)
                crc = (crc << 1) ^ CRC32_POLYNOM;
            else
                crc <<= 1;

        lookupTable[byte] = crc;
    }

    isLookupTableInitialized = TRUE;
}
    
uint32_t
_gekkota_crc32_calculate(const GekkotaBuffer *buffers, size_t bufferCount)
{
    register uint32_t crc = CRC32_INITIAL;

    if (!isLookupTableInitialized)
        _gekkota_crc32_initialize_lookup_table();

    while (bufferCount-- > 0)
    {
        const byte_t *head = (const byte_t *) buffers->data;
        const byte_t *tail = &head[buffers->length];

        while (head < tail)
            crc = ((crc << 8) | *head++) ^ lookupTable[crc >> 24];        

        ++buffers;
    }

    return gekkota_host_to_net_32(~crc);
}
