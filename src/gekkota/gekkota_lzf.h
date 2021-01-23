/******************************************************************************
 * @file    gekkota_lzf.h
 * @date    22-Jul-2007
 * @author  <a href="mailto:giuseppe.greco@agamura.com">Giuseppe Greco</a>
 *
 * Copyright (C) 2007 Agamura, Inc. - http://www.agamura.com
 * All right reserved.
 ******************************************************************************/

#ifndef __GEKKOTA_LZF_H__
#define __GEKKOTA_LZF_H__

#include "gekkota/gekkota_buffer.h"
#include "gekkota/gekkota_types.h"

typedef enum
{
    GEKKOTA_COMPRESSION_LEVEL_UNDEFINED = 0x00,
    GEKKOTA_COMPRESSION_LEVEL_FASTEST   = 0x0D,
    GEKKOTA_COMPRESSION_LEVEL_VERY_FAST = 0x0E,
    GEKKOTA_COMPRESSION_LEVEL_FAST      = 0x0F,
    GEKKOTA_COMPRESSION_LEVEL_GOOD      = 0x10,
    GEKKOTA_COMPRESSION_LEVEL_VERY_GOOD = 0x11,
    GEKKOTA_COMPRESSION_LEVEL_BEST      = 0x12
} GekkotaCompressionLevel;

typedef struct _GekkotaLZF GekkotaLZF;

#define gekkota_lzf_new() \
    (gekkota_lzf_new_1(GEKKOTA_COMPRESSION_LEVEL_FAST))

GEKKOTA_API GekkotaLZF *
gekkota_lzf_new_0(GekkotaLZF *restrict lzf);

GEKKOTA_API GekkotaLZF *
gekkota_lzf_new_1(GekkotaCompressionLevel compressionLevel);

GEKKOTA_API int32_t
gekkota_lzf_destroy(GekkotaLZF *lzf);

GEKKOTA_API GekkotaCompressionLevel
gekkota_lzf_get_compression_level(const GekkotaLZF *lzf);

GEKKOTA_API int32_t
gekkota_lzf_set_compression_level(
        GekkotaLZF *restrict lzf,
        GekkotaCompressionLevel compressionLevel);

GEKKOTA_API int32_t
gekkota_lzf_get_inflated_length(
        const GekkotaLZF *lzf,
        const GekkotaBuffer *deflated);

GEKKOTA_API int32_t
gekkota_lzf_get_max_deflated_length(
        const GekkotaLZF *lzf,
        const GekkotaBuffer *source);

GEKKOTA_API int32_t
gekkota_lzf_deflate(
        const GekkotaLZF *lzf,
        const GekkotaBuffer *source,
        GekkotaBuffer *restrict deflated);

GEKKOTA_API int32_t
gekkota_lzf_inflate(
        const GekkotaLZF *lzf,
        const GekkotaBuffer *source,
        GekkotaBuffer *restrict inflated);

#endif /* __GEKKOTA_LZF_H__ */
