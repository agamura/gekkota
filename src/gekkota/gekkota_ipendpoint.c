/******************************************************************************
 * @file    gekkota_ipendpoint.c
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
#include "gekkota_ipendpoint.h"

static inline GekkotaIPEndPoint *
_gekkota_ipendpoint_new(GekkotaIPAddress *address, uint16_t port);

GekkotaIPEndPoint *
gekkota_ipendpoint_new(GekkotaIPAddress *address, uint16_t port)
{
    GekkotaIPEndPoint *endPoint;

    if ((address = address != NULL
            ? gekkota_ipaddress_new_0(address, FALSE)
            : gekkota_ipaddress_new_2(NULL)) == NULL)
        return NULL;

    port = gekkota_host_to_net_16(port);

    if ((endPoint = _gekkota_ipendpoint_new(address, port)) == NULL)
        gekkota_ipaddress_destroy(address);

    return endPoint;
}

GekkotaIPEndPoint *
gekkota_ipendpoint_new_0(GekkotaIPEndPoint *restrict endPoint, bool_t deep)
{
    if (endPoint == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return NULL;
    }

    if (deep)
    {
        GekkotaIPAddress *newAddress;
        GekkotaIPEndPoint *newEndPoint;

        if ((newAddress = gekkota_ipaddress_new_0(endPoint->address, TRUE)) == NULL)
            return NULL;

        if ((newEndPoint = gekkota_memory_alloc(sizeof(GekkotaIPEndPoint), FALSE)) == NULL)
        {
            gekkota_ipaddress_destroy(newAddress);
            return NULL;
        }

        newEndPoint->address = newAddress;
        newEndPoint->port = endPoint->port;
        newEndPoint->refCount = 1;

        return newEndPoint;
    }

    ++endPoint->refCount;
    return endPoint;
}

GekkotaIPEndPoint *
gekkota_ipendpoint_new_1(GekkotaAddressFamily addressFamily, uint16_t port)
{
    GekkotaIPEndPoint *endPoint;
    GekkotaIPAddress *address;
    GekkotaBuffer buffer;

    switch (addressFamily)
    {
        case GEKKOTA_ADDRESS_FAMILY_INET:
            buffer.length = sizeof(struct in_addr);
            break;

        case GEKKOTA_ADDRESS_FAMILY_INET6:
            buffer.length = sizeof(struct in6_addr);
            break;

        default:
            errno = GEKKOTA_ERROR_ADDRESS_FAMILY_NOT_SUPPORTED;
            return NULL;
    }

    buffer.data = (void_t *) gekkota_ipaddress_unspecified;

    if ((address = gekkota_ipaddress_new_2(&buffer)) == NULL)
        return NULL;

    port = gekkota_host_to_net_16(port);

    if ((endPoint = _gekkota_ipendpoint_new(address, port)) == NULL)
        gekkota_ipaddress_destroy(address);

    return endPoint;
}

GekkotaIPEndPoint *
gekkota_ipendpoint_new_2(const GekkotaSocketAddress *socketAddress)
{
    /*
     * Values in [socketAddress] are expected to be already in
     * network byte order.
     */

    GekkotaBuffer buffer;
    GekkotaIPAddress *address;
    GekkotaIPEndPoint *endPoint;

    if (socketAddress == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return NULL;
    }

    switch (socketAddress->family)
    {
        case AF_INET:
            buffer.data = &((struct sockaddr_in *) socketAddress)->sin_addr;
            buffer.length = sizeof(struct in_addr);
            break;

        case AF_INET6:
            buffer.data = &((struct sockaddr_in6 *) socketAddress)->sin6_addr;
            buffer.length = sizeof(struct in6_addr);
            break;

        default:
            errno = GEKKOTA_ERROR_ADDRESS_FAMILY_NOT_SUPPORTED;
            return NULL;
    }

    if ((address = gekkota_ipaddress_new_2(&buffer)) == NULL)
        return NULL;

    if (socketAddress->family == AF_INET6)
        address->scopeID = ((struct sockaddr_in6 *) socketAddress)->sin6_scope_id;

    if ((endPoint = _gekkota_ipendpoint_new(address, socketAddress->port)) == NULL)
    {
        gekkota_ipaddress_destroy(address);
        return NULL;
    }

    return endPoint;
}

