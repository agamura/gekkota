/******************************************************************************
 * @file    gekkota_iphostentry.c
 * @date    09-Jan-2007
 * @author  <a href="mailto:giuseppe.greco@agamura.com">Giuseppe Greco</a>
 *
 * Copyright (C) 2007 Agamura, Inc. - http://www.agamura.com
 * All right reserved.
 ******************************************************************************/

#include <errno.h>
#include "gekkota_errors.h"
#include "gekkota_memory.h"
#include "gekkota_iphostentry.h"

GekkotaIPHostEntry *
gekkota_iphostentry_new_0(GekkotaIPHostEntry *restrict hostEntry)
{
    if (hostEntry == NULL)
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
    else
        ++hostEntry->refCount;

    return hostEntry;
}

int32_t
gekkota_iphostentry_destroy(GekkotaIPHostEntry *hostEntry)
{
    if (hostEntry == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return -1;
    }

    if (--hostEntry->refCount == 0)
    {
        GekkotaListIterator iterator;

        for (iterator = gekkota_list_first(&hostEntry->hostAddresses);
                iterator != gekkota_list_tail(&hostEntry->hostAddresses);
                iterator = gekkota_list_next(iterator))
            gekkota_ipaddress_destroy(((GekkotaIPHostAddress *) iterator)->address);

        for (iterator = gekkota_list_first(&hostEntry->hostAliases);
                iterator != gekkota_list_tail(&hostEntry->hostAliases);
                iterator = gekkota_list_next(iterator))
            gekkota_memory_free(((GekkotaIPHostAlias *) iterator)->alias);

        gekkota_memory_free(hostEntry);
    }
    else
        return hostEntry->refCount;

    return 0;
}

char_t *
gekkota_iphostentry_get_host_name(const GekkotaIPHostEntry *hostEntry)
{
    if (hostEntry == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return NULL;
    }

    return (char_t *) hostEntry->hostName;
}

GekkotaList *
gekkota_iphostentry_get_addresses(const GekkotaIPHostEntry *hostEntry)
{
    if (hostEntry == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return NULL;
    }

    return (GekkotaList *) &hostEntry->hostAddresses;
}

GekkotaList *
gekkota_iphostentry_get_aliases(const GekkotaIPHostEntry *hostEntry)
{
    if (hostEntry == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return NULL;
    }

    return (GekkotaList *) &hostEntry->hostAliases;
}

GekkotaIPAddress *
gekkota_iphostentry_address(GekkotaIPHostAddress *restrict hostAddress)
{
    if (hostAddress == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return NULL;
    }

    return hostAddress->address;
}

char_t *
gekkota_iphostentry_alias(GekkotaIPHostAlias *restrict hostAlias) 
{
    if (hostAlias == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return NULL;
    }

    return hostAlias->alias;
}
