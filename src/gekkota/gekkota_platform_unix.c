/******************************************************************************
 * @file    gekkota_platform_unix.c
 * @date    11-Aug-2007
 * @author  <a href="mailto:giuseppe.greco@agamura.com">Giuseppe Greco</a>
 *
 * Copyright (C) 2007 Agamura, Inc. - http://www.agamura.com
 * All right reserved.
 ******************************************************************************/

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "gekkota.h"
#include "gekkota_errors.h"
#include "gekkota_idn.h"
#include "gekkota_platform.h"

#define GEKKOTA_PLATFORM_LINUX          "Linux"
#define GEKKOTA_PLATFORM_FREEBSD        "FreeBSD"
#define GEKKOTA_PLATFORM_NETBSD         "NetBSD"
#define GEKKOTA_PLATFORM_OPENBSD        "OpenBSD"
#define GEKKOTA_PLATFORM_APPLE          "Darwin"
#define GEKKOTA_PLATFORM_SYMBIAN        "Symbian"

#define GEKKOTA_PLATFORM_MAJOR_VERSION  1
#define GEKKOTA_PLATFORM_MINOR_VERSION  2
#define GEKKOTA_PLATFORM_BUILD_NUMBER   3

static GekkotaLoading loadings[] =
{
#ifndef HAVE_NATIVE_IDN
    { GEKKOTA_PLATFORM_FEATURE_IDN,
        TRUE, "libidn.so", NULL, gekkota_idn_initialize }
#endif /* !HAVE_NATIVE_IDN */
};

int32_t
_gekkota_platform_initialize(void_t)
{
    struct utsname utsname;
    char_t delimiters[] = ".\0";
    char_t *token;
    int32_t tokenCount = 0;

    if (uname(&utsname) < 0)
    {
        errno = GEKKOTA_ERROR_UNKNOWN_PLATFORM;
        return -1;
    }

    /*
     * Extract major version, minor version, and build number.
     */
    token = strtok(utsname.release, delimiters);
    while (token != NULL && tokenCount < GEKKOTA_PLATFORM_BUILD_NUMBER)
    {
        switch (++tokenCount)
        {
            case GEKKOTA_PLATFORM_MAJOR_VERSION:
                platform.majorVersion = atoi(token);
                break;

            case GEKKOTA_PLATFORM_MINOR_VERSION:
                platform.minorVersion = atoi(token);
                break;

            default: /* GEKKOTA_PLATFORM_BUILD_NUMBER */
                platform.buildNumber = atoi(token);
                break;
        }

        token = strtok(NULL, delimiters);
    }

    if (strcmp(utsname.sysname, GEKKOTA_PLATFORM_LINUX) == 0)
    {
        /* Linux */
        if (platform.majorVersion < 2)
        {
            errno = GEKKOTA_ERROR_PLATFORM_NOT_SUPPORTED;
            return -1;
        }

        platform.type = GEKKOTA_PLATFORM_TYPE_LINUX;
    }
    else if (strcmp(utsname.sysname, GEKKOTA_PLATFORM_FREEBSD) == 0)
    {
        /* FreeBSD */
        if (platform.majorVersion < 4)
        {
            errno = GEKKOTA_ERROR_PLATFORM_NOT_SUPPORTED;
            return -1;
        }

        platform.type = GEKKOTA_PLATFORM_TYPE_FREEBSD;
    }
    else if (strcmp(utsname.sysname, GEKKOTA_PLATFORM_NETBSD) == 0)
    {
        /* NetBSD */
        if ((platform.majorVersion < 1) ||
                (platform.majorVersion == 1 && platform.minorVersion < 5))
        {
            errno = GEKKOTA_ERROR_PLATFORM_NOT_SUPPORTED;
            return -1;
        }

        platform.type= GEKKOTA_PLATFORM_TYPE_OPENBSD;
    }
    else if (strcmp(utsname.sysname, GEKKOTA_PLATFORM_OPENBSD) == 0)
    {
        /* OpenBSD */
        if (platform.majorVersion < 2)
        {
            errno = GEKKOTA_ERROR_PLATFORM_NOT_SUPPORTED;
            return -1;
        }

        platform.type = GEKKOTA_PLATFORM_TYPE_OPENBSD;
    }
    else if (strcmp(utsname.sysname, GEKKOTA_PLATFORM_APPLE) == 0)
    {
        /* Darwin */
        if ((platform.majorVersion < 7) ||
                (platform.majorVersion == 7 && platform.minorVersion < 3))
        {
            errno = GEKKOTA_ERROR_PLATFORM_NOT_SUPPORTED;
            return -1;
        }

        platform.type = GEKKOTA_PLATFORM_TYPE_APPLE;
    }
    else if (strcmp(utsname.sysname, GEKKOTA_PLATFORM_SYMBIAN) == 0)
    {
        /* Symbian */
        if (platform.majorVersion < 7)
        {
            errno = GEKKOTA_ERROR_PLATFORM_NOT_SUPPORTED;
            return -1;
        }

        platform.type = GEKKOTA_PLATFORM_TYPE_SYMBIAN;
    }
    else
        /* Generic UNIX */
        platform.type = GEKKOTA_PLATFORM_TYPE_UNIX;

    return _gekkota_platform_load_modules(
            loadings, sizeof(loadings) / sizeof(GekkotaLoading));

}

int32_t
_gekkota_platform_uninitialize(void_t)
{
    return _gekkota_platform_unload_modules();
}
