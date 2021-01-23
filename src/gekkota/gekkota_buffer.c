/******************************************************************************
 * @file    gekkota_buffer.c
 * @date    20-Oct-2006
 * @author  <a href="mailto:giuseppe.greco@agamura.com">Giuseppe Greco</a>
 *
 * Copyright (C) 2006 Agamura, Inc. - http://www.agamura.com
 * All right reserved.
 ******************************************************************************/

#include <errno.h>
#include <string.h>
#include "gekkota_buffer.h"
#include "gekkota_errors.h"
#include "gekkota_memory.h"

int32_t
gekkota_buffer_malloc(GekkotaBuffer *restrict buffer, size_t size, bool_t initialize)
{
    if (buffer == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return -1;
    }

    if ((buffer->data = gekkota_memory_alloc(size, initialize)) == NULL)
        return -1;

    buffer->length = size;
    return 0;
}

int32_t
gekkota_buffer_free(GekkotaBuffer *buffer)
{
    if (buffer == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return -1;
    }

    gekkota_memory_free(buffer->data);
    buffer->data = NULL;
    buffer->length = 0;

    return 0;
}

int32_t
gekkota_buffer_resize(GekkotaBuffer *restrict buffer, size_t newSize, bool_t initialize)
{
    void_t *data;

    if (buffer == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return -1;
    }

    if ((data = gekkota_memory_realloc(buffer->data, newSize, initialize)) == NULL)
        return -1;

    buffer->data = data;
    buffer->length = newSize;

    return 0;
}

int32_t
gekkota_buffer_copy(GekkotaBuffer *restrict buffer, const GekkotaBuffer *value)
{
    if (buffer == NULL || value == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return -1;
    }

    if (buffer->data == NULL || value->data == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_BUFFER;
        return -1;
    }

    if (buffer->length == 0 || value->length)
    {
        errno = GEKKOTA_ERROR_ZERO_LENGTH_BUFFER;
        return -1;
    }

    if (buffer->length < value->length)
    {
        errno = GEKKOTA_ERROR_BUFFER_OVERFLOW;
        return -1;
    }

    if (buffer != value)
        memmove(buffer->data, value->data, value->length);

    return 0;
}

bool_t
gekkota_buffer_equals(const GekkotaBuffer *buffer, const GekkotaBuffer *value)
{
    if (buffer == value)
        return TRUE;

    if (buffer == NULL || value == NULL)
        return FALSE;

    if (buffer->data == value->data)
        return TRUE;

    if (buffer->data == NULL || value->data == NULL)
        return FALSE;

    if (buffer->length != value->length)
        return FALSE;

    return memcmp(buffer->data, value->data, buffer->length) == 0;
}

int32_t
gekkota_buffer_clear(GekkotaBuffer *restrict buffer)
{
    if (buffer == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return -1;
    }

    memset(buffer->data, 0x00, buffer->length);
    return 0;
}
