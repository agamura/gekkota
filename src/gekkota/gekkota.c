/******************************************************************************
 * @file    gekkota.c
 * @date    11-Nov-2006
 * @author  <a href="mailto:giuseppe.greco@agamura.com">Giuseppe Greco</a>
 *
 * Copyright (C) 2006 Agamura, Inc. - http://www.agamura.com
 * All right reserved.
 ******************************************************************************/

#include <errno.h>
#include <string.h>
#include "gekkota.h"
#include "gekkota_errors.h"
#include "gekkota_platform.h"
#include "gekkota_time.h"

static bool_t isInitialized = FALSE;
static time_t baseTime = 0;

#define _gekkota_hash(string, type_t, mask) \
{ \
    size_t length; \
    register type_t hashCode = 0; \
    if (string != NULL && ((length = strlen(string)) > 0)) \
    { \
        uint32_t i; \
        register type_t shift = 0; \
        for (i = 0; i < length; i++) \
        { \
            hashCode = (type_t) ((hashCode << 4) + string[i]); \
            shift = (type_t) (hashCode & mask); \
            if (shift != 0) \
            { \
                hashCode ^= (type_t) (shift >> 8); \
                hashCode ^= shift; \
            } \
        } \
    } \
    return hashCode; \
}

#if __BYTE_ORDER == __LITTLE_ENDIAN
static uint64_t
_gekkota_swap_64(register uint64_t number)
{
    byte_t b0 = (byte_t) ((number >> 56) & 0xFF);
    byte_t b1 = (byte_t) ((number >> 48) & 0xFF);
    byte_t b2 = (byte_t) ((number >> 40) & 0xFF);
    byte_t b3 = (byte_t) ((number >> 32) & 0xFF);
    byte_t b4 = (byte_t) ((number >> 24) & 0xFF);
    byte_t b5 = (byte_t) ((number >> 16) & 0xFF);
    byte_t b6 = (byte_t) ((number >> 8) & 0xFF);
    byte_t b7 = (byte_t) (number & 0xFF);

    return (uint64_t) b0 + ((uint64_t) b1 << 8) +
        ((uint64_t) b2 << 16) + ((uint64_t) b3 << 24) +
        ((uint64_t) b4 << 32) + ((uint64_t) b5 << 40) +
        ((uint64_t) b6 << 48) + ((uint64_t) b7 << 56);
}
#endif /* __BYTE_ORDER */

int32_t
gekkota_initialize(void_t)
{
    if (isInitialized)
    {
        errno = GEKKOTA_ERROR_LIB_ALREADY_INITIALIZED;
        return -1;
    }

    if (_gekkota_initialize() != 0)
        return -1;

    if (_gekkota_platform_initialize() != 0)
        return -1;

    isInitialized = TRUE;
    return 0;
}

int32_t
gekkota_uninitialize(void_t)
{
    int32_t rc1, rc2;

    if (!isInitialized)
    {
        errno = GEKKOTA_ERROR_LIB_NOT_INITIALIZED;
        return -1;
    }

    rc1 = _gekkota_uninitialize();
    rc2 = _gekkota_platform_uninitialize();

    isInitialized = FALSE;
    return rc1 == 0 && rc2 == 0 ? 0 : -1;
}

bool_t
gekkota_is_initialized(void_t)
{
    return isInitialized;
}

time_t
gekkota_get_time(void_t)
{
    if (!gekkota_is_initialized())
    {
        errno = GEKKOTA_ERROR_LIB_NOT_INITIALIZED;
        return ~0;
    }

    return gekkota_time_now() - baseTime;
}

int32_t
gekkota_set_time(time_t time)
{
    if (!gekkota_is_initialized())
    {
        errno = GEKKOTA_ERROR_LIB_NOT_INITIALIZED;
        return -1;
    }

    baseTime = gekkota_time_now() - time;
    return 0;
}

uint16_t
gekkota_host_to_net_16(uint16_t host)
{
    return htons(host);
}

uint32_t
gekkota_host_to_net_32(uint32_t host)
{
    return htonl(host);
}

uint64_t
gekkota_host_to_net_64(uint64_t host)
{
#if __BYTE_ORDER == __LITTLE_ENDIAN
    return _gekkota_swap_64(host);
#else
    return host;
#endif /* __BYTE_ORDER */
}

uint16_t
gekkota_net_to_host_16(uint16_t net)
{
    return ntohs(net);
}

uint32_t
gekkota_net_to_host_32(uint32_t net)
{
    return ntohl(net);
}

uint64_t
gekkota_net_to_host_64(uint64_t net)
{
#if __BYTE_ORDER == __LITTLE_ENDIAN
    return _gekkota_swap_64(net);
#else
    return net;
#endif /* __BIG_ENDIAN */
}

uint16_t
gekkota_hash_16(const char_t *string)
{
    _gekkota_hash(string, uint16_t, 0xF000);
}

uint32_t
gekkota_hash_32(const char_t *string)
{
    _gekkota_hash(string, uint32_t, 0xF0000000);
}

int32_t
gekkota_get_last_error()
{
    return errno;
}
