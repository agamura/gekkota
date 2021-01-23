/******************************************************************************
 * @file    gekkota_socket.c
 * @date    19-Jul-2007
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
#include "gekkota_memory.h"
#include "gekkota_list.h"
#include "gekkota_networkinterface.h"
#include "gekkota_socket.h"

#define GEKKOTA_SOCKET_DEFAULT_RECEIVE_BUFFER_SIZE  256 * 1024
#define GEKKOTA_SOCKET_DEFAULT_SEND_BUFFER_SIZE     256 * 1024

static GekkotaSocket *
_gekkota_socket_new(
        GekkotaSocketType socketType,
        const GekkotaSocketAddress *localSocketAddress);

static int32_t
_gekkota_socket_transcode_option_level_and_name(
        GekkotaSocketOptionLevel *restrict level,
        GekkotaSocketOptionName *restrict name);

GekkotaSocket *
gekkota_socket_new_0(GekkotaSocket *restrict socket)
{
    if (socket == NULL)
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
    else
        ++socket->refCount;

    return socket;
}

GekkotaSocket *
gekkota_socket_new_2(
        GekkotaSocketType socketType,
        GekkotaAddressFamily family,
        uint16_t port)
{
    GekkotaSocket *newSocket;
    struct sockaddr_in6 sockaddr;

    if (!gekkota_is_initialized())
    {
        errno = GEKKOTA_ERROR_LIB_NOT_INITIALIZED;
        return NULL;
    }

    switch (socketType)
    {
        case GEKKOTA_SOCKET_TYPE_STREAM:
            socketType = SOCK_STREAM;
            break;

        case GEKKOTA_SOCKET_TYPE_DATAGRAM:
            socketType = SOCK_DGRAM;
            break;

        default:
            errno = GEKKOTA_ERROR_SOCKET_TYPE_NOT_SUPPORTED;
            return NULL;
    }

    switch (family)
    {
        case GEKKOTA_ADDRESS_FAMILY_INET:
            family = PF_INET;
            break;

        case GEKKOTA_ADDRESS_FAMILY_INET6:
            family = PF_INET6;
            break;

        default:
            errno = GEKKOTA_ERROR_ADDRESS_FAMILY_NOT_SUPPORTED;
            return NULL;
    }

    memset(&sockaddr, 0x00, sizeof(struct sockaddr_in6));
    sockaddr.sin6_family = family;
    sockaddr.sin6_port = gekkota_host_to_net_16(port);

    if ((newSocket = _gekkota_socket_new(
            socketType, (GekkotaSocketAddress *) &sockaddr)) == NULL)
        return NULL;

    if (socketType == SOCK_DGRAM)
    {
        if ((newSocket->localEndPoint = gekkota_ipendpoint_new_1(family, port)) == NULL)
        {
            gekkota_socket_destroy(newSocket);
            return NULL;
        }
    }

    return newSocket;
}

GekkotaSocket *
gekkota_socket_new_3(
        GekkotaSocketType socketType,
        GekkotaIPEndPoint *localEndPoint)
{
    GekkotaSocket *newSocket;
    struct sockaddr_in6 sockaddr;

    if (!gekkota_is_initialized())
    {
        errno = GEKKOTA_ERROR_LIB_NOT_INITIALIZED;
        return NULL;
    }

    switch (socketType)
    {
        case GEKKOTA_SOCKET_TYPE_STREAM:
            socketType = SOCK_STREAM;
            break;

        case GEKKOTA_SOCKET_TYPE_DATAGRAM:
            socketType = SOCK_DGRAM;
            break;

        default:
            errno = GEKKOTA_ERROR_SOCKET_TYPE_NOT_SUPPORTED;
            return NULL;
    }

    if (localEndPoint != NULL)
        gekkota_ipendpoint_to_socketaddress(
                localEndPoint, (GekkotaSocketAddress *) &sockaddr);
    else
    {
        memset(&sockaddr, 0x00, sizeof(struct sockaddr_in6));
        sockaddr.sin6_family = PF_INET;
    }

    if ((newSocket = _gekkota_socket_new(
            socketType, (GekkotaSocketAddress *) &sockaddr)) == NULL)
        return NULL;

    if (socketType == SOCK_DGRAM)
    {
        newSocket->localEndPoint = localEndPoint != NULL
            ? gekkota_ipendpoint_new_0(localEndPoint, FALSE)
            : gekkota_ipendpoint_new_1(PF_INET, 0);
    }

    return newSocket;
}

int32_t
gekkota_socket_destroy(GekkotaSocket *socket)
{
    if (socket == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return -1;
    }

    if (--socket->refCount == 0)
    {
        _gekkota_socket_close(socket->client);
        gekkota_ipendpoint_destroy(socket->localEndPoint);
        gekkota_ipendpoint_destroy(socket->remoteEndPoint);
        gekkota_memory_free(socket);
    }
    else
        return socket->refCount;

    return 0;
}

int32_t
gekkota_socket_get_option(
        GekkotaSocket *restrict socket,
        GekkotaSocketOptionLevel level,
        GekkotaSocketOptionName name,
        GekkotaBuffer *value)
{
    if (socket == NULL || value == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return -1;
    }

    if (value->data == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_BUFFER;
        return -1;
    }

    if (value->length == 0)
    {
        errno = GEKKOTA_ERROR_ZERO_LENGTH_BUFFER;
        return -1;
    }

    if (_gekkota_socket_transcode_option_level_and_name(&level, &name) < 0)
    {
        errno = GEKKOTA_ERROR_ARGUMENT_NOT_VALID;
        return -1;
    }

    return _gekkota_socket_getsockopt(
            socket->client, level, name, value->data, &value->length);
}

int32_t
gekkota_socket_set_option(
        GekkotaSocket *restrict socket,
        GekkotaSocketOptionLevel level,
        GekkotaSocketOptionName name,
        const GekkotaBuffer *value)
{
    if (socket == NULL || value == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return -1;
    }

    if (value->data == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_BUFFER;
        return -1;
    }

    if (value->length == 0)
    {
        errno = GEKKOTA_ERROR_ZERO_LENGTH_BUFFER;
        return -1;
    }

    if (_gekkota_socket_transcode_option_level_and_name(&level, &name) < 0)
    {
        errno = GEKKOTA_ERROR_ARGUMENT_NOT_VALID;
        return -1;
    }

    return _gekkota_socket_setsockopt(
            socket->client, level, name, value->data, value->length); 
}

GekkotaSocketType
gekkota_socket_get_type(const GekkotaSocket *socket)
{
    if (socket == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return GEKKOTA_SOCKET_TYPE_UNDEFINED;
    }

    return socket->type == SOCK_DGRAM
        ? GEKKOTA_SOCKET_TYPE_DATAGRAM
        : GEKKOTA_SOCKET_TYPE_STREAM;
}

GekkotaSocket *
gekkota_socket_accept(const GekkotaSocket *socket)
{
    GekkotaSocket *newSocket;
    socket_t client;

    if (socket == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return NULL;
    }

    if ((client = _gekkota_socket_accept(socket->client, NULL)) != 0)
        return NULL;

    if ((newSocket = gekkota_memory_alloc(sizeof(GekkotaSocket), TRUE)) == NULL)
    {
        _gekkota_socket_close(client);
        return NULL;
    }

    newSocket->client = client;
    newSocket->type = socket->type;
    newSocket->refCount = 1;

    return newSocket;
} 

int32_t
gekkota_socket_connect(
        GekkotaSocket *restrict socket,
        const GekkotaIPEndPoint *remoteEndPoint)
{
    GekkotaSocketAddress socketAddress;

    if (socket == NULL || remoteEndPoint == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return -1;
    }

    gekkota_ipendpoint_to_socketaddress(remoteEndPoint, &socketAddress);
    
    if (_gekkota_socket_connect(socket->client, &socketAddress) != 0)
        return -1;

    /*
     * Any try to connect an already connected connection-oriented socket would
     * fail, so if execution ends here it means the socket is either not
     * connected or connectionless; in the latter case it is necessary to reset
     * any previous remote endpoint.
     */

    gekkota_ipendpoint_destroy(socket->remoteEndPoint);
    socket->remoteEndPoint = NULL;

    return 0;
}

