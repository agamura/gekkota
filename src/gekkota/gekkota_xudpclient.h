/******************************************************************************
 * @file    gekkota_xudpclient.h
 * @date    26-Jul-2007
 * @author  <a href="mailto:giuseppe.greco@agamura.com">Giuseppe Greco</a>
 *
 * Copyright (C) 2007 Agamura, Inc. - http://www.agamura.com
 * All right reserved.
 ******************************************************************************/

#ifndef __GEKKOTA_XUDPCLIENT_H__
#define __GEKKOTA_XUDPCLIENT_H__

#include "gekkota/gekkota_ipendpoint.h"
#include "gekkota/gekkota_lzf.h"
#include "gekkota/gekkota_packet.h"
#include "gekkota/gekkota_types.h"

typedef enum
{
    GEKKOTA_CLIENT_STATE_UNDEFINED = 0,
    GEKKOTA_CLIENT_STATE_DISCONNECTED,
    GEKKOTA_CLIENT_STATE_CONNECTING,
    GEKKOTA_CLIENT_STATE_ACKNOWLEDGING_CONNECT,
    GEKKOTA_CLIENT_STATE_CONNECTION_PENDING,
    GEKKOTA_CLIENT_STATE_CONNECTION_SUCCEEDED,
    GEKKOTA_CLIENT_STATE_CONNECTED,
    GEKKOTA_CLIENT_STATE_DELAYING_DISCONNECT,
    GEKKOTA_CLIENT_STATE_DISCONNECTING,
    GEKKOTA_CLIENT_STATE_ACKNOWLEDGING_DISCONNECT,
    GEKKOTA_CLIENT_STATE_ZOMBIE
} GekkotaClientState;

typedef enum
{
    GEKKOTA_CLOSE_MODE_UNDEFINED = 0,
    GEKKOTA_CLOSE_MODE_GRACEFUL,
    GEKKOTA_CLOSE_MODE_DELAYED,
    GEKKOTA_CLOSE_MODE_IMMEDIATE
} GekkotaCloseMode;

typedef struct _GekkotaXudpClient GekkotaXudpClient;

GEKKOTA_API int32_t
gekkota_xudpclient_destroy(GekkotaXudpClient *client);

#define gekkota_xudpclient_close(client) \
    (gekkota_xudpclient_close_1(client, GEKKOTA_CLOSE_MODE_GRACEFUL))

GEKKOTA_API int32_t
gekkota_xudpclient_close_1(GekkotaXudpClient *client, GekkotaCloseMode closeMode);

GEKKOTA_API int32_t
gekkota_xudpclient_get_channel_count(const GekkotaXudpClient *client);

GEKKOTA_API GekkotaCompressionLevel
gekkota_xudpclient_get_compression_level(const GekkotaXudpClient *client);

GEKKOTA_API GekkotaIPEndPoint *
gekkota_xudpclient_get_local_endpoint(const GekkotaXudpClient *client);

GEKKOTA_API GekkotaIPEndPoint *
gekkota_xudpclient_get_remote_endpoint(const GekkotaXudpClient *client);

GEKKOTA_API GekkotaClientState
gekkota_xudpclient_get_state(const GekkotaXudpClient *client);

GEKKOTA_API int32_t
gekkota_xudpclient_get_throttle_configuration(
        GekkotaXudpClient *client,
        uint32_t *restrict interval,
        uint32_t *restrict acceleration,
        uint32_t *restrict deceleration);

GEKKOTA_API int32_t
gekkota_xudpclient_set_throttle_configuration(
        GekkotaXudpClient *client,
        uint32_t interval,
        uint32_t acceleration,
        uint32_t deceleration);

GEKKOTA_API bool_t
gekkota_xudpclient_is_multicast_group_member(const GekkotaXudpClient *client);

GEKKOTA_API int32_t
gekkota_xudpclient_receive(
        GekkotaXudpClient *client,
        uint8_t channelId,
        GekkotaPacket **packet,
        GekkotaIPEndPoint **remoteEndPoint);
    
GEKKOTA_API int32_t
gekkota_xudpclient_send(
        GekkotaXudpClient *client,
        uint8_t channelId,
        GekkotaPacket *packet);

GEKKOTA_API int32_t
gekkota_xudpclient_ping(GekkotaXudpClient *client);

#if defined (GEKKOTA_BUILDING_LIB) || defined (GEKKOTA_BUILDING_STATIC_LIB)
#include "gekkota_xudpclient_internal.h"
#endif /* GEKKOTA_BUILDING_LIB || GEKKOTA_BUILDING_STATIC_LIB */

#endif /* !__GEKKOTA_XUDPCLIENT_H__ */
