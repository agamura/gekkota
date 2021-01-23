/******************************************************************************
 * @file    gekkota_buffer.h
 * @date    20-Oct-2006
 * @author  <a href="mailto:giuseppe.greco@agamura.com">Giuseppe Greco</a>
 *
 * Copyright (C) 2006 Agamura, Inc. - http://www.agamura.com
 * All right reserved.
 ******************************************************************************/

#ifndef __GEKKOTA_BUFFER_H__
#define __GEKKOTA_BUFFER_H__

#include "gekkota/gekkota_types.h"

typedef struct _GekkotaBuffer
{
#ifdef WIN32
    size_t  length;
    void_t  *data;
#else
    void_t  *data;
    size_t  length;
#endif /* WIN32 */
} GekkotaBuffer;

GEKKOTA_API int32_t
gekkota_buffer_malloc(GekkotaBuffer *restrict buffer, size_t size, bool_t initialize);

GEKKOTA_API int32_t
gekkota_buffer_free(GekkotaBuffer *buffer);

GEKKOTA_API int32_t
gekkota_buffer_resize(GekkotaBuffer *restrict buffer, size_t newSize, bool_t initialize);

GEKKOTA_API int32_t
gekkota_buffer_copy(GekkotaBuffer *restrict buffer, const GekkotaBuffer *value);

GEKKOTA_API bool_t
gekkota_buffer_equals(const GekkotaBuffer *buffer, const GekkotaBuffer *value);

GEKKOTA_API int32_t
gekkota_buffer_clear(GekkotaBuffer *restrict buffer);

#endif /* !__GEKKOTA_BUFFER_H__ */
