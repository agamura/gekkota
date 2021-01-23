/******************************************************************************
 * @file    gekkota_iphostentry_win32.c
 * @date    09-Jan-2007
 * @author  <a href="mailto:giuseppe.greco@agamura.com">Giuseppe Greco</a>
 *
 * Copyright (C) 2007 Agamura, Inc. - http://www.agamura.com
 * All right reserved.
 ******************************************************************************/

#include <errno.h>
#include <string.h>
#include "gekkota.h"
#include "gekkota_bit.h"
#include "gekkota_errors.h"
#include "gekkota_memory.h"
#include "gekkota_idn.h"
#include "gekkota_iphostentry.h"
#include "gekkota_platform.h"

GekkotaIPHostEntry *
_gekkota_iphostentry_new(const char_t *hostNameOrAddress)
{
    GekkotaIPHostEntry *hostEntry = NULL;
    struct sockaddr_in6 sin;
    size_t sinLength;
    ADDRINFO hints;
    ADDRINFO *res = NULL, *resIterator;
    char_t *hostName = NULL;
    size_t memSize, hostNameLength = 0, addressCount = 0, aliasCount = 0;
    int32_t rc;

    memset(&hints, 0x00, sizeof(ADDRINFO));
    gekkota_bit_set(hints.ai_flags, AI_CANONNAME);
    sinLength = sizeof(struct sockaddr_in6);

    if (WSAStringToAddressA(
            (char_t *) hostNameOrAddress, AF_INET, NULL,
            (LPSOCKADDR) &sin, (LPINT) &sinLength) == 0)
    {
        /*
         * [hostNameOrAddress] is an IPv4 numeric address.
         */
        hints.ai_family = AF_INET;
        gekkota_bit_set(hints.ai_flags, AI_NUMERICHOST);
    }
    else if (WSAStringToAddressA(
            (char_t *) hostNameOrAddress, AF_INET6, NULL,
            (LPSOCKADDR) &sin, (LPINT) &sinLength) == 0)
    {
        /*
         * [hostNameOrAddress] is an IPv6 numeric address.
         */
        hints.ai_family = AF_INET6;
        gekkota_bit_set(hints.ai_flags, AI_NUMERICHOST);
    }
    else if (WSAGetLastError() == WSA_NOT_ENOUGH_MEMORY)
    {
        errno = GEKKOTA_ERROR_OUT_OF_MEMORY;
        goto _gekkota_iphostentry_new_exit;
    }
    else if (WSAGetLastError() == WSA_NOT_ENOUGH_MEMORY)
    {
        errno = GEKKOTA_ERROR_OUT_OF_MEMORY;
        goto _gekkota_iphostentry_new_exit;
    }
    else
    {
        /*
         * [hostNameOrAddress] is a canonical host name.
         */
        hints.ai_family = AF_UNSPEC;

        if (gekkota_platform_supports_feature(GEKKOTA_PLATFORM_FEATURE_IDN))
        {
            /*
             * Convert the host name to punycode.
             */
            if (gekkota_idn_to_punycode(hostNameOrAddress, &hostName, NULL) != 0)
                return NULL;

            hostNameOrAddress = hostName;
        }
    }

    rc = getaddrinfo(hostNameOrAddress, NULL, &hints, &res);

    if (gekkota_platform_supports_feature(GEKKOTA_PLATFORM_FEATURE_IDN)
            && hostName != NULL)
    {
        /*
         * The host name encoded as punycode is no longer needed.
         */
        gekkota_memory_free(hostName);
        hostName = NULL;
    }

    if (rc != 0)
    {
        switch (rc)
        {
            case WSATRY_AGAIN:
                errno = GEKKOTA_ERROR_TRY_AGAIN;
                break;
            case WSANO_RECOVERY:
                errno = GEKKOTA_ERROR_CANNOT_RESOLVE_HOSTNAME;
                break;
            case WSA_NOT_ENOUGH_MEMORY:
                errno = GEKKOTA_ERROR_OUT_OF_MEMORY;
                break;
            default:
                errno = GEKKOTA_ERROR_NETWORK_FAILURE;
                break;
        }

        return NULL;
    }

    if (res->ai_canonname != NULL)
    {
        if (gekkota_platform_supports_feature(GEKKOTA_PLATFORM_FEATURE_IDN))
        {
            /*
             * The first node points to the official host name.
             */
            if (gekkota_idn_to_utf8(res->ai_canonname, &hostName, &hostNameLength) != 0)
                goto _gekkota_iphostentry_new_exit;

            /*
             * Subtract the null terminator.
             */
            hostNameLength -= sizeof(char_t);
        }
        else
        {
            hostName = res->ai_canonname;
            hostNameLength = strlen(hostName);
        }
    }

    /*
     * Calculate the address count.
     */
    for (resIterator = res; resIterator != NULL; resIterator = resIterator->ai_next)
        if (resIterator->ai_family == AF_INET6 || resIterator->ai_family == AF_INET)
            addressCount++;

    /*
     * Calculate the alias count.
     */
    /* --> add code here <-- */

    /*
     * Calculate the required memory space.
     */
    memSize = sizeof(GekkotaIPHostEntry)
        + hostNameLength                                /* host name, if any */
        + (addressCount * sizeof(GekkotaIPHostAddress)) /* GekkotaIPHostAddress array */
        + (aliasCount * sizeof(GekkotaIPHostAlias));    /* GekkotaIPhostAlias array */

    if ((hostEntry = gekkota_memory_alloc(memSize, FALSE)) != NULL)
    {
        GekkotaBuffer address;

        /*
         * Calculate the address of the first GekkotaIPHostAlias structure.
         */
        GekkotaIPHostAlias *hostAlias = (GekkotaIPHostAlias *)
            ((byte_t *) hostEntry + memSize - (aliasCount * sizeof(GekkotaIPHostAlias)));

        /*
         * Calculate the address of the first GekkotaIPHostAddress structure.
         */
        GekkotaIPHostAddress *hostAddress = (GekkotaIPHostAddress *)
            ((byte_t *) hostAlias - (addressCount * sizeof(GekkotaIPHostAddress)));

        /*
         * Initialize linked lists.
         */
        gekkota_list_clear(&hostEntry->hostAddresses);
        gekkota_list_clear(&hostEntry->hostAliases);

        if (hostName != NULL)
            strcpy_s(hostEntry->hostName, hostNameLength + 1, hostName);
        else
            hostEntry->hostName[0] = '\0';

        /*
         * Populate the address list.
         */
        for (resIterator = res; resIterator != NULL; resIterator = resIterator->ai_next)
        {
            if (resIterator->ai_family != AF_INET6 && resIterator->ai_family != AF_INET)
                continue;

            if (resIterator->ai_family == AF_INET6)
            {
                address.data = &((struct sockaddr_in6 *) resIterator->ai_addr)->sin6_addr;
                address.length = sizeof(struct in6_addr);
            }
            else
            {
                address.data = &((struct sockaddr_in *) resIterator->ai_addr)->sin_addr;
                address.length = sizeof(struct in_addr);
            }

            if ((hostAddress->address = gekkota_ipaddress_new_2(&address)) == NULL)
                goto _gekkota_iphostentry_new_undo;

            gekkota_list_add(&hostEntry->hostAddresses, hostAddress);
            hostAddress++;
        }

        /*
         * Populate the alias list.
         */
        /* --> add code here <-- */

        hostEntry->refCount = 1;
    }

    goto _gekkota_iphostentry_new_exit;

_gekkota_iphostentry_new_undo:
    gekkota_iphostentry_destroy(hostEntry);
    hostEntry = NULL;

_gekkota_iphostentry_new_exit:
    /*
     * If [platform.hasIdnSupport] is FALSE and hostName is not NULL, then
     * it means [hostName] is just an alias for [res->ai_canonname] and
     * therefore it will be released by freeaddrinfo().
     */
    if (gekkota_platform_supports_feature(GEKKOTA_PLATFORM_FEATURE_IDN)
            && hostName != NULL)
        gekkota_memory_free(hostName);

    freeaddrinfo(res);
    return hostEntry;
}