int32_t
gekkota_socket_connect_1(
        GekkotaSocket *restrict socket,
        const char_t *remoteHostNameOrAddress,
        uint16_t port)
{
    int32_t rc = -1;
    GekkotaIPHostEntry *hostEntry;

    if (socket == NULL || remoteHostNameOrAddress == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return -1;
    }

    if ((hostEntry = gekkota_dns_get_host_entry(remoteHostNameOrAddress)) != NULL)
    {
        struct sockaddr_in6 sockaddr;
        GekkotaList *hostAddresses;
        GekkotaListIterator iterator;
    
        memset(&sockaddr, 0x00, sizeof(struct sockaddr_in6));
        hostAddresses = gekkota_iphostentry_get_addresses(hostEntry);

        for (iterator = gekkota_list_first(hostAddresses);
                iterator != gekkota_list_tail(hostAddresses);
                iterator = gekkota_list_next(iterator))
        {
            sockaddr.sin6_family = gekkota_iphostentry_address(
                    (GekkotaIPHostAddress *) iterator)->family;
            sockaddr.sin6_port = gekkota_host_to_net_16(port);

            if ((rc =_gekkota_socket_connect(
                    socket->client,
                    (GekkotaSocketAddress *) &sockaddr)) == 0)
            {
                /*
                 * See comment in function [gekkota_socket_connect].
                 */
                gekkota_ipendpoint_destroy(socket->remoteEndPoint);
                socket->remoteEndPoint = NULL;
                break;
            }
        }

        gekkota_iphostentry_destroy(hostEntry);
    }

    return rc;
}

