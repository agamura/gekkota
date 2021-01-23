/******************************************************************************
 * @file    gekkota_dns.c
 * @date    09-Jan-2007
 * @author  <a href="mailto:giuseppe.greco@agamura.com">Giuseppe Greco</a>
 *
 * Copyright (C) 2007 Agamura, Inc. - http://www.agamura.com
 * All right reserved.
 ******************************************************************************/

#include <errno.h>
#include <string.h>
#include "gekkota.h"
#include "gekkota_dns.h"
#include "gekkota_errors.h"

GekkotaIPHostEntry *
gekkota_dns_get_host_entry(const char_t *hostNameOrAddress)
{
    if (!gekkota_is_initialized())
    {
        errno = GEKKOTA_ERROR_LIB_NOT_INITIALIZED;
        return NULL;
    }

    if (hostNameOrAddress == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return NULL;
    }

    return _gekkota_iphostentry_new(hostNameOrAddress);
}

GekkotaIPHostEntry *
gekkota_dns_get_host_entry_1(const GekkotaIPAddress *address)
{
    char_t ipString[INET6_ADDRSTRLEN];
    GekkotaBuffer buffer;

    if (!gekkota_is_initialized())
    {
        errno = GEKKOTA_ERROR_LIB_NOT_INITIALIZED;
        return NULL;
    }

    buffer.data = ipString;
    buffer.length = sizeof(ipString);

    if (gekkota_ipaddress_to_string(address, &buffer) != 0)
        return NULL;

    return _gekkota_iphostentry_new(ipString);
}

int32_t
gekkota_dns_get_host_name(GekkotaBuffer *restrict hostName)
{
    if (hostName == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return -1;
    }

    if (hostName->data == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_BUFFER;
        return -1;
    }

    if (gethostname(hostName->data, (int32_t) hostName->length) != 0)
    {
        errno = GEKKOTA_ERROR_BUFFER_OVERFLOW;
        return -1;
    }

    if (strnlen(hostName->data, hostName->length) == hostName->length)
    {
        errno = GEKKOTA_ERROR_BUFFER_OVERFLOW;
        return -1;
    }

    return 0;
}