int32_t
gekkota_ipendpoint_destroy(GekkotaIPEndPoint *endPoint)
{
    if (endPoint == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return -1;
    }

    if (--endPoint->refCount == 0)
    {
        gekkota_ipaddress_destroy(endPoint->address);
        gekkota_memory_free(endPoint);
    }
    else
        return endPoint->refCount;

    return 0;
}

int32_t
gekkota_ipendpoint_copy(
        GekkotaIPEndPoint *restrict endPoint,
        GekkotaIPEndPoint *value)
{
    if (endPoint == NULL || value == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return -1;
    }

    if (endPoint != value)
    {
        gekkota_ipaddress_copy(endPoint->address, value->address);
        endPoint->port = value->port;
    }

    return 0;
}

bool_t
gekkota_ipendpoint_equals(
        const GekkotaIPEndPoint *endPoint,
        const GekkotaIPEndPoint *value)
{
    if (endPoint == value)
        return TRUE;

    if (endPoint == NULL || value == NULL)
        return FALSE;

    return endPoint->port == value->port &&
        gekkota_ipaddress_equals(endPoint->address, value->address);
}

GekkotaIPAddress *
gekkota_ipendpoint_get_address(const GekkotaIPEndPoint *endPoint)
{
    if (endPoint == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return NULL;
    }

    return endPoint->address;
}

int32_t
gekkota_ipendpoint_set_address(
        GekkotaIPEndPoint *restrict endPoint,
        GekkotaIPAddress *address)
{
    if (endPoint == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return -1;
    }

    if (address != endPoint->address)
    {
        if ((address = address != NULL
                ? gekkota_ipaddress_new_0(address, FALSE)
                : gekkota_ipaddress_new_2(NULL)) == NULL)
            return -1;

        gekkota_ipaddress_destroy(endPoint->address);
        endPoint->address = address;
    }

    return 0;
}

int32_t
gekkota_ipendpoint_get_port(const GekkotaIPEndPoint *endPoint)
{
    if (endPoint == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return -1;
    }

    return gekkota_net_to_host_16(endPoint->port);
}

int32_t
gekkota_ipendpoint_set_port(GekkotaIPEndPoint *restrict endPoint, uint16_t port)
{
    if (endPoint == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return -1;
    }

    endPoint->port = gekkota_host_to_net_16(port);
    return 0;
}

int32_t
gekkota_ipendpoint_to_socketaddress(
        const GekkotaIPEndPoint *endPoint,
        GekkotaSocketAddress *socketAddress)
{
    byte_t* address;
    size_t addressLength;

    if (endPoint == NULL || socketAddress == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return -1;
    }

    socketAddress->family = endPoint->address->family;
    socketAddress->port = endPoint->port;
    address = gekkota_ipaddress_get_bytes(endPoint->address, &addressLength);

    if (socketAddress->family == AF_INET)
    {
        struct sockaddr_in *sockaddr = (struct sockaddr_in *) socketAddress;
        *((uint32_t *) &sockaddr->sin_addr) = *((uint32_t *) address);
        memset(sockaddr->sin_zero, 0x00, sizeof(sockaddr->sin_zero));
    }
    else
    {
        /*
         * Address family is AF_INET6.
         */
        struct sockaddr_in6 *sockaddr = (struct sockaddr_in6 *) socketAddress;
        sockaddr->sin6_flowinfo = 0;
        memcpy(&sockaddr->sin6_addr, address, addressLength);
        sockaddr->sin6_scope_id = endPoint->address->scopeID;
    }

    return 0;
}

static inline GekkotaIPEndPoint *
_gekkota_ipendpoint_new(GekkotaIPAddress *address, uint16_t port)
{
    GekkotaIPEndPoint *endPoint;

    if ((endPoint = gekkota_memory_alloc(sizeof(GekkotaIPEndPoint), FALSE)) == NULL)
        return NULL;

    endPoint->address = address;
    endPoint->port = port;
    endPoint->refCount = 1;

    return endPoint;
}