GekkotaIPEndPoint *
gekkota_socket_get_local_endpoint(GekkotaSocket *restrict socket)
{
    if (socket == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return NULL;
    }

    if (socket->localEndPoint == NULL)
    {
        GekkotaSocketAddress socketAddress;

        if (_gekkota_socket_getsockname(socket->client, &socketAddress) == 0)
            socket->localEndPoint = gekkota_ipendpoint_new_2(&socketAddress);
    }

    return socket->localEndPoint;
}

GekkotaIPEndPoint *
gekkota_socket_get_remote_endpoint(GekkotaSocket *restrict socket)
{
    if (socket == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return NULL;
    }

    if (socket->remoteEndPoint == NULL)
    {
        GekkotaSocketAddress socketAddress;

        if (_gekkota_socket_getpeername(socket->client, &socketAddress) == 0)
            socket->remoteEndPoint = gekkota_ipendpoint_new_2(&socketAddress);
    }

    return socket->remoteEndPoint;
}

int32_t
gekkota_socket_join_multicast_group_1(
        const GekkotaSocket *socket,
        const GekkotaIPAddress *multicastAddress,
        uint32_t multicastInterfaceIndex)
{
    byte_t *address;
    size_t addressLength;

    if (socket == NULL || multicastAddress == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return -1;
    }

    if (!gekkota_ipaddress_is_multicast(multicastAddress) ||
            (multicastAddress->family == AF_INET &&
            multicastInterfaceIndex > GEKKOTA_NETWORK_INTERFACE_MAX_INDEX))
    {
        errno = GEKKOTA_ERROR_ARGUMENT_NOT_VALID;
        return -1;
    }

    address = gekkota_ipaddress_get_bytes(multicastAddress, &addressLength);

    if (multicastAddress->family == AF_INET)
    {
        struct ip_mreq mreq;
        mreq.imr_multiaddr.s_addr = *((uint32_t *) address);
        mreq.imr_interface.s_addr = gekkota_host_to_net_32(multicastInterfaceIndex);

        return _gekkota_socket_setsockopt(
                socket->client, IPPROTO_IP, IP_ADD_MEMBERSHIP,
                (const void_t *) &mreq, sizeof(struct ip_mreq));
    }
    else
    {
        struct ipv6_mreq mreq;
        memcpy(&mreq.ipv6mr_multiaddr, address, addressLength);
        mreq.ipv6mr_interface = multicastInterfaceIndex;

        return _gekkota_socket_setsockopt(
                socket->client, IPPROTO_IPV6, IPV6_JOIN_GROUP,
                (const void_t *) &mreq, sizeof(struct ipv6_mreq));
    }

    /*
     * Execution never ends here.
     */
}

