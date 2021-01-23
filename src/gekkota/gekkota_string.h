/******************************************************************************
 * @file    gekkota_string.h
 * @date    19-Sep-2007
 * @author  <a href="mailto:giuseppe.greco@agamura.com">Giuseppe Greco</a>
 *
 * Copyright (C) 2007 Agamura, Inc. - http://www.agamura.com
 * All right reserved.
 ******************************************************************************/

#ifndef __GEKKOTA_STRING_H__
#define __GEKKOTA_STRING_H__

#include "gekkota/gekkota_types.h"

typedef struct _GekkotaString GekkotaString;

typedef enum
{
    GEKKOTA_STRING_LENGTH_TYPE_UNDEFINED = 0,
    GEKKOTA_STRING_LENGTH_TYPE_CHARS,
    GEKKOTA_STRING_LENGTH_TYPE_BYTES
} GekkotaStringLengthType;

GEKKOTA_API GekkotaString *cast_mbs(const char_t *value);
GEKKOTA_API GekkotaString *cast_wcs(const wchar_t *value);

GEKKOTA_API GekkotaString *
gekkota_string_new(void_t);

GEKKOTA_API GekkotaString *
gekkota_string_new_0(GekkotaString *restrict string, bool_t deep);

GEKKOTA_API GekkotaString *
gekkota_string_new_1(size_t initialCapacity);

GEKKOTA_API GekkotaString *
gekkota_string_new_2(const GekkotaString *value);

GEKKOTA_API int32_t
gekkota_string_destroy(GekkotaString *string);

GEKKOTA_API int32_t
gekkota_string_append(GekkotaString *string, const GekkotaString *value);

GEKKOTA_API int32_t
gekkota_string_clear(GekkotaString *string);

GEKKOTA_API int32_t
gekkota_string_copy(GekkotaString *string, const GekkotaString *value);

GEKKOTA_API bool_t
gekkota_string_equals(const GekkotaString *string, const GekkotaString *value);

#define gekkota_string_get_length(string) \
    (gekkota_string_get_length_1(string, GEKKOTA_STRING_LENGTH_TYPE_CHARS))

GEKKOTA_API int32_t
gekkota_string_get_length_1(
        const GekkotaString *string,
        GekkotaStringLengthType lengthType);

GEKKOTA_API size_t
gekkota_string_get_capacity(const GekkotaString *string);

GEKKOTA_API int32_t
gekkota_string_set_capacity(const GekkotaString *string, size_t newCapacity);

GEKKOTA_API uint16_t
gekkota_string_hash_16(const GekkotaString *string);

GEKKOTA_API uint32_t
gekkota_string_hash_32(const GekkotaString *string);

GEKKOTA_API bool_t
gekkota_string_is_empty(const GekkotaString *string);

GEKKOTA_API int32_t
gekkota_string_insert(
        GekkotaString *string,
        const GekkotaString *value,
        uint32_t index);

GEKKOTA_API GekkotaString *
gekkota_string_substring(
        const GekkotaString *string,
        uint32_t startIndex,
        uint32_t count);

GEKKOTA_API int32_t
gekkota_string_prepend(GekkotaString *string, const GekkotaString *value);

GEKKOTA_API int32_t
gekkota_string_to_lower(GekkotaString *string);

GEKKOTA_API int32_t
gekkota_string_to_upper(GekkotaString *string);

GEKKOTA_API char_t *
gekkota_string_mbs(GekkotaString *restrict string);

GEKKOTA_API wchar_t *
gekkota_string_wcs(GekkotaString *restrict string);

#if defined (GEKKOTA_BUILDING_LIB) || defined (GEKKOTA_BUILDING_STATIC_LIB)
#include "gekkota_string_internal.h"
#endif /* GEKKOTA_BUILDING_LIB || GEKKOTA_BUILDING_STATIC_LIB */

#endif /* !__GEKKOTA_STRING_H__ */
