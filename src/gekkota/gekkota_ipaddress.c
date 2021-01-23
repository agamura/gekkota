/******************************************************************************
 * @file    gekkota_ipaddress.c
 * @date    27-Oct-2006
 * @author  <a href="mailto:giuseppe.greco@agamura.com">Giuseppe Greco</a>
 *
 * Copyright (C) 2006 Agamura, Inc. - http://www.agamura.com
 * All right reserved.
 ******************************************************************************/

#include <errno.h>
#include <string.h>
#include "gekkota.h"
#include "gekkota_errors.h"
#include "gekkota_memory.h"
#include "gekkota_ipaddress.h"

#define GEKKOTA_IPADDRESS_IPV6_BLOCKS   8

/*
 * The following macros take IPv4 addresses in network byte order.
 */
#define IN_IS_ADDR_UNSPECIFIED(address) \
    ((address) == 0x00000000)

#define IN_IS_ADDR_LOOPBACK(address) \
    ((address) == 0x0100007F)

#define IN_IS_ADDR_MULTICAST(address) \
    ((((uint32_t) (address)) & 0x000000F0) == 0x000000E0)

#ifndef IN6_ARE_ADDR_EQUAL
#define IN6_ARE_ADDR_EQUAL(address1, address2) \
    (memcmp((void_t *) (address1), (void_t *) (address2), sizeof(struct in6_addr)) == 0)
#endif /* IN6_ARE_ADDR_EQUAL */

const byte_t *gekkota_ipaddress_unspecified = (byte_t *) &in6addr_any;
const byte_t *gekkota_ipaddress_loopback = (byte_t *) &in6addr_loopback;

GekkotaIPAddress *
gekkota_ipaddress_new(const char_t *ipString)
{
    struct in6_addr inAddr;
    GekkotaBuffer address;

    if (ipString == NULL)
        return gekkota_ipaddress_new_1(NULL);

    if (_gekkota_ipaddress_string_to_address(ipString, AF_INET, &inAddr) == 0)
        /*
         * [ipString] is an IPv4 address.
         */
        address.length = sizeof(struct in_addr);
    else if (_gekkota_ipaddress_string_to_address(ipString, AF_INET6, &inAddr) == 0)
        /*
         * [ipString] is an IPv6 address.
         */
        address.length = sizeof(struct in6_addr);
    else
        /*
         * [ipString] is not a numeric IP address.
         */
        return NULL;

    address.data = &inAddr;
    return gekkota_ipaddress_new_2(&address);
}

GekkotaIPAddress *
gekkota_ipaddress_new_0(GekkotaIPAddress *address, bool_t deep)
{
    if (address == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return NULL;
    }

    if (deep)
    {
        GekkotaIPAddress *newAddress;
        
        if ((newAddress = gekkota_memory_alloc(sizeof(GekkotaIPAddress), FALSE)) == NULL)
            return NULL;

        gekkota_ipaddress_copy(newAddress, address);
        newAddress->refCount = 1;

        return newAddress;
    }

    ++address->refCount;
    return address;
}

GekkotaIPAddress *
gekkota_ipaddress_new_1(const GekkotaBuffer *hostByteOrderAddress)
{
    GekkotaIPAddress *newAddress;

    if ((newAddress = gekkota_ipaddress_new_2(hostByteOrderAddress)) == NULL)
        return NULL;

    if (hostByteOrderAddress != NULL)
    {
        if (newAddress->family == AF_INET)
        {
            newAddress->ipAddress32[0] =
                gekkota_host_to_net_32(newAddress->ipAddress32[0]);
        }
        else
        {
            /*
             * Address family is AF_INET6.
             */
            register int i;
            for (i = 0; i < GEKKOTA_IPADDRESS_IPV6_BLOCKS; i++)
                newAddress->ipAddress16[i] =
                    gekkota_host_to_net_16(newAddress->ipAddress16[i]);
        }
    }

    return newAddress;
}

