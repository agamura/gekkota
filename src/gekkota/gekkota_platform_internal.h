/******************************************************************************
 * @file    gekkota_platform_internal.h
 * @date    11-Aug-2007
 * @author  <a href="mailto:giuseppe.greco@agamura.com">Giuseppe Greco</a>
 *
 * Copyright (C) 2007 Agamura, Inc. - http://www.agamura.com
 * All right reserved.
 ******************************************************************************/

#ifndef __GEKKOTA_PLATFORM_INTERNAL_H__
#define __GEKKOTA_PLATFORM_INTERNAL_H__

#include "gekkota/gekkota_types.h"
#include "gekkota/gekkota_module.h"
#include "gekkota/gekkota_platform.h"

typedef struct _GekkotaLoading
{
    GekkotaPlatformFeature  feature;
    bool_t                  isOptional;
    char_t                  *moduleName;
    GekkotaModule           module;
    int32_t (GEKKOTA_CALLBACK *initialize) (const GekkotaModule);
} GekkotaLoading;

struct _GekkotaPlatform
{
    int32_t             type;
    int32_t             majorVersion;
    int32_t             minorVersion;
    int32_t             buildNumber;
    GekkotaLoading      *loadings;
    size_t              loadingCount;
};

extern GekkotaPlatform platform;

extern int32_t
_gekkota_platform_initialize(void_t);

extern int32_t
_gekkota_platform_uninitialize(void_t);

extern int32_t
_gekkota_platform_load_modules(
        GekkotaLoading *restrict loadings,
        size_t loadingCount);

extern int32_t
_gekkota_platform_unload_modules(void_t);

#endif /* !__GEKKOTA_PLATFORM_INTERNAL_H__ */
