/******************************************************************************
 * @file    gekkota_module_win32.c
 * @date    08-Sep-2007
 * @author  <a href="mailto:giuseppe.greco@agamura.com">Giuseppe Greco</a>
 *
 * Copyright (C) 2007 Agamura, Inc. - http://www.agamura.com
 * All right reserved.
 ******************************************************************************/

#include <errno.h>
#include "gekkota/gekkota.h"
#include "gekkota/gekkota_errors.h"
#include "gekkota/gekkota_module.h"

GEKKOTA_API GekkotaModule
gekkota_module_load(char_t *moduleName)
{
    GekkotaModule module;

    if (moduleName == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return NULL;
    }

    if ((module = (GekkotaModule) LoadLibrary((wchar_t *) moduleName)) == NULL)
    {
        errno = GEKKOTA_ERROR_RESOURCE_NOT_AVAILABLE;
        return NULL;
    }

    return module;
}

GEKKOTA_API int32_t
gekkota_module_unload(GekkotaModule module)
{
    if (module == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return -1;
    }

    if (FreeLibrary((HMODULE) module) == 0)
    {
        errno = GEKKOTA_ERROR_CANNOT_RELEASE_RESOURCE;
        return -1;
    }

    return 0;
}

GEKKOTA_API GekkotaFunctionPtr
gekkota_module_get_function_ptr(GekkotaModule module, char_t *functionName)
{
    GekkotaFunctionPtr functionPtr;

    if (module == NULL || functionName == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return NULL;
    }

    if ((functionPtr = (GekkotaFunctionPtr) GetProcAddress(
            (HMODULE) module, functionName)) == NULL)
    {
        errno = GEKKOTA_ERROR_RESOURCE_NOT_AVAILABLE;
        return NULL;
    }

    return functionPtr;
}
