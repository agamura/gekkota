/******************************************************************************
 * @file    gekkota_platform_win32.c
 * @date    11-Aug-2007
 * @author  <a href="mailto:giuseppe.greco@agamura.com">Giuseppe Greco</a>
 *
 * Copyright (C) 2007 Agamura, Inc. - http://www.agamura.com
 * All right reserved.
 ******************************************************************************/

#include <errno.h>
#include <string.h>
#include "gekkota.h"
#include "gekkota_errors.h"
#include "gekkota_idn.h"
#include "gekkota_platform.h"

static GekkotaLoading loadings[] =
{
    { GEKKOTA_PLATFORM_FEATURE_IDN,
        TRUE, (char_t *) L"normaliz.dll", NULL, gekkota_idn_initialize }
};

int32_t
_gekkota_platform_initialize(void_t)
{
    OSVERSIONINFO osVersionInfo;

    memset(&osVersionInfo, 0x00, sizeof(OSVERSIONINFO));
    osVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

    if (GetVersionEx(&osVersionInfo) == 0)
    {
        errno = GEKKOTA_ERROR_UNKNOWN_PLATFORM;
        return -1;
    }

    /*
     * Support Windows XP (0x0501) or higher.
     */
    if ((osVersionInfo.dwMajorVersion < 5) ||
        (osVersionInfo.dwMajorVersion == 5 && osVersionInfo.dwMinorVersion < 1))
    {
        errno = GEKKOTA_ERROR_PLATFORM_NOT_SUPPORTED;
        return -1;
    }

    platform.majorVersion = (int32_t) osVersionInfo.dwMajorVersion;
    platform.minorVersion = (int32_t) osVersionInfo.dwMinorVersion;
    platform.buildNumber = (int32_t) osVersionInfo.dwBuildNumber;

    switch (osVersionInfo.dwPlatformId)
    {
        case VER_PLATFORM_WIN32_NT:
            platform.type = GEKKOTA_PLATFORM_TYPE_WINDOWS_WINNT;
            break;

        default:
            platform.type = GEKKOTA_PLATFORM_TYPE_WINDOWS;
            break;
    }

    return _gekkota_platform_load_modules(
            loadings, sizeof(loadings) / sizeof(GekkotaLoading));
}

int32_t
_gekkota_platform_uninitialize(void_t)
{
    return _gekkota_platform_unload_modules();
}
