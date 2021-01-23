/******************************************************************************
 * @file    gekkota_string.c
 * @date    19-Sep-2007
 * @author  <a href="mailto:giuseppe.greco@agamura.com">Giuseppe Greco</a>
 *
 * Copyright (C) 2007 Agamura, Inc. - http://www.agamura.com
 * All right reserved.
 ******************************************************************************/

#include <errno.h>
#include <string.h>
#include "gekkota_errors.h"
#include "gekkota_memory.h"
#include "gekkota_string.h"
#include "gekkota_utils.h"

GekkotaString *cast_mbs(const char_t *value)
{
    return NULL;
}

GekkotaString *cast_wcs(const wchar_t *value)
{
    return NULL;
}

GekkotaString *
gekkota_string_new(void_t)
{
    return NULL;
}

GekkotaString *
gekkota_string_new_0(GekkotaString *restrict string, bool_t deep)
{
    return NULL;
}

GekkotaString *
gekkota_string_new_1(size_t initialCapacity)
{
    return NULL;
}

GekkotaString *
gekkota_string_new_2(const GekkotaString *value)
{
    return NULL;
}

int32_t
gekkota_string_destroy(GekkotaString *string)
{
    return 0;
}

int32_t
gekkota_string_append(GekkotaString *string, const GekkotaString *value)
{
    return 0;
}

int32_t
gekkota_string_clear(GekkotaString *string)
{
    return 0;
}

int32_t
gekkota_string_copy(GekkotaString *string, const GekkotaString *value)
{
    return 0;
}

bool_t
gekkota_string_equals(const GekkotaString *string, const GekkotaString *value)
{
    return FALSE;
}

int32_t
gekkota_string_get_length_1(
        const GekkotaString *string,
        GekkotaStringLengthType lengthType)
{
    return 0;
}

size_t
gekkota_string_get_capacity(const GekkotaString *string)
{
    return 0;
}

int32_t
gekkota_string_set_capacity(const GekkotaString *string, size_t newCapacity)
{
    return 0;
}

uint16_t
gekkota_string_hash_16(const GekkotaString *string)
{
    return 0;
}

uint32_t
gekkota_string_hash_32(const GekkotaString *string)
{
    return 0;
}

bool_t
gekkota_string_is_empty(const GekkotaString *string)
{
    return TRUE;
}

int32_t
gekkota_string_insert(
        GekkotaString *string,
        const GekkotaString *value,
        uint32_t index)
{
    return 0;
}

GekkotaString *
gekkota_string_substring(
        const GekkotaString *string,
        uint32_t startIndex,
        uint32_t count)
{
    return NULL;
}

int32_t
gekkota_string_prepend(GekkotaString *string, const GekkotaString *value)
{
    return 0;
}

int32_t
gekkota_string_to_lower(GekkotaString *string)
{
    return 0;
}

int32_t
gekkota_string_to_upper(GekkotaString *string)
{
    return 0;
}

char_t *
gekkota_string_mbs(GekkotaString *restrict string)
{
    return NULL;
}

wchar_t *
gekkota_string_wcs(GekkotaString *restrict string)
{
    return NULL;
}
