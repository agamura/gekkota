/******************************************************************************
 * @file    gekkota_iphostentry_unix.c
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
#include "gekkota_hooks.h"
#include "gekkota_iphostentry.h"

#ifndef HAVE_NATIVE_IDN
#include "gekkota_idn.h"
#include "platform.h"
#endif /* !HAVE_NATIVE_IDN */

GekkotaIPHostEntry *
_gekkota_iphostentry_new(const char_t *hostNameOrAddress)
{
    GekkotaIPHostEntry *hostEntry = NULL;
    struct in6_addr sin_addr;
    struct addrinfo hints, *res = NULL, *resIterator;
#ifndef HAVE_NATIVE_IDN
    char_t *hostName = NULL;
    size_t hostNameLength = 0;
#endif /* !HAVE_NATIVE_IDN */
    size_t memSize, addressCount = 0, aliasCount = 0;
    int32_t rc;

    memset(&hints, 0x00, sizeof(struct addrinfo));
    gekkota_bit_set(hints.ai_flags, AI_CANONNAME);
#ifdef HAVE_NATIVE_IDN
    gekkota_bit_set(hints.ai_flags, AI_CANONIDN);
#endif /* HAVE_NATIVE_IDN */

    if (inet_pton(AF_INET, hostNameOrAddress, &sin_addr) == 1)
    {
        /*
         * [hostNameOrAddress] is an IPv4 numeric address.
         */
        hints.ai_family = AF_INET;
        gekkota_bit_set(hints.ai_flags, AI_NUMERICHOST);
    }
    else if (inet_pton(AF_INET6, hostNameOrAddress, &sin_addr) == 1)
    {
        /*
         * [hostNameOrAddress] is an IPv6 numeric address.
         */
        hints.ai_family = AF_INET6;
        gekkota_bit_set(hints.ai_flags, AI_NUMERICHOST);
    }
    else
    {
        /*
         * [hostNameOrAddress] is a canonical host name.
         */
        hints.ai_family = AF_UNSPEC;

#ifdef HAVE_NATIVE_IDN
        gekkota_bit_set(hints.ai_flags, AI_IDN);
#else
        if (platform.hasIdnSupport)
        {
            /*
             * Convert the host name to punycode.
             */
            if (gekkota_idn_to_punycode(hostNameOrAddress, &hostName, NULL) != 0)
                return NULL;

            hostNameOrAddress = hostName;
        }
#endif /* HAVE_NATIVE_IDN */
    }

    rc = getaddrinfo(hostNameOrAddress, NULL, &hints, &res);

#ifndef HAVE_NATIVE_IDN
    if (hostName != NULL)
    {
        /*
         * The host name encoded as punycode is no longer needed.
         *
         * Since the underlying Libidn API allocates the required buffer space
         * with standard malloc(), [hostName] must be released with free().
         */
        free(hostName);
        hostName = NULL;
    }
#endif /* !HAVE_NATIVE_IDN */

    if (rc != 0)
    {
        switch (rc)
        {
            case EAI_AGAIN:
                errno = GEKKOTA_ERROR_TRY_AGAIN;
                break;
            case EAI_FAIL:
                errno = GEKKOTA_ERROR_CANNOT_RESOLVE_HOSTNAME;
                break;
            case EAI_MEMORY:
                errno = GEKKOTA_ERROR_OUT_OF_MEMORY;
                break;
            default:
                errno = GEKKOTA_ERROR_NETWORK_FAILURE;
                break;
        }

        return NULL;
    }

#ifndef HAVE_NATIVE_IDN
    if (res->ai_canonname != NULL)
    {
        if (platform.hasIdnSupport)
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
#endif /* !HAVE_NATIVE_IDN */

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
#ifdef HAVE_NATIVE_IDN
        + (res->ai_canonname != NULL ? strlen(res->ai_canonname) : 0)
#else
        + hostNameLength
#endif /* HAVE_NATIVE_IDN */
        + (addressCount * sizeof(GekkotaIPHostAddress))
        + (aliasCount * sizeof(GekkotaIPHostAlias));

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

#ifdef HAVE_NATIVE_IDN
        if (res->ai_canonname != NULL)
            strcpy(hostEntry->hostName, res->ai_canonname);
#else
        if (hostName != NULL)
            strcpy(hostEntry->hostName, hostName);
#endif /* HAVE_NATIVE_IDN */
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

            hostAddress->address = gekkota_ipaddress_new_2(&address);

            if (hostAddress->address == NULL)
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
#ifndef HAVE_NATIVE_IDN
    /*
     * If [platform.hasIdnSupport] is FALSE and hostName is not NULL, then
     * it means [hostName] is just an alias for [res->ai_canonname] and
     * therefore it will be released by freeaddrinfo().
     */
    if (platform.hasIdnSupport && hostName != NULL)
        /*
         * Since the underlying Libidn API allocates the required buffer space
         * with the standard malloc(), [hostName] must be released with free().
         */
        free(hostName);
#endif /* !HAVE_NATIVE_IDN */

    freeaddrinfo(res);
    return hostEntry;
}
