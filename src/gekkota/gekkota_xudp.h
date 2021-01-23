/******************************************************************************
 * @file    gekkota_xudp.h
 * @date    26-Jul-2007
 * @author  <a href="mailto:giuseppe.greco@agamura.com">Giuseppe Greco</a>
 *
 * Copyright (C) 2007 Agamura, Inc. - http://www.agamura.com
 * All right reserved.
 ******************************************************************************/

#ifndef __GEKKOTA_XUDP_H__
#define __GEKKOTA_XUDP_H__

#include "gekkota/gekkota_event.h"
#include "gekkota/gekkota_ipendpoint.h"
#include "gekkota/gekkota_lzf.h"
#include "gekkota/gekkota_packet.h"
#include "gekkota/gekkota_socket.h"
#include "gekkota/gekkota_types.h"
#include "gekkota/gekkota_xudpclient.h"

typedef struct _GekkotaXudp GekkotaXudp;

#define gekkota_xudp_new() \
    (gekkota_xudp_new_5(NULL, 0))

#define gekkota_xudp_new_1(port) \
    (gekkota_xudp_new_4(port, 0))

#define gekkota_xudp_new_2(localEndPoint) \
    (gekkota_xudp_new_5(localEndPoint, 0))

#define gekkota_xudp_new_3(maxClient) \
    (gekkota_xudp_new_5(NULL, maxClient))

GEKKOTA_API GekkotaXudp *
gekkota_xudp_new_4(uint16_t port, uint16_t maxClient);

GEKKOTA_API GekkotaXudp *
gekkota_xudp_new_5(GekkotaIPEndPoint *localEndPoint, uint16_t maxClient);

GEKKOTA_API GekkotaXudp *
gekkota_xudp_new_6(
        char_t *localHostNameOrAddress,
        uint16_t port,
        uint16_t maxClient);

GEKKOTA_API int32_t
gekkota_xudp_destroy(GekkotaXudp *xudp);

GEKKOTA_API int32_t
gekkota_xudp_get_max_client(const GekkotaXudp *xudp);

GEKKOTA_API uint32_t
gekkota_xudp_get_incoming_bandwidth(const GekkotaXudp *xudp);

GEKKOTA_API int32_t
gekkota_xudp_set_incoming_bandwidth(GekkotaXudp *restrict xudp, uint32_t bandwidth);

GEKKOTA_API uint32_t
gekkota_xudp_get_outgoing_bandwidth(const GekkotaXudp *xudp);

GEKKOTA_API int32_t
gekkota_xudp_set_outgoing_bandwidth(GekkotaXudp *restrict xudp, uint32_t bandwidth);

GEKKOTA_API GekkotaSocket *
gekkota_xudp_get_socket(const GekkotaXudp *xudp);

GEKKOTA_API int32_t
gekkota_xudp_broadcast(
        GekkotaXudp *restrict xudp,
        uint8_t channelId,
        GekkotaPacket *packet);

#define gekkota_xudp_connect(xudp, remoteEndPoint) \
    (gekkota_xudp_connect_3( \
            xudp, \
            remoteEndPoint, \
            0, \
            GEKKOTA_COMPRESSION_LEVEL_FAST))

#define gekkota_xudp_connect_1(xudp, remoteEndPoint, compressionLevel) \
    (gekkota_xudp_connect_3(xudp, remoteEndPoint, 0, compressionLevel))

GEKKOTA_API GekkotaXudpClient *
gekkota_xudp_connect_2(
        GekkotaXudp *restrict xudp,
        char_t *remoteHostNameOrAddress,
        uint16_t port,
        uint8_t channelCount,
        GekkotaCompressionLevel compressionLevel);

GEKKOTA_API GekkotaXudpClient *
gekkota_xudp_connect_3(
        GekkotaXudp *restrict xudp,
        GekkotaIPEndPoint *remoteEndPoint,
        uint8_t channelCount,
        GekkotaCompressionLevel compressionLevel);

#define gekkota_xudp_join_multicast_group( \
        xudp, \
        multicastEndPoint, \
        multicastInterfaceIndex, \
        channelCount) \
    (gekkota_xudp_join_multicast_group_1( \
            xudp, \
            multicastEndPoint, \
            multicastInterfaceIndex, \
            GEKKOTA_MULTICAST_TTL_SUBNET, \
            channelCount))

GEKKOTA_API GekkotaXudpClient *
gekkota_xudp_join_multicast_group_1(
        GekkotaXudp *restrict xudp,
        GekkotaIPEndPoint *multicastEndPoint,
        uint32_t multicastInterfaceIndex,
        GekkotaMulticastTTL ttl,
        uint8_t channelCount);

GEKKOTA_API int32_t
gekkota_xudp_flush(GekkotaXudp *xudp);

GEKKOTA_API int32_t
gekkota_xudp_poll(GekkotaXudp *xudp, GekkotaEvent **event, int32_t timeout);

#if defined (GEKKOTA_BUILDING_LIB) || defined (GEKKOTA_BUILDING_STATIC_LIB)
#include "gekkota_xudp_internal.h"
#endif /* GEKKOTA_BUILDING_LIB || GEKKOTA_BUILDING_STATIC_LIB */

#endif /* !__GEKKOTA_XUDP_H__ */
