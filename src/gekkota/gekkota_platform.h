/******************************************************************************
 * @file    gekkota_platform.h
 * @date    11-Aug-2007
 * @author  <a href="mailto:giuseppe.greco@agamura.com">Giuseppe Greco</a>
 *
 * Copyright (C) 2007 Agamura, Inc. - http://www.agamura.com
 * All right reserved.
 ******************************************************************************/

#ifndef __GEKKOTA_PLATFORM_H__
#define __GEKKOTA_PLATFORM_H__

#include "gekkota/gekkota_types.h"

typedef enum
{
    GEKKOTA_PLATFORM_TYPE_UNDEFINED     = 0x00000000,
    GEKKOTA_PLATFORM_TYPE_UNIX          = 0x00010000,
    GEKKOTA_PLATFORM_TYPE_LINUX         = GEKKOTA_PLATFORM_TYPE_UNIX | 0x00000001,
    GEKKOTA_PLATFORM_TYPE_FREEBSD       = GEKKOTA_PLATFORM_TYPE_UNIX | 0x00000002,
    GEKKOTA_PLATFORM_TYPE_OPENBSD       = GEKKOTA_PLATFORM_TYPE_UNIX | 0x00000003,
    GEKKOTA_PLATFORM_TYPE_APPLE         = GEKKOTA_PLATFORM_TYPE_UNIX | 0x00000004,
    GEKKOTA_PLATFORM_TYPE_SYMBIAN       = GEKKOTA_PLATFORM_TYPE_UNIX | 0x00000005,
    GEKKOTA_PLATFORM_TYPE_WINDOWS       = 0x00020000,
    GEKKOTA_PLATFORM_TYPE_WINDOWS_WINNT = GEKKOTA_PLATFORM_TYPE_WINDOWS | 0x00000001
} GekkotaPlatformType;

typedef enum
{
    GEKKOTA_PLATFORM_FEATURE_UNDEFINED  = 0,
    GEKKOTA_PLATFORM_FEATURE_IDN        = 1
} GekkotaPlatformFeature;

typedef struct _GekkotaPlatform GekkotaPlatform;

GEKKOTA_API int32_t
gekkota_platform_get_type(void_t);

GEKKOTA_API int32_t
gekkota_platform_get_major_version(void_t);

GEKKOTA_API int32_t
gekkota_platform_get_minor_version(void_t);

GEKKOTA_API int32_t
gekkota_platform_get_build_number(void_t);

GEKKOTA_API bool_t
gekkota_platform_supports_feature(GekkotaPlatformFeature feature);

#if defined (GEKKOTA_BUILDING_LIB) || defined (GEKKOTA_BUILDING_STATIC_LIB)
#include "gekkota_platform_internal.h"
#endif /* GEKKOTA_BUILDING_LIB || GEKKOTA_BUILDING_STATIC_LIB */

#endif /* !__GEKKOTA_PLATFORM_H__ */
