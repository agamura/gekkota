/******************************************************************************
 * @file    gekkota_string_internal.h
 * @date    19-Sep-2007
 * @author  <a href="mailto:giuseppe.greco@agamura.com">Giuseppe Greco</a>
 *
 * Copyright (C) 2007 Agamura, Inc. - http://www.agamura.com
 * All right reserved.
 ******************************************************************************/

#ifndef __GEKKOTA_STRING_INTERNAL_H__
#define __GEKKOTA_STRING_INTERNAL_H__

#include "gekkota/gekkota_buffer.h"
#include "gekkota/gekkota_list.h"
#include "gekkota/gekkota_string.h"

typedef enum
{
    GEKKOTA_STRING_FLAG_NONE    = 0,
    GEKKOTA_STRING_FLAG_UTF8    = (1 << 0),
    GEKKOTA_STRING_FLAG_CONST   = (1 << 1),
} GekkotaStringFlag;

struct _GekkotaString
{
    GekkotaListNode listNode;
    byte_t          flags;
    GekkotaBuffer   value;
    size_t          length;
    uint32_t        refCount;
};

extern int32_t
_gekkota_string_to_wcs(const GekkotaBuffer *mbs, GekkotaBuffer *wcs);

extern int32_t
_gekkota_string_to_mbs(const GekkotaBuffer *wcs, GekkotaBuffer *mbs);

#endif /* !__GEKKOTA_STRING_INTERNAL_H__ */