int32_t
gekkota_socket_join_multicast_group_2(
        const GekkotaSocket *socket,
        const GekkotaIPAddress *multicastAddress,
        uint32_t multicastInterfaceIndex,
        GekkotaMulticastTTL ttl)
{
    if (ttl < GEKKOTA_MULTICAST_TTL_HOST ||
            ttl > GEKKOTA_MULTICAST_TTL_UNRESTRICTED)
    {
        errno = GEKKOTA_ERROR_ARGUMENT_NOT_VALID;
        return -1;
    }

    if (gekkota_socket_join_multicast_group_1(
            socket, multicastAddress, multicastInterfaceIndex) != 0)
        return -1;

    if (multicastAddress->family == AF_INET)
        return _gekkota_socket_setsockopt(
                socket->client, IPPROTO_IP, IP_MULTICAST_TTL,
                (const void_t *) &((uint8_t) ttl), sizeof(uint8_t));
    else
        return _gekkota_socket_setsockopt(
                socket->client, IPPROTO_IPV6, IPV6_MULTICAST_HOPS,
                (const void_t *) &ttl, sizeof(int32_t));

    /*
     * Execution never ends here.
     */
}

int32_t
gekkota_socket_leave_multicast_group_1(
        const GekkotaSocket *socket,
        const GekkotaIPAddress *multicastAddress,
        uint32_t multicastInterfaceIndex)
{
    byte_t *address;
    size_t addressLength;

    if (socket == NULL || multicastAddress == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return -1;
    }

    if (!gekkota_ipaddress_is_multicast(multicastAddress) ||
            (multicastAddress->family == AF_INET &&
            multicastInterfaceIndex > GEKKOTA_NETWORK_INTERFACE_MAX_INDEX))
    {
        errno = GEKKOTA_ERROR_ARGUMENT_NOT_VALID;
        return -1;
    }

    address = gekkota_ipaddress_get_bytes(multicastAddress, &addressLength);

    if (multicastAddress->family == AF_INET)
    {
        struct ip_mreq mreq;
        mreq.imr_multiaddr.s_addr = *((uint32_t *) address);
        mreq.imr_interface.s_addr = gekkota_host_to_net_32(multicastInterfaceIndex);

        return _gekkota_socket_setsockopt(
                socket->client, IPPROTO_IP, IP_DROP_MEMBERSHIP,
                (const void_t *) &mreq, sizeof(struct ip_mreq));
    }
    else
    {
        struct ipv6_mreq mreq;
        memcpy(&mreq.ipv6mr_multiaddr, address, addressLength);
        mreq.ipv6mr_interface = multicastInterfaceIndex;

        return _gekkota_socket_setsockopt(
                socket->client, IPPROTO_IPV6, IPV6_LEAVE_GROUP,
                (const void_t *) &mreq, sizeof(struct ipv6_mreq));
    }

    /*
     * Execution never ends here.
     */
}

int32_t
gekkota_socket_poll(
        const GekkotaSocket *socket,
        GekkotaSelectMode selectMode,
        int32_t timeout)
{
    if (socket == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return -1;
    }

    if (selectMode != GEKKOTA_SELECT_MODE_READ &&
        selectMode != GEKKOTA_SELECT_MODE_WRITE &&
        selectMode != GEKKOTA_SELECT_MODE_ERROR)
    {
        errno = GEKKOTA_ERROR_ARGUMENT_NOT_VALID;
        return -1;
    }

    return _gekkota_socket_poll(socket->client, selectMode, timeout);
}

int32_t
gekkota_socket_send(
        const GekkotaSocket *socket,
        const GekkotaIPEndPoint *remoteEndPoint,
        const GekkotaBuffer *buffers,
        size_t bufferCount)
{
    if (socket == NULL || buffers == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return -1;
    }

    /*
    if (!gekkota_socket_is_connected(socket) && remoteEndPoint == NULL)
    {
        errno = GEKKOTA_ERROR_SOCKET_NOT_CONNECTED;
        return -1;
    }

    if (gekkota_socket_is_connected(socket) && remoteEndPoint != NULL)
    {
        errno = GEKKOTA_ERROR_SOCKET_ALREADY_CONNECTED;
        return -1;
    }
    */

    if (bufferCount > 0)
    {
        GekkotaSocketAddress socketAddress;

        if (remoteEndPoint != NULL)
            gekkota_ipendpoint_to_socketaddress(remoteEndPoint, &socketAddress);

        return _gekkota_socket_send(
                socket->client,
                remoteEndPoint != NULL ? &socketAddress : NULL,
                buffers, bufferCount);
    }

    return 0;
}

