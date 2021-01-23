/******************************************************************************
 * @file    gekkota_memory.h
 * @date    18-Mar-2009
 * @author  <a href="mailto:giuseppe.greco@agamura.com">Giuseppe Greco</a>
 *
 * Copyright (C) 2007 Agamura, Inc. - http://www.agamura.com
 * All right reserved.
 ******************************************************************************/

#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "gekkota_errors.h"
#include "gekkota_list.h"
#include "gekkota_memory.h"

/* http://www.codeproject.com/KB/cpp/MemoryPoolIntroduction.aspx */

typedef struct _GekkotaFastHeap
{
    void_t          *memory;
    GekkotaList     allocatedBlocks;
    GekkotaList     freeBlocks;
    size_t          blockSize;
    uint32_t        blockCount;
}GekkotaFastHeap;

static GekkotaFastHeap fastHeap =
{
    NULL,
    { NULL, NULL },
    { NULL, NULL },
    0,
    0
};

int32_t
gekkota_memory_initialize(size_t blockSize, uint32_t blockCount)
{
    uint32_t i;

    if (gekkota_memory_is_initialized())
    {
        errno = GEKKOTA_ERROR_MEMORY_ALREADY_INITIALIZED;
        return -1;
    }

    fastHeap.blockSize = blockSize > 0
        ? blockSize
        : GEKKOTA_MEMORY_DEFAULT_BLOCK_SIZE;

    fastHeap.blockCount = blockCount > 0
        ? blockCount
        : GEKKOTA_MEMORY_DEFAULT_BLOCK_COUNT;

    if ((fastHeap.memory = malloc(
        fastHeap.blockCount * (fastHeap.blockSize + sizeof(GekkotaListNode)))) == NULL)
    {
        errno = GEKKOTA_ERROR_OUT_OF_MEMORY;
        return -1;
    }

    gekkota_list_clear(&fastHeap.allocatedBlocks);
    gekkota_list_clear(&fastHeap.freeBlocks);

    for (i = 0; i < fastHeap.blockCount; i++)
    {
        GekkotaListIterator iterator = (GekkotaListIterator)
            ((byte_t *) fastHeap.memory + i * (fastHeap.blockSize + sizeof(GekkotaListNode)));

        gekkota_list_add(&fastHeap.freeBlocks, iterator);
    }

    return 0;
}

int32_t
gekkota_memory_uninitialize(void_t)
{
    if (!gekkota_memory_is_initialized())
    {
        errno = GEKKOTA_ERROR_MEMORY_NOT_INITIALIZED;
        return -1;
    }

    free(fastHeap.memory);
    memset(&fastHeap, 0x00, sizeof(GekkotaFastHeap));
    return 0;
}

bool_t
gekkota_memory_is_initialized()
{
    return fastHeap.memory != NULL;
}

size_t
gekkota_memory_get_block_size(void_t)
{
    if (!gekkota_memory_is_initialized())
    {
        errno = GEKKOTA_ERROR_MEMORY_NOT_INITIALIZED;
        return 0;
    }

    return fastHeap.blockSize;
}

uint32_t
gekkota_memory_get_block_count(void_t)
{
    if (!gekkota_memory_is_initialized())
    {
        errno = GEKKOTA_ERROR_MEMORY_NOT_INITIALIZED;
        return 0;
    }

    return fastHeap.blockCount;
}

void_t *
gekkota_memory_alloc(size_t size, bool_t initialize)
{
    void_t *memory;

    if (!gekkota_memory_is_initialized())
    {
        errno = GEKKOTA_ERROR_MEMORY_NOT_INITIALIZED;
        return NULL;
    }

    if (size < 1)
    {
        errno = GEKKOTA_ERROR_ARGUMENT_NOT_VALID;
        return NULL;
    }

    if (size > fastHeap.blockSize || gekkota_list_is_empty(&fastHeap.freeBlocks))
    {
        memory = malloc(size);
    }
    else
    {
        memory = gekkota_list_remove(gekkota_list_first(&fastHeap.freeBlocks));
        gekkota_list_add(&fastHeap.allocatedBlocks, memory);
    }

    memory = (void_t *) ((byte_t *) memory + sizeof(GekkotaListNode));

    if (initialize)
        memset(memory, 0x00, size);

    return memory;
}

void_t *
gekkota_memory_realloc(void_t *memory, size_t newSize, bool_t initialize)
{
    size_t memorySize;

    if (!gekkota_memory_is_initialized())
    {
        errno = GEKKOTA_ERROR_MEMORY_NOT_INITIALIZED;
        return NULL;
    }

    if (memory == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return NULL;
    }

    if (newSize < 1)
    {
        errno = GEKKOTA_ERROR_ARGUMENT_NOT_VALID;
        return NULL;
    }

    memorySize = fastHeap.blockCount * (fastHeap.blockSize + sizeof(GekkotaListNode));

    if (fastHeap.memory < memory && memory < (void_t *) ((byte_t *) fastHeap.memory + memorySize))
    {
        if (newSize > fastHeap.blockSize)
        {
            GekkotaListIterator iterator = (GekkotaListIterator) ((byte_t *) memory - sizeof(GekkotaListNode));
            gekkota_list_add(&fastHeap.freeBlocks, gekkota_list_remove(iterator));

            if ((memory = malloc(newSize)) == NULL)
                errno = GEKKOTA_ERROR_OUT_OF_MEMORY;
        }
    }
    else
    {
        if ((memory = realloc(memory, newSize)) == NULL)
            errno = GEKKOTA_ERROR_OUT_OF_MEMORY;
    }

    if (initialize)
        memset(memory, 0x00, newSize);

    return memory;
}

void_t
gekkota_memory_free(void_t *memory)
{
    size_t memorySize;

    if (!gekkota_memory_is_initialized() || memory == NULL)
        return;

    memorySize = fastHeap.blockCount * (fastHeap.blockSize + sizeof(GekkotaListNode));

    if (fastHeap.memory < memory && memory < (void_t *) ((byte_t *) fastHeap.memory + memorySize))
    {
        GekkotaListIterator iterator = (GekkotaListIterator) ((byte_t *) memory - sizeof(GekkotaListNode));
        gekkota_list_add(&fastHeap.freeBlocks, gekkota_list_remove(iterator));
    }
    else
    {
        free(memory);
    }
}