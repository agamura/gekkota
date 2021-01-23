/******************************************************************************
 * @file    gekkota_platform.c
 * @date    11-Aug-2007
 * @author  <a href="mailto:giuseppe.greco@agamura.com">Giuseppe Greco</a>
 *
 * Copyright (C) 2007 Agamura, Inc. - http://www.agamura.com
 * All right reserved.
 ******************************************************************************/

#include <errno.h>
#include "gekkota.h"
#include "gekkota_errors.h"
#include "gekkota_module.h"
#include "gekkota_platform.h"

GekkotaPlatform platform;

int32_t
gekkota_platform_get_type()
{
    if (!gekkota_is_initialized())
    {
        errno = GEKKOTA_ERROR_LIB_NOT_INITIALIZED;
        return -1;
    }

    return platform.type;
}

int32_t
gekkota_platorm_get_major_version()
{
    if (!gekkota_is_initialized())
    {
        errno = GEKKOTA_ERROR_LIB_NOT_INITIALIZED;
        return -1;
    }

    return platform.majorVersion;
}

int32_t
gekkota_platform_get_minor_version()
{
    if (!gekkota_is_initialized())
    {
        errno = GEKKOTA_ERROR_LIB_NOT_INITIALIZED;
        return -1;
    }

    return platform.minorVersion;
}

int32_t
gekkota_platform_get_build_number()
{
    if (!gekkota_is_initialized())
    {
        errno = GEKKOTA_ERROR_LIB_NOT_INITIALIZED;
        return -1;
    }

    return platform.buildNumber;
}

bool_t
gekkota_platform_supports_feature(GekkotaPlatformFeature feature)
{
    uint32_t i;
    errno = GEKKOTA_ERROR_SUCCESS;

    if (platform.loadingCount == 0)
        return FALSE;

    for (i = 0; i < platform.loadingCount; i++)
    {
        if (platform.loadings[i].feature == feature)
            return platform.loadings[i].module != NULL ? TRUE : FALSE;
    }

    errno = GEKKOTA_ERROR_ARGUMENT_NOT_VALID;
    return FALSE;
}

int32_t
_gekkota_platform_load_modules(
        GekkotaLoading *restrict loadings,
        size_t loadingCount)
{
    uint32_t i;

    if (loadingCount == 0)
        return 0;

    for (i = 0; i < loadingCount; i++)
    {
        if ((loadings[i].module = gekkota_module_load(
                loadings[i].moduleName)) != NULL)
        {
            /*
             * Initialize the function pointers associated with the module
             * just loaded.
             */
            loadings[i].initialize(loadings[i].module);
        }
        else
        {
            uint32_t j;

            if (loadings[i].isOptional)
            {
                /*
                 * A feature might require more than one module; in such a case,
                 * if one loading fails, then all the modules associated with
                 * that feature loaded previously should be unloaded in order to
                 * keep things consistent.
                 */
                for (j = 0; j < i; j++)
                {
                    if (loadings[j].feature == loadings[i].feature)
                    {
                        gekkota_module_unload(loadings[j].module);
                        loadings[j].module = NULL;
                    }
                }
            }
            else
            {
                /*
                 * The current loading is not optional, so execution cannot
                 * continue. Unload all the modules loaded previously and
                 * return error.
                 */
                for (j = 0; j < i; j++)
                    gekkota_module_unload(loadings[j].module);

                return -1;
            }
        }
    }

    platform.loadings = loadings;
    platform.loadingCount = loadingCount;

    return 0;
}

int32_t
_gekkota_platform_unload_modules(void_t)
{
    uint32_t i;
    int32_t rc = 0;

    if (platform.loadingCount == 0)
        return 0;

    for (i = 0; i < platform.loadingCount; i++)
        if (platform.loadings[i].module != NULL)
            if (gekkota_module_unload(platform.loadings[i].module) < 0)
                rc = -1;

    return rc;
}