int32_t
gekkota_socket_receive(
        const GekkotaSocket *socket,
        GekkotaBuffer *buffers,
        size_t bufferCount,
        GekkotaIPEndPoint **remoteEndPoint)
{
    int32_t recv = 0;

    if (socket == NULL || buffers == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return -1;
    }

    if (bufferCount > 0)
    {
        GekkotaSocketAddress socketAddress;

        if ((recv = _gekkota_socket_receive(
                socket->client,
                buffers, bufferCount,
                remoteEndPoint != NULL ? &socketAddress : NULL)) == -1)
            return -1;

        if (recv > 0 && remoteEndPoint != NULL)
            *remoteEndPoint = gekkota_ipendpoint_new_2(&socketAddress);

            /*
             * Since so far the messages have been successfully received,
             * the function returns the number of bytes received even if
             * [gekkota_ipendpoint_new_2] fails; it is up to the caller to
             * check whether or not [*remoteEndPoint] is NULL and in case
             * look at [errno] for error information
             */
    }

    return recv;
}

static GekkotaSocket *
_gekkota_socket_new(
        GekkotaSocketType socketType,
        const GekkotaSocketAddress *localSocketAddress)
{
    socket_t client;
    GekkotaSocket *newSocket = NULL;

    if ((client = _gekkota_socket_socket(
            localSocketAddress->family,
            socketType,
            0)) == GEKKOTA_INVALID_SOCKET)
        return NULL;

    if  (socketType == SOCK_DGRAM)
    {
        int32_t receiveBufferSize = GEKKOTA_SOCKET_DEFAULT_RECEIVE_BUFFER_SIZE;
        int32_t sendBufferSize = GEKKOTA_SOCKET_DEFAULT_SEND_BUFFER_SIZE;
        int32_t enable = 1;

        _gekkota_socket_setsockopt(client, SOL_SOCKET, SO_RCVBUF,
            (const void_t *) &receiveBufferSize, sizeof(int32_t));
        _gekkota_socket_setsockopt(client, SOL_SOCKET, SO_SNDBUF,
            (const void_t *) &sendBufferSize, sizeof(int32_t));
        _gekkota_socket_setsockopt(client, SOL_SOCKET, SO_REUSEADDR,
            (const void_t *) &enable, sizeof(int32_t));
    }

    if (_gekkota_socket_bind(client, localSocketAddress) == -1 ||
            (socketType == SOCK_STREAM && _gekkota_socket_listen(client) == -1))
        return NULL;

    if ((newSocket = gekkota_memory_alloc(sizeof(GekkotaSocket), TRUE)) == NULL)
        return NULL;

    newSocket->client = client;
    newSocket->type = socketType;
    newSocket->refCount = 1;

    return newSocket;
}