GekkotaIPAddress *
gekkota_ipaddress_new_2(const GekkotaBuffer *netByteOrderAddress)
{
    GekkotaIPAddress *newAddress;

    if (netByteOrderAddress != NULL)
    {
        if (netByteOrderAddress->data == NULL)
        {
            errno = GEKKOTA_ERROR_NULL_BUFFER;
            return NULL;
        }

        if (netByteOrderAddress->length == 0)
        {
            errno = GEKKOTA_ERROR_ZERO_LENGTH_BUFFER;
            return NULL;
        }
    }

    if (netByteOrderAddress != NULL &&
            netByteOrderAddress->length != sizeof(struct in6_addr) &&
            netByteOrderAddress->length != sizeof(struct in_addr))
    {
        errno = GEKKOTA_ERROR_ARGUMENT_NOT_VALID;
        return NULL;
    }

    if ((newAddress = gekkota_memory_alloc(sizeof(GekkotaIPAddress), TRUE)) == NULL)
        return NULL;

    if (netByteOrderAddress == NULL ||
            netByteOrderAddress->length == sizeof(struct in_addr))
    {
        newAddress->family = AF_INET;
        newAddress->ipAddress32[0] = netByteOrderAddress != NULL
            ? *(uint32_t *) netByteOrderAddress->data
            : INADDR_ANY;
    }
    else
    {
        newAddress->family = AF_INET6;
        memcpy(&newAddress->ipAddress, netByteOrderAddress != NULL
                ? netByteOrderAddress->data
                : gekkota_ipaddress_unspecified, sizeof(struct in6_addr));
    }

    newAddress->refCount = 1;
    return newAddress;
}

int32_t
gekkota_ipaddress_destroy(GekkotaIPAddress *address)
{
    if (address == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return -1;
    }

    if (--address->refCount == 0)
        gekkota_memory_free(address);
    else
        return address->refCount;

    return 0;
}

int32_t
gekkota_ipaddress_copy(
        GekkotaIPAddress *address,
        const GekkotaIPAddress *value)
{
    if (address == NULL || value == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return -1;
    }

    if (address != value)
    {
        memcpy(address, value, sizeof(value->ipAddress));
        address->family = value->family;
        address->scopeID = value->scopeID;
    }

    return 0;
}

GekkotaAddressFamily
gekkota_ipaddress_get_family(const GekkotaIPAddress *address)
{
    if (address == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return GEKKOTA_ADDRESS_FAMILY_UNDEFINED;
    }

    return address->family == AF_INET
        ? GEKKOTA_ADDRESS_FAMILY_INET
        : GEKKOTA_ADDRESS_FAMILY_INET6;
}

bool_t
gekkota_ipaddress_equals(
        const GekkotaIPAddress *address,
        const GekkotaIPAddress *value)
{
    if (address == value)
        return TRUE;

    if (address == NULL || value == NULL)
        return FALSE;

    return address->family == value->family &&
        address->scopeID == value->scopeID &&
        IN6_ARE_ADDR_EQUAL(
            (const IN6_ADDR *) &address->ipAddress,
            (const IN6_ADDR *) &value->ipAddress);
}

bool_t
gekkota_ipaddress_is_ipv4_compatible(const GekkotaIPAddress *address)
{
    if (address == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return FALSE;
    }

    if (address->family != AF_INET6)
    {
        errno = GEKKOTA_ERROR_OPERATION_NOT_SUPPORTED;
        return FALSE;
    }

    errno = GEKKOTA_ERROR_SUCCESS;
    return IN6_IS_ADDR_V4COMPAT((struct in6_addr *) &address->ipAddress);
}

bool_t
gekkota_ipaddress_is_ipv4_mapped(const GekkotaIPAddress *address)
{
    if (address == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return FALSE;
    }

    if (address->family != AF_INET6)
    {
        errno = GEKKOTA_ERROR_OPERATION_NOT_SUPPORTED;
        return FALSE;
    }

    errno = GEKKOTA_ERROR_SUCCESS;
    return IN6_IS_ADDR_V4MAPPED((struct in6_addr *) &address->ipAddress);
}

bool_t
gekkota_ipaddress_is_broadcast(const GekkotaIPAddress *address)
{
    if (address == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return FALSE;
    }

    if (address->family != AF_INET)
    {
        errno = GEKKOTA_ERROR_OPERATION_NOT_SUPPORTED;
        return FALSE;
    }

    errno = GEKKOTA_ERROR_SUCCESS;
    return address->ipAddress32[0] == INADDR_NONE;
}

bool_t
gekkota_ipaddress_is_multicast(const GekkotaIPAddress *address)
{
    if (address == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return FALSE;
    }

    errno = GEKKOTA_ERROR_SUCCESS;
    return address->family == AF_INET
        ? IN_IS_ADDR_MULTICAST(address->ipAddress32[0])
        : IN6_IS_ADDR_MULTICAST((struct in6_addr *) &address->ipAddress);
}

bool_t
gekkota_ipaddress_is_loopback(const GekkotaIPAddress *address)
{
    if (address == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return FALSE;
    }

    errno = GEKKOTA_ERROR_SUCCESS;
    return address->family == AF_INET
        ? IN_IS_ADDR_LOOPBACK(address->ipAddress32[0])
        : IN6_IS_ADDR_LOOPBACK((struct in6_addr *) &address->ipAddress);
}

bool_t
gekkota_ipaddress_is_unspecified(const GekkotaIPAddress *address)
{
    if (address == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return FALSE;
    }

    errno = GEKKOTA_ERROR_SUCCESS;
    return address->family == AF_INET
        ? IN_IS_ADDR_UNSPECIFIED(address->ipAddress32[0])
        : IN6_IS_ADDR_UNSPECIFIED((struct in6_addr *) &address->ipAddress);
}

byte_t *
gekkota_ipaddress_get_bytes(
        const GekkotaIPAddress *address,
        size_t *restrict addressLength)
{
    if (address == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return NULL;
    }

    if (addressLength != NULL)
        *addressLength = address->family == AF_INET
            ? sizeof(struct in_addr)
            : sizeof(struct in6_addr);

    return (byte_t *) &address->ipAddress;
}

uint32_t
gekkota_ipaddress_get_scope_id(const GekkotaIPAddress *address)
{
    if (address == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return 0;
    }

    if (address->family != AF_INET6)
    {
        errno = GEKKOTA_ERROR_OPERATION_NOT_SUPPORTED;
        return 0;
    }

    errno = GEKKOTA_ERROR_SUCCESS;
    return gekkota_net_to_host_32(address->scopeID);
}

int32_t
gekkota_ipaddress_set_scope_id(
        GekkotaIPAddress *restrict address,
        uint32_t scopeID)
{
    if (address == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return -1;
    }

    if (address->family != AF_INET6)
    {
        errno = GEKKOTA_ERROR_OPERATION_NOT_SUPPORTED;
        return -1;
    }

    address->scopeID = gekkota_host_to_net_32(scopeID);
    return 0;
}

int32_t
gekkota_ipaddress_to_string(
        const GekkotaIPAddress *address,
        GekkotaBuffer *ipString)
{
    int32_t ipStringLength;

    if (address == NULL || ipString == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return -1;
    }

    if (ipString->data == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_BUFFER;
        return -1;
    }

    if (ipString->length == 0)
    {
        errno = GEKKOTA_ERROR_ZERO_LENGTH_BUFFER;
        return -1;
    }

    ipStringLength = address->family == AF_INET
        ? INET_ADDRSTRLEN
        : INET6_ADDRSTRLEN;

    if (ipString->length < (size_t) ipStringLength)
    {
        errno = GEKKOTA_ERROR_BUFFER_OVERFLOW;
        return -1;
    }

    if (_gekkota_ipaddress_address_to_string(
            &address->ipAddress, address->family, ipString) != 0)
        return -1;

    return ipStringLength;
}
