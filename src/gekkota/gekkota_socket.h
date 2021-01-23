/******************************************************************************
 * @file    gekkota_socket.h
 * @date    27-Oct-2006
 * @author  <a href="mailto:giuseppe.greco@agamura.com">Giuseppe Greco</a>
 *
 * Copyright (C) 2006 Agamura, Inc. - http://www.agamura.com
 * All right reserved.
 ******************************************************************************/

#ifndef __GEKKOTA_SOCKET_H__
#define __GEKKOTA_SOCKET_H__

#include "gekkota/gekkota_buffer.h"
#include "gekkota/gekkota_ipendpoint.h"
#include "gekkota/gekkota_types.h"

typedef enum
{
    GEKKOTA_SOCKET_TYPE_UNDEFINED = 0,
    GEKKOTA_SOCKET_TYPE_STREAM,
    GEKKOTA_SOCKET_TYPE_DATAGRAM
} GekkotaSocketType;

typedef enum
{
    GEKKOTA_SOCKET_OPTION_LEVEL_UNDEFINED = 0,
    GEKKOTA_SOCKET_OPTION_LEVEL_SOCKET,
    GEKKOTA_SOCKET_OPTION_LEVEL_IP,
    GEKKOTA_SOCKET_OPTION_LEVEL_IPV6,
    GEKKOTA_SOCKET_OPTION_LEVEL_TCP
} GekkotaSocketOptionLevel;

typedef enum
{
    GEKKOTA_SOCKET_OPTION_NAME_UNDEFINED = 0,

    /* Socket Level Options */
    GEKKOTA_SOCKET_OPTION_NAME_ACCEPT_CONNECTION,
    GEKKOTA_SOCKET_OPTION_NAME_BROADCAST,
    GEKKOTA_SOCKET_OPTION_NAME_DEBUG,
    GEKKOTA_SOCKET_OPTION_NAME_DONT_ROUTE,
    GEKKOTA_SOCKET_OPTION_NAME_ERROR,
    GEKKOTA_SOCKET_OPTION_NAME_KEEP_ALIVE,
    GEKKOTA_SOCKET_OPTION_NAME_LINGER,
    GEKKOTA_SOCKET_OPTION_NAME_OUT_OF_BAND_INLINE,
    GEKKOTA_SOCKET_OPTION_NAME_RECEIVE_BUFFER,
    GEKKOTA_SOCKET_OPTION_NAME_RECEIVE_LOW_WATER_MARK,
    GEKKOTA_SOCKET_OPTION_NAME_RECEIVE_TIMEOUT,
    GEKKOTA_SOCKET_OPTION_NAME_REUSE_ADDRESS,
    GEKKOTA_SOCKET_OPTION_NAME_REUSE_PORT,
    GEKKOTA_SOCKET_OPTION_NAME_SEND_BUFFER,
    GEKKOTA_SOCKET_OPTION_NAME_SEND_LOW_WATER_MARK,
    GEKKOTA_SOCKET_OPTION_NAME_SEND_TIMEOUT,
    GEKKOTA_SOCKET_OPTION_NAME_TYPE,
    GEKKOTA_SOCKET_OPTION_NAME_USE_LOOPBACK,

    /* IP Level Options */
    GEKKOTA_SOCKET_OPTION_NAME_HEADER_INCLUDED,
    GEKKOTA_SOCKET_OPTION_NAME_HOP_LIMIT,
    GEKKOTA_SOCKET_OPTION_NAME_IP_OPTIONS,
    GEKKOTA_SOCKET_OPTION_NAME_MULTICAST_LOOPBACK,
    GEKKOTA_SOCKET_OPTION_NAME_PACKET_INFORMATION,
    GEKKOTA_SOCKET_OPTION_NAME_TIME_TO_LIVE,
    GEKKOTA_SOCKET_OPTION_NAME_TYPE_OF_SERVICE,

    /* TCP Level Options */
    GEKKOTA_SOCKET_OPTION_NAME_NO_DELAY
} GekkotaSocketOptionName;

typedef enum
{
    GEKKOTA_SELECT_MODE_UNDEFINED = 0,
    GEKKOTA_SELECT_MODE_READ,
    GEKKOTA_SELECT_MODE_WRITE,
    GEKKOTA_SELECT_MODE_ERROR
} GekkotaSelectMode;

typedef enum
{
    GEKKOTA_MULTICAST_TTL_HOST          = 0x00,
    GEKKOTA_MULTICAST_TTL_SUBNET        = 0x01,
    GEKKOTA_MULTICAST_TTL_SITE          = 0x20,
    GEKKOTA_MULTICAST_TTL_REGION        = 0x40,
    GEKKOTA_MULTICAST_TTL_CONTINENT     = 0x80,
    GEKKOTA_MULTICAST_TTL_UNRESTRICTED  = 0xFF
} GekkotaMulticastTTL;