static int32_t
_gekkota_socket_transcode_option_level_and_name(
        GekkotaSocketOptionLevel *restrict level,
        GekkotaSocketOptionName *restrict name)
{
    if (*level == GEKKOTA_SOCKET_OPTION_LEVEL_SOCKET)
    {
        *level = SOL_SOCKET;

        switch (*name)
        {
#ifdef SO_ACCEPTCONN
            case GEKKOTA_SOCKET_OPTION_NAME_ACCEPT_CONNECTION:
                *name = SO_ACCEPTCONN;
                break;
#endif /* SO_ACCEPTCONN */

            case GEKKOTA_SOCKET_OPTION_NAME_BROADCAST:
                *name = SO_BROADCAST;
                break;

            case GEKKOTA_SOCKET_OPTION_NAME_DEBUG:
                *name = SO_DEBUG;
                break;

            case GEKKOTA_SOCKET_OPTION_NAME_DONT_ROUTE:
                *name = SO_DONTROUTE;
                break;

            case GEKKOTA_SOCKET_OPTION_NAME_ERROR:
                *name = SO_ERROR;
                break;

            case GEKKOTA_SOCKET_OPTION_NAME_KEEP_ALIVE:
                *name = SO_KEEPALIVE;
                break;

            case GEKKOTA_SOCKET_OPTION_NAME_LINGER:
                *name = SO_LINGER;
                break;

            case GEKKOTA_SOCKET_OPTION_NAME_OUT_OF_BAND_INLINE:
                *name = SO_OOBINLINE;
                break;

            case GEKKOTA_SOCKET_OPTION_NAME_RECEIVE_BUFFER:
                *name = SO_RCVBUF;
                break;

            case GEKKOTA_SOCKET_OPTION_NAME_RECEIVE_LOW_WATER_MARK:
                *name = SO_RCVLOWAT;
                break;

            case GEKKOTA_SOCKET_OPTION_NAME_RECEIVE_TIMEOUT:
                *name = SO_RCVTIMEO;
                break;

            case GEKKOTA_SOCKET_OPTION_NAME_REUSE_ADDRESS:
                *name = SO_REUSEADDR;
                break;

            case GEKKOTA_SOCKET_OPTION_NAME_REUSE_PORT:
#ifdef SO_REUSEPORT
                *name = SO_REUSEPORT;
#else
                *name = SO_REUSEADDR;
#endif /* SO_REUSEPORT */
                break;

            case GEKKOTA_SOCKET_OPTION_NAME_SEND_BUFFER:
                *name = SO_SNDBUF;
                break;

            case GEKKOTA_SOCKET_OPTION_NAME_SEND_LOW_WATER_MARK:
                *name = SO_SNDLOWAT;
                break;

            case GEKKOTA_SOCKET_OPTION_NAME_SEND_TIMEOUT:
                *name = SO_SNDTIMEO;
                break;

            case GEKKOTA_SOCKET_OPTION_NAME_TYPE:
                *name = SO_TYPE;
                break;

            case GEKKOTA_SOCKET_OPTION_NAME_USE_LOOPBACK:
                *name = SO_USELOOPBACK;
                break;

            default:
                return -1;
        }
    }
    else if (*level == GEKKOTA_SOCKET_OPTION_LEVEL_IP)
    {
        *level = IPPROTO_IP;

        switch (*name)
        {
#ifdef IP_HDRINCL
            case GEKKOTA_SOCKET_OPTION_NAME_HEADER_INCLUDED:
                *name = IP_HDRINCL;
                break;
#endif /* IP_HDRINCL */

            case GEKKOTA_SOCKET_OPTION_NAME_IP_OPTIONS:
                *name = IP_OPTIONS;
                break;

            case GEKKOTA_SOCKET_OPTION_NAME_MULTICAST_LOOPBACK:
                *name = IP_MULTICAST_LOOP;
                break;

#ifdef IP_PKTINFO
            case GEKKOTA_SOCKET_OPTION_NAME_PACKET_INFORMATION:
                *name = IP_PKTINFO;
                break;
#endif /* IP_PKTINFO */

            case GEKKOTA_SOCKET_OPTION_NAME_HOP_LIMIT:
            case GEKKOTA_SOCKET_OPTION_NAME_TIME_TO_LIVE:
                *name = IP_TTL;
                break;

            case GEKKOTA_SOCKET_OPTION_NAME_TYPE_OF_SERVICE:
                *name = IP_TOS;
                break;

            default:
                return -1;
        }
    }
    else if (*level == GEKKOTA_SOCKET_OPTION_LEVEL_IPV6)
    {
        *level = IPPROTO_IPV6;

        switch (*name)
        {
            case GEKKOTA_SOCKET_OPTION_NAME_HOP_LIMIT:
            case GEKKOTA_SOCKET_OPTION_NAME_TIME_TO_LIVE:
                *name = IPV6_UNICAST_HOPS;
                break;

            case GEKKOTA_SOCKET_OPTION_NAME_MULTICAST_LOOPBACK:
                *name = IPV6_MULTICAST_LOOP;
                break;

            default:
                return -1;
        }
    }
    else if (*level == GEKKOTA_SOCKET_OPTION_LEVEL_TCP)
    {
        *level = IPPROTO_TCP;

        switch (*name)
        {
            case GEKKOTA_SOCKET_OPTION_NAME_NO_DELAY:
                *name = TCP_NODELAY;
                break;

            default:
                return -1;
        }
    }
    else return -1;

    return 0;
}