typedef struct _GekkotaSocket GekkotaSocket;

#define gekkota_socket_new(socketType) \
    (gekkota_socket_new_1(socketType, 0))

GEKKOTA_API GekkotaSocket *
gekkota_socket_new_0(GekkotaSocket *restrict socket);

#define gekkota_socket_new_1(socketType, port) \
    (gekkota_socket_new_2(socketType, GEKKOTA_ADDRESS_FAMILY_INET6, port))

GEKKOTA_API GekkotaSocket *
gekkota_socket_new_2(
        GekkotaSocketType socketType,
        GekkotaAddressFamily family,
        uint16_t port);

GEKKOTA_API GekkotaSocket *
gekkota_socket_new_3(
        GekkotaSocketType socketType,
        GekkotaIPEndPoint *localEndPoint);

GEKKOTA_API int32_t
gekkota_socket_destroy(GekkotaSocket *socket);

GEKKOTA_API int32_t
gekkota_socket_get_option(
        GekkotaSocket *restrict socket,
        GekkotaSocketOptionLevel level,
        GekkotaSocketOptionName name,
        GekkotaBuffer *value);

GEKKOTA_API int32_t
gekkota_socket_set_option(
        GekkotaSocket *restrict socket,
        GekkotaSocketOptionLevel level,
        GekkotaSocketOptionName name,
        const GekkotaBuffer *value);

GEKKOTA_API GekkotaSocketType
gekkota_socket_get_type(const GekkotaSocket *socket);

GEKKOTA_API GekkotaSocket *
gekkota_socket_accept(const GekkotaSocket *socket);

GEKKOTA_API int32_t
gekkota_socket_connect(
        GekkotaSocket *restrict socket,
        const GekkotaIPEndPoint *remoteEndPoint);

GEKKOTA_API int32_t
gekkota_socket_connect_1(
        GekkotaSocket *restrict socket,
        const char_t *remoteHostNameOrAddress,
        uint16_t port);

#define gekkota_socket_is_connected(socket) \
    (gekkota_socket_poll(socket, GEKKOTA_SELECT_MODE_WRITE, 0) != -1)

GEKKOTA_API GekkotaIPEndPoint *
gekkota_socket_get_local_endpoint(GekkotaSocket *restrict socket);

GEKKOTA_API GekkotaIPEndPoint *
gekkota_socket_get_remote_endpoint(GekkotaSocket *restrict socket);

#define gekkota_socket_join_multicast_group(socket, multicastAddress) \
    (gekkota_socket_join_multicast_group_1(socket, multicastAddress, 0))

GEKKOTA_API int32_t
gekkota_socket_join_multicast_group_1(
        const GekkotaSocket *socket,
        const GekkotaIPAddress *multicastAddress,
        uint32_t multicastInterfaceIndex);

GEKKOTA_API int32_t
gekkota_socket_join_multicast_group_2(
        const GekkotaSocket *socket,
        const GekkotaIPAddress *multicastAddress,
        uint32_t multicastInterfaceIndex,
        GekkotaMulticastTTL ttl);

#define gekkota_socket_leave_multicast_group(socket, multicastAddress) \
    (gekkota_socket_leave_multicast_group_1(socket, multicastAddress, 0))
        
GEKKOTA_API int32_t
gekkota_socket_leave_multicast_group_1(
        const GekkotaSocket *socket,
        const GekkotaIPAddress *multicastAddress,
        uint32_t multicastInterfaceIndex);

GEKKOTA_API int32_t
gekkota_socket_poll(
        const GekkotaSocket *socket,
        GekkotaSelectMode selectMode,
        int32_t timeout);
        
GEKKOTA_API int32_t
gekkota_socket_send(
        const GekkotaSocket *socket,
        const GekkotaIPEndPoint *remoteEndPoint,
        const GekkotaBuffer *buffers,
        size_t bufferCount);

GEKKOTA_API int32_t
gekkota_socket_receive(
        const GekkotaSocket *socket,
        GekkotaBuffer *buffers,
        size_t bufferCount,
        GekkotaIPEndPoint **remoteEndPoint);

#if defined (GEKKOTA_BUILDING_LIB) || defined (GEKKOTA_BUILDING_STATIC_LIB)
#include "gekkota_socket_internal.h"
#endif /* GEKKOTA_BUILDING_LIB || GEKKOTA_BUILDING_STATIC_LIB */

#endif /* !__GEKKOTA_SOCKET_H__ */
