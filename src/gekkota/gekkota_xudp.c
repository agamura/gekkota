/******************************************************************************
 * @file    gekkota_xudp.c
 * @date    26-Jul-2007
 * @author  <a href="mailto:giuseppe.greco@agamura.com">Giuseppe Greco</a>
 *
 * Copyright (C) 2007 Agamura, Inc. - http://www.agamura.com
 * All right reserved.
 ******************************************************************************/

#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include "gekkota.h"
#include "gekkota_bit.h"
#include "gekkota_dns.h"
#include "gekkota_errors.h"
#include "gekkota_memory.h"
#include "gekkota_iphostentry.h"
#include "gekkota_list.h"
#include "gekkota_time.h"
#include "gekkota_utils.h"
#include "gekkota_xudp.h"

#ifdef CRC32_ENABLED
#include "gekkota_crc32.h"
#endif /* CRC32_ENABLED */

#define GEKKOTA_XUDP_MULTICAST_PORT_LENGTH      5
#define GEKKOTA_XUDP_MULTICAST_ID_LENGTH        INET6_ADDRSTRLEN + \
                                                GEKKOTA_XUDP_MULTICAST_PORT_LENGTH

typedef struct _GekkotaXudpMessageHandlerArgs
{
    GekkotaXudpHeader   *header;                /* [in] XUDP header */
    GekkotaXudpMessage  *message;               /* [in] XUDP message */
    GekkotaXudpClient   *client;                /* [in|out] XUDP client */
    byte_t              *data;                  /* [in|out] received data */
} GekkotaXudpMessageHandlerArgs;

/*
 * XUDP message handlers must return a value less than zero on failure,
 * zero if the message was discarded, and a value greater than zero if
 * the message was handled successfully.
 */

typedef int32_t (GEKKOTA_CALLBACK *XudpMessageHandler) (
        GekkotaXudp *xudp,                      /* XUDP instance */
        GekkotaXudpMessageHandlerArgs *args,    /* message handler arguments */
        GekkotaEvent **event);                  /* event the message handler
                                                   might generate */

static GekkotaXudp *
_gekkota_xudp_new(GekkotaIPEndPoint *localEndPoint, uint16_t maxClient);

static int32_t
_gekkota_xudp_acknowledge(
        const GekkotaXudp *xudp,
        GekkotaXudpMessageHandlerArgs *args);

static int32_t
_gekkota_xudp_check_for_timeouts(
        GekkotaXudp *xudp,
        GekkotaXudpClient *client,
        GekkotaEvent **event);

static int32_t
_gekkota_xudp_dispatch(
        GekkotaXudp *restrict xudp,
        GekkotaEvent **restrict event);

static GekkotaXudpMessageType
_gekkota_xudp_dispose_acknowledged_message(
        GekkotaXudpClient *restrict client,
        uint16_t sequenceNumber,
        uint8_t channelId);

static int32_t
_gekkota_xudp_get_message_handler_args(
        GekkotaXudp *xudp,
        GekkotaXudpMessageHandlerArgs *args);

static int32_t
_gekkota_xudp_notify_connect(
        GekkotaXudp *restrict xudp,
        GekkotaXudpClient *restrict client,
        GekkotaEvent **restrict event);

static int32_t
_gekkota_xudp_notify_disconnect(
        GekkotaXudp *restrict xudp,
        GekkotaXudpClient *client,
        GekkotaEvent **restrict event);

static int32_t
_gekkota_xudp_on_acknowledge_message(
        GekkotaXudp *xudp,
        GekkotaXudpMessageHandlerArgs *args,
        GekkotaEvent **event);

static int32_t
_gekkota_xudp_on_connect_message(
        GekkotaXudp *xudp,
        GekkotaXudpMessageHandlerArgs *args,
        GekkotaEvent **event);

static int32_t
_gekkota_xudp_on_validate_connect_message(
        GekkotaXudp *xudp,
        GekkotaXudpMessageHandlerArgs *args,
        GekkotaEvent **event);

static int32_t
_gekkota_xudp_on_disconnect_message(
        GekkotaXudp *xudp,
        GekkotaXudpMessageHandlerArgs *args,
        GekkotaEvent **event);

static int32_t
_gekkota_xudp_on_join_multicast_group_message(
        GekkotaXudp *xudp,
        GekkotaXudpMessageHandlerArgs *args,
        GekkotaEvent **event);

static int32_t
_gekkota_xudp_on_leave_multicast_group_message(
        GekkotaXudp *xudp,
        GekkotaXudpMessageHandlerArgs *args,
        GekkotaEvent **event);

static int32_t
_gekkota_xudp_on_ping_message(
        GekkotaXudp *xudp,
        GekkotaXudpMessageHandlerArgs *args,
        GekkotaEvent **event);

static int32_t
_gekkota_xudp_on_reliable_data_message(
        GekkotaXudp *xudp,
        GekkotaXudpMessageHandlerArgs *args,
        GekkotaEvent **event);

static int32_t
_gekkota_xudp_on_unreliable_data_message(
        GekkotaXudp *xudp,
        GekkotaXudpMessageHandlerArgs *args,
        GekkotaEvent **event);

static int32_t
_gekkota_xudp_on_unsequenced_data_message(
        GekkotaXudp *xudp,
        GekkotaXudpMessageHandlerArgs *args,
        GekkotaEvent **event);

static int32_t
_gekkota_xudp_on_data_fragment_message(
        GekkotaXudp *xudp,
        GekkotaXudpMessageHandlerArgs *args,
        GekkotaEvent **event);

static int32_t
_gekkota_xudp_on_configure_bandwidth_message(
        GekkotaXudp *xudp,
        GekkotaXudpMessageHandlerArgs *args,
        GekkotaEvent **event);

static int32_t
_gekkota_xudp_on_configure_throttle_message(
        GekkotaXudp *xudp,
        GekkotaXudpMessageHandlerArgs *args,
        GekkotaEvent **event);

static int32_t
_gekkota_xudp_receive(
        GekkotaXudp *xudp,
        GekkotaEvent **event);

static int32_t
_gekkota_xudp_send(
        GekkotaXudp *xudp,
        GekkotaEvent **event,
        bool_t checkForTimeouts);

static int32_t
_gekkota_xudp_send_acknowledgements(
        GekkotaXudp *restrict xudp,
        GekkotaXudpClient *restrict client);

static int32_t
_gekkota_xudp_send_reliable(
        GekkotaXudp *restrict xudp,
        GekkotaXudpClient *restrict client);

static int32_t
_gekkota_xudp_send_unreliable(
        GekkotaXudp *restrict xudp,
        GekkotaXudpClient *client);

static int32_t
_gekkota_xudp_throttle_bandwidth(GekkotaXudp *restrict xudp);

static size_t messageSizes[] =
{
    0,
    sizeof(GekkotaXudpAcknowledgeMessage),
    sizeof(GekkotaXudpConnectMessage),
    sizeof(GekkotaXudpValidateConnectMessage),
    sizeof(GekkotaXudpDisconnectMessage),
    sizeof(GekkotaXudpJoinMulticastGroupMessage),
    sizeof(GekkotaXudpLeaveMulticastGroupMessage),
    sizeof(GekkotaXudpPingMessage),
    sizeof(GekkotaXudpReliableDataMessage),
    sizeof(GekkotaXudpUnreliableDataMessage),
    sizeof(GekkotaXudpUnsequencedDataMessage),
    sizeof(GekkotaXudpDataFragmentMessage),
    sizeof(GekkotaXudpConfigureBandwidthMessage),
    sizeof(GekkotaXudpConfigureThrottleMessage)
};

static XudpMessageHandler messageHandlers[] =
{
    NULL,
    _gekkota_xudp_on_acknowledge_message,
    _gekkota_xudp_on_connect_message,
    _gekkota_xudp_on_validate_connect_message,
    _gekkota_xudp_on_disconnect_message,
    _gekkota_xudp_on_join_multicast_group_message,
    _gekkota_xudp_on_leave_multicast_group_message,
    _gekkota_xudp_on_ping_message,
    _gekkota_xudp_on_reliable_data_message,
    _gekkota_xudp_on_unreliable_data_message,
    _gekkota_xudp_on_unsequenced_data_message,
    _gekkota_xudp_on_data_fragment_message,
    _gekkota_xudp_on_configure_bandwidth_message,
    _gekkota_xudp_on_configure_throttle_message
};

GekkotaXudp *
gekkota_xudp_new_4(uint16_t port, uint16_t maxClient)
{
    GekkotaXudp *xudp;
    GekkotaIPEndPoint *localEndPoint;

    if (!gekkota_is_initialized())
    {
        errno = GEKKOTA_ERROR_LIB_NOT_INITIALIZED;
        return NULL;
    }

    if ((localEndPoint = gekkota_ipendpoint_new(NULL, port)) == NULL)
        return NULL;

    xudp = _gekkota_xudp_new(localEndPoint, maxClient);
    gekkota_ipendpoint_destroy(localEndPoint);

    return xudp;
}

GekkotaXudp *
gekkota_xudp_new_5(GekkotaIPEndPoint *localEndPoint, uint16_t maxClient)
{
    if (!gekkota_is_initialized())
    {
        errno = GEKKOTA_ERROR_LIB_NOT_INITIALIZED;
        return NULL;
    }

    return _gekkota_xudp_new(localEndPoint, maxClient);
}

GekkotaXudp *
gekkota_xudp_new_6(
        char_t *localHostNameOrAddress,
        uint16_t port,
        uint16_t maxClient)
{
    GekkotaXudp *xudp = NULL;
    GekkotaIPHostEntry *hostEntry;

    if (localHostNameOrAddress == NULL)
        return gekkota_xudp_new_4(port, maxClient);

    if (!gekkota_is_initialized())
    {
        errno = GEKKOTA_ERROR_LIB_NOT_INITIALIZED;
        return NULL;
    }

    if ((hostEntry = gekkota_dns_get_host_entry(localHostNameOrAddress)) != NULL)
    {
        GekkotaList *hostAddresses;
        GekkotaListIterator iterator;
        GekkotaIPEndPoint *localEndPoint;
    
        hostAddresses = gekkota_iphostentry_get_addresses(hostEntry);

        for (iterator = gekkota_list_first(hostAddresses);
                iterator != gekkota_list_tail(hostAddresses);
                iterator = gekkota_list_next(iterator))
        {
            if ((localEndPoint = gekkota_ipendpoint_new(
                    gekkota_iphostentry_address(
                            (GekkotaIPHostAddress *) iterator), port)) == NULL)
                return NULL;

            xudp = _gekkota_xudp_new(localEndPoint, maxClient);
            gekkota_ipendpoint_destroy(localEndPoint);

            if (xudp != NULL)
                break;
        }

        gekkota_iphostentry_destroy(hostEntry);
    }

    return xudp;
}

int32_t
gekkota_xudp_destroy(GekkotaXudp *xudp)
{
    GekkotaXudpClient *client;

    if (xudp == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return -1;
    }

    for (client = xudp->clients;
            client < &xudp->clients[xudp->clientCount];
            client++)
        gekkota_xudpclient_destroy(client);

    gekkota_socket_destroy(xudp->socket);
    gekkota_ipendpoint_destroy(xudp->remoteEndPoint);

    gekkota_memory_free(xudp);
    return 0;
}

int32_t
gekkota_xudp_get_max_client(const GekkotaXudp *xudp)
{
    if (xudp == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return -1;
    }

    return (int32_t) xudp->clientCount;
}

uint32_t
gekkota_xudp_get_incoming_bandwidth(const GekkotaXudp *xudp)
{
    if (xudp == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return 0;
    }

    errno = GEKKOTA_ERROR_SUCCESS;
    return xudp->incomingBandwidth;
}

int32_t
gekkota_xudp_set_incoming_bandwidth(GekkotaXudp *restrict xudp, uint32_t bandwidth)
{
    if (xudp == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return -1;
    }

    xudp->incomingBandwidth = bandwidth;
    xudp->reconfigureBandwidth = TRUE;

    return 0;
}

uint32_t
gekkota_xudp_get_outgoing_bandwidth(const GekkotaXudp *xudp)
{
    if (xudp == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return 0;
    }

    errno = GEKKOTA_ERROR_SUCCESS;
    return xudp->outgoingBandwidth;
}

int32_t
gekkota_xudp_set_outgoing_bandwidth(GekkotaXudp *restrict xudp, uint32_t bandwidth)
{
    if (xudp == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return -1;
    }

    xudp->outgoingBandwidth = bandwidth;
    xudp->reconfigureBandwidth = TRUE;

    return 0;
}

GekkotaSocket *
gekkota_xudp_get_socket(const GekkotaXudp *xudp)
{
    if (xudp == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return NULL;
    }

    return xudp->socket;
}

int32_t
gekkota_xudp_broadcast(
        GekkotaXudp *restrict xudp,
        uint8_t channelId,
        GekkotaPacket *packet)
{
    GekkotaXudpClient *client;

    if (xudp == NULL || packet == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return -1;
    }

    for (client = xudp->clients;
            client < &xudp->clients[xudp->clientCount];
            client++)
    {
        if (client->state != GEKKOTA_CLIENT_STATE_CONNECTED)
            continue;

        if (gekkota_xudpclient_send(client, channelId, packet) != 0)
            return -1;
    }

    return 0;
}

GekkotaXudpClient *
gekkota_xudp_connect_2(
        GekkotaXudp *restrict xudp,
        char_t *remoteHostNameOrAddress,
        uint16_t port,
        uint8_t channelCount,
        GekkotaCompressionLevel compressionLevel)
{
    GekkotaIPHostEntry *hostEntry;
    GekkotaXudpClient *client = NULL;

    if (xudp == NULL || remoteHostNameOrAddress == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return NULL;
    }

    if ((hostEntry = gekkota_dns_get_host_entry(remoteHostNameOrAddress)) != NULL)
    {
        GekkotaList *hostAddresses;
        GekkotaListIterator iterator;
        GekkotaIPEndPoint *remoteEndPoint;
    
        hostAddresses = gekkota_iphostentry_get_addresses(hostEntry);

        for (iterator = gekkota_list_first(hostAddresses);
                iterator != gekkota_list_tail(hostAddresses);
                iterator = gekkota_list_next(iterator))
        {
            if ((remoteEndPoint = gekkota_ipendpoint_new(
                    gekkota_iphostentry_address(
                            (GekkotaIPHostAddress *) iterator), port)) == NULL)
                return NULL;

            client = gekkota_xudp_connect_3(
                    xudp,
                    remoteEndPoint,
                    channelCount, compressionLevel);

            gekkota_ipendpoint_destroy(remoteEndPoint);

            if (client != NULL)
                break;
        }

        gekkota_iphostentry_destroy(hostEntry);
    }

    return client;
}

GekkotaXudpClient *
gekkota_xudp_connect_3(
        GekkotaXudp *restrict xudp,
        GekkotaIPEndPoint *remoteEndPoint,
        uint8_t channelCount,
        GekkotaCompressionLevel compressionLevel)
{
    GekkotaXudpClient *client;
    GekkotaChannel *channel;
    GekkotaXudpMessage message;

    if (xudp == NULL || remoteEndPoint == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return NULL;
    }

    if (gekkota_ipaddress_is_multicast(remoteEndPoint->address) ||
            compressionLevel < GEKKOTA_COMPRESSION_LEVEL_FASTEST ||
            compressionLevel > GEKKOTA_COMPRESSION_LEVEL_BEST)
    {
        errno = GEKKOTA_ERROR_ARGUMENT_NOT_VALID;
        return NULL;
    }

    if (channelCount == 0)
        channelCount = GEKKOTA_XUDP_DEFAULT_CHANNEL_COUNT;

    /*
     * Search for the first available client, if any.
     */
    for (client = xudp->clients;
            client < &xudp->clients[xudp->clientCount];
            client++)
        if (client->state == GEKKOTA_CLIENT_STATE_DISCONNECTED)
            break;

    if (client >= &xudp->clients[xudp->clientCount])
    {
        errno = GEKKOTA_ERROR_MAX_CLIENT_REACHED;
        return NULL;
    }

    if ((client->channels = gekkota_memory_alloc(
            sizeof(GekkotaChannel) * channelCount, TRUE)) == NULL)
        return NULL;

    client->channelCount = channelCount;
    client->state = GEKKOTA_CLIENT_STATE_CONNECTING;
    client->remoteEndPoint = gekkota_ipendpoint_new_0(remoteEndPoint, FALSE);
    client->isMulticastGroupMember = FALSE;
    client->sessionId = (uint32_t) rand();
    client->compressionLevel = compressionLevel;

    if (xudp->outgoingBandwidth == 0)
        client->windowSize = GEKKOTA_XUDP_MAX_WINDOW_SIZE;
    else
    {
        client->windowSize = (xudp->outgoingBandwidth
                / GEKKOTA_XUDP_CLIENT_WINDOW_SIZE_SCALE)
                * GEKKOTA_XUDP_MIN_WINDOW_SIZE;

        if (client->windowSize < GEKKOTA_XUDP_MIN_WINDOW_SIZE)
            client->windowSize = GEKKOTA_XUDP_MIN_WINDOW_SIZE;
        else if (client->windowSize > GEKKOTA_XUDP_MAX_WINDOW_SIZE)
            client->windowSize = GEKKOTA_XUDP_MAX_WINDOW_SIZE;
    }

    for (channel = client->channels;
            channel < &client->channels[channelCount];
            channel++)
    {
        gekkota_list_clear(&channel->incomingReliableMessages);
        gekkota_list_clear(&channel->incomingUnreliableMessages);
    }

    /* message header */
    message.header.messageType = GEKKOTA_XUDP_MESSAGE_TYPE_CONNECT;
    message.header.channelId = 0xFF;
    message.header.flags = GEKKOTA_XUDP_MESSAGE_FLAG_ACKNOWLEDGE;

    /* message body */
    message.connect.clientId = gekkota_host_to_net_16(client->localClientId);
    message.connect.sessionId = client->sessionId;
    message.connect.channelCount = client->channelCount;
    message.connect.mtu = gekkota_host_to_net_16(client->mtu);
    message.connect.windowSize = gekkota_host_to_net_32(client->windowSize);
    message.connect.incomingBandwidth = gekkota_host_to_net_32(xudp->incomingBandwidth);
    message.connect.outgoingBandwidth = gekkota_host_to_net_32(xudp->outgoingBandwidth);
    message.connect.throttleInterval = gekkota_host_to_net_32(client->packetThrottleInterval);
    message.connect.throttleAcceleration = gekkota_host_to_net_32(client->packetThrottleAcceleration);
    message.connect.throttleDeceleration = gekkota_host_to_net_32(client->packetThrottleDeceleration);
    message.connect.compressionLevel = compressionLevel;

    if (_gekkota_xudpclient_queue_outgoing_message(
            client, &message, NULL, 0, 0, NULL) != 0)
    {
        gekkota_xudpclient_destroy(client);
        return NULL;
    }

    return client;
}

GekkotaXudpClient *
gekkota_xudp_join_multicast_group_1(
        GekkotaXudp *restrict xudp,
        GekkotaIPEndPoint *multicastEndPoint,
        uint32_t multicastInterfaceIndex,
        GekkotaMulticastTTL ttl,
        uint8_t channelCount)
{
    char_t multicastPort[GEKKOTA_XUDP_MULTICAST_PORT_LENGTH];
    char_t multicastId[GEKKOTA_XUDP_MULTICAST_ID_LENGTH];

    GekkotaBuffer buffer;
    GekkotaXudpMessage message;
    GekkotaChannel *channel;
    GekkotaXudpClient *client;
    GekkotaIPAddress *multicastAddress = NULL;

    if (xudp == NULL || multicastEndPoint == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return NULL;
    }

    if (channelCount == 0)
        channelCount = GEKKOTA_XUDP_DEFAULT_CHANNEL_COUNT;

    /*
     * Search for the first available client, if any.
     */
    for (client = xudp->clients;
            client < &xudp->clients[xudp->clientCount];
            client++)
    {
        if (client->state == GEKKOTA_CLIENT_STATE_CONNECTED &&
                client->isMulticastGroupMember &&
                client->multicastInterfaceIndex == multicastInterfaceIndex &&
                gekkota_ipaddress_equals(
                        client->remoteEndPoint->address,
                        multicastEndPoint->address))
        {
            if (client->remoteEndPoint->port == multicastEndPoint->port)
            {
                /*
                 * The multicast group has already been joined for the specified
                 * multicast endpoint. Abort.
                 */
                errno = GEKKOTA_ERROR_MULTICAST_GROUP_ALREADY_JOINED;
                return NULL;
            }

            multicastAddress = client->remoteEndPoint->address;
        }
        else if (client->state == GEKKOTA_CLIENT_STATE_DISCONNECTED)
            break;
    }

    if (client >= &xudp->clients[xudp->clientCount])
    {
        errno = GEKKOTA_ERROR_MAX_CLIENT_REACHED;
        return NULL;
    }

    if ((client->channels = gekkota_memory_alloc(
            sizeof(GekkotaChannel) * channelCount, TRUE)) == NULL)
        return NULL;

    /*
     * Join the multicast group if and only if the multicast port is
     * equal to the local port.
     */
    if (multicastEndPoint->port == gekkota_socket_get_local_endpoint(xudp->socket)->port)
    {
        if (gekkota_socket_join_multicast_group_2(
                xudp->socket,
                multicastEndPoint->address,
                multicastInterfaceIndex,
                ttl) < 0)
        {
            gekkota_memory_free(client->channels);
            client->channels = NULL;

            return NULL;
        }
    }

    /*
     * The multicast id consists of a 16-bit hash code generated from the
     * string representation of the multicast endpoint.
     */
    buffer.data = multicastId;
    buffer.length = sizeof(multicastId);
    gekkota_ipaddress_to_string(multicastEndPoint->address, &buffer);
#ifdef WIN32
    _itoa_s(multicastEndPoint->port, multicastPort, sizeof(multicastPort), 16);
    strcat_s(multicastId, sizeof(multicastId), ":");
    strcat_s(multicastId, sizeof(multicastId), multicastPort);
#else
    itoa(multicastEndPoint->port, multicastPort, 16);
    strcat(multicastId, ":");
    strcat(multicastId, multicastPort);
#endif /* WIN32 */

    if (multicastAddress != NULL)
        /*
         * The multicast address is already used by other clients;
         * reuse the address object to save some memory.
         */
        gekkota_ipendpoint_set_address(multicastEndPoint, multicastAddress);

    client->remoteClientId = gekkota_hash_16(multicastId);
    client->remoteEndPoint = gekkota_ipendpoint_new_0(multicastEndPoint, FALSE);
    client->isMulticastGroupMember = TRUE;
    client->multicastInterfaceIndex = multicastInterfaceIndex;
    client->state = GEKKOTA_CLIENT_STATE_CONNECTED;
    client->channelCount = channelCount;
    client->compressionLevel = GEKKOTA_COMPRESSION_LEVEL_FAST;

    if (xudp->outgoingBandwidth == 0)
        client->windowSize = GEKKOTA_XUDP_MAX_WINDOW_SIZE;
    else
    {
        client->windowSize = (xudp->outgoingBandwidth
                / GEKKOTA_XUDP_CLIENT_WINDOW_SIZE_SCALE)
                * GEKKOTA_XUDP_MIN_WINDOW_SIZE;

        if (client->windowSize < GEKKOTA_XUDP_MIN_WINDOW_SIZE)
            client->windowSize = GEKKOTA_XUDP_MIN_WINDOW_SIZE;
        else if (client->windowSize > GEKKOTA_XUDP_MAX_WINDOW_SIZE)
            client->windowSize = GEKKOTA_XUDP_MAX_WINDOW_SIZE;
    }

    for (channel = client->channels;
            channel < &client->channels[channelCount];
            channel++)
    {
        gekkota_list_clear(&channel->incomingReliableMessages);
        gekkota_list_clear(&channel->incomingUnreliableMessages);
    }

    /* message header */
    message.header.messageType = GEKKOTA_XUDP_MESSAGE_TYPE_JOIN_MULTICAST_GROUP;
    message.header.channelId = 0xFF;
    message.header.flags = GEKKOTA_XUDP_MESSAGE_FLAG_UNSEQUENCED;

    if (_gekkota_xudpclient_queue_outgoing_message(
            client, &message, NULL, 0, 0, NULL) < 0)
    {
        gekkota_xudpclient_destroy(client);
        return NULL;
    }

    return client;
}

int32_t
gekkota_xudp_flush(GekkotaXudp *xudp)
{
    if (xudp == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return -1;
    }

    xudp->currentTime = (uint32_t) gekkota_time_now();
    return _gekkota_xudp_send(xudp, NULL, FALSE);
}

int32_t
gekkota_xudp_poll(GekkotaXudp *xudp, GekkotaEvent **event, int32_t timeout)
{
    int32_t poll;
    time_t pollTimeout;

    if (xudp == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return -1;
    }

    if (event != NULL)
    {
        switch (_gekkota_xudp_dispatch(xudp, event))
        {
            case -1:    return -1;  /* error */
            case 1:     return 1;   /* message dispatched */
            default:    break;      /* no message dispatched */
        }
    }

    xudp->currentTime = (uint32_t) gekkota_time_now();

    if (timeout >= 0)
        pollTimeout = (time_t) timeout + xudp->currentTime;

    do
    {
        if (gekkota_time_get_lag(
                xudp->currentTime,
                xudp->bandwidthThrottleEpoch) >= GEKKOTA_XUDP_BANDWIDTH_THROTTLE_INTERVAL)
        {
            _gekkota_xudp_throttle_bandwidth(xudp);
        }

        switch (_gekkota_xudp_send(xudp, event, TRUE))
        {
            case -1:    return -1;  /* error */
            case 1:     return 1;   /* connection timeout occurred */
            default:    break;      /* no connection timeout occurred */
        }

        switch (_gekkota_xudp_receive(xudp, event))
        {
            case -1:    return -1;  /* error */
            case 1:     return 1;   /* message received */
            default:    break;      /* no message received */
        }

        switch (_gekkota_xudp_send(xudp, event, TRUE))
        {
            case -1:    return -1;  /* error */
            case 1:     return 1;   /* connection timeout occurred */
            default:    break;      /* no connection timeout occurred */
        }

        if (event != NULL)
        {
            switch (_gekkota_xudp_dispatch(xudp, event))
            {
                case -1:    return -1;  /* error */
                case 1:     return 1;   /* message dispatched */
                default:    break;      /* no message dispatched */
            }
        }

        xudp->currentTime = (uint32_t) gekkota_time_now();

        /*
         * If [timeout] is greater than 0, then wait at least [timeout]
         * milliseconds for an event to occur on the underlying socket.
         *
         * If [timeout] is 0, then return immediately.
         *
         * if [timeout] is less than 0, then wait for a predefined amount of
         * time and continue looping.
         */

        if (timeout > -1)
            if (gekkota_time_compare(xudp->currentTime, pollTimeout) >= 0)
                return 0;

        /*
         * Listen for incoming packets.
         */
        if ((poll = gekkota_socket_poll(
                xudp->socket, GEKKOTA_SELECT_MODE_READ,
                timeout < 0
                    ? GEKKOTA_XUDP_DEFAULT_POLL_TIMEOUT
                    : (int32_t) gekkota_time_get_lag(pollTimeout, xudp->currentTime))) == -1)
            return -1;

        xudp->currentTime = (uint32_t) gekkota_time_now();
    } while (poll > 0 || timeout < 0);

    return 0;
}

size_t
_gekkota_xudp_message_size(GekkotaXudpMessageType messageType)
{
    return messageSizes[messageType];
}

static GekkotaXudp *
_gekkota_xudp_new(GekkotaIPEndPoint *localEndPoint, uint16_t maxClient)
{
    GekkotaXudp *xudp;
    GekkotaXudpClient *client;
    size_t memSize;

    if (maxClient == 0)
        maxClient = GEKKOTA_XUDP_DEFAULT_CLIENT_COUNT;

    memSize = sizeof(GekkotaXudp) + (sizeof(GekkotaXudpClient) * maxClient);

    if ((xudp = gekkota_memory_alloc(memSize, TRUE)) == NULL)
        return NULL;

    if ((xudp->socket = gekkota_socket_new_3(
            GEKKOTA_SOCKET_TYPE_DATAGRAM, localEndPoint)) == NULL)
    {
        gekkota_memory_free(xudp);
        return NULL;
    }

    xudp->protocolId = gekkota_host_to_net_16(gekkota_hash_16(GEKKOTA_XUDP_ID));
    xudp->clients = (GekkotaXudpClient *) (xudp + 1);
    xudp->clientCount = maxClient;
    xudp->mtu = GEKKOTA_XUDP_DEFAULT_MTU;
    xudp->lastServicedClient = xudp->clients;

    for (client = xudp->clients;
            client < &xudp->clients[xudp->clientCount];
            client++)
    {
        client->xudp = xudp;

        gekkota_list_clear(&client->acknowledgements);
        gekkota_list_clear(&client->sentReliableMessages);
        gekkota_list_clear(&client->sentUnreliableMessages);
        gekkota_list_clear(&client->outgoingReliableMessages);
        gekkota_list_clear(&client->outgoingUnreliableMessages);

        _gekkota_xudpclient_reset(client);
    }

    return xudp;
}

static int32_t
_gekkota_xudp_acknowledge(
        const GekkotaXudp *xudp,
        GekkotaXudpMessageHandlerArgs *args)
{
    uint16_t sentTime;

    if (!gekkota_bit_isset(
            args->message->header.flags,
            GEKKOTA_XUDP_MESSAGE_FLAG_ACKNOWLEDGE))
        return 0;

    if (!gekkota_bit_isset(
            args->header->version & GEKKOTA_XUDP_HEADER_FLAG_MASK,
            GEKKOTA_XUDP_HEADER_FLAG_SENT_TIME))
        return 0;

    sentTime = gekkota_net_to_host_16(args->header->sentTime);

    switch (args->client->state)
    {
        case GEKKOTA_CLIENT_STATE_DISCONNECTING:
        case GEKKOTA_CLIENT_STATE_ACKNOWLEDGING_CONNECT:
            return 0;

        case GEKKOTA_CLIENT_STATE_ACKNOWLEDGING_DISCONNECT:
            if (args->message->header.messageType == GEKKOTA_XUDP_MESSAGE_TYPE_DISCONNECT)
            {
                if (_gekkota_xudpclient_queue_acknowledgement(
                        args->client, args->message, sentTime, NULL) != 0)
                    return -1;

                break;
            }
            
            return 0;

        default:
            if (_gekkota_xudpclient_queue_acknowledgement(
                    args->client, args->message, sentTime, NULL) != 0)
                return -1;

            break;
    }

    return 1;
}

static int32_t
_gekkota_xudp_check_for_timeouts(
        GekkotaXudp *xudp,
        GekkotaXudpClient *client,
        GekkotaEvent **event)
{
    GekkotaOutgoingMessage *outgoingMessage;
    GekkotaListIterator iterator;

    iterator = gekkota_list_head(&client->sentReliableMessages);

    while (iterator != gekkota_list_tail(&client->sentReliableMessages))
    {
        outgoingMessage = (GekkotaOutgoingMessage *) iterator;
        iterator = gekkota_list_next(iterator);

        if (gekkota_time_get_lag(
                xudp->currentTime,
                outgoingMessage->sentTime) < outgoingMessage->roundTripTimeout)
            continue;

        if (client->earliestTimeout == 0 || gekkota_time_compare(
                    outgoingMessage->sentTime, client->earliestTimeout) < 0)
            client->earliestTimeout = outgoingMessage->sentTime;

        if (client->earliestTimeout != 0)
        {
            uint32_t timeLag = (uint32_t) gekkota_time_get_lag(
                    (time_t) xudp->currentTime,
                    (time_t) client->earliestTimeout);

            if ((timeLag >= GEKKOTA_XUDP_CLIENT_MAX_TIMEOUT ||
                    (outgoingMessage->roundTripTimeout >= outgoingMessage->roundTripTimeoutLimit &&
                    timeLag >= GEKKOTA_XUDP_CLIENT_MIN_TIMEOUT)))
                return _gekkota_xudp_notify_disconnect(xudp, client, event) != 0 ? -1 : 1;
        }

        if (outgoingMessage->packet != NULL)
            client->reliableDataInTransit -= outgoingMessage->fragmentLength;

        outgoingMessage->roundTripTimeout *= 2;

        gekkota_list_insert(
                gekkota_list_head(&client->outgoingReliableMessages),
                gekkota_list_remove(&outgoingMessage->listNode));

        if (iterator == gekkota_list_head(&client->sentReliableMessages) &&
                !gekkota_list_is_empty(&client->sentReliableMessages))
        {
            outgoingMessage = (GekkotaOutgoingMessage *) iterator;
            client->nextTimeout = outgoingMessage->sentTime + outgoingMessage->roundTripTimeout;
        }
    }

    return 0;
}

static int32_t
_gekkota_xudp_dispatch(
        GekkotaXudp *restrict xudp,
        GekkotaEvent **restrict event)
{
    GekkotaXudpClient *client;
    GekkotaChannel *channel;
    GekkotaPacket *packet;
    GekkotaIPEndPoint *remoteEndPoint;

    client = xudp->lastServicedClient;
    
    do
    {
        ++client;

        if (client >= &xudp->clients[xudp->clientCount])
            client = xudp->clients;

        switch (client->state)
        {
            case GEKKOTA_CLIENT_STATE_CONNECTION_PENDING:
            case GEKKOTA_CLIENT_STATE_CONNECTION_SUCCEEDED:
                client->state = GEKKOTA_CLIENT_STATE_CONNECTED;

                return ((*event = _gekkota_event_new(
                        GEKKOTA_EVENT_TYPE_CONNECT, client,
                        0, NULL,
                        client->remoteEndPoint, TRUE)) == NULL) ? -1 : 1;

            case GEKKOTA_CLIENT_STATE_ZOMBIE:
                xudp->reconfigureBandwidth = TRUE;
                gekkota_xudpclient_destroy(client);
                xudp->lastServicedClient = client;

                return ((*event = _gekkota_event_new(
                        GEKKOTA_EVENT_TYPE_DISCONNECT, client,
                        0, NULL,
                        client->remoteEndPoint, TRUE)) == NULL) ? -1 : 1;
        }

        if (client->state != GEKKOTA_CLIENT_STATE_CONNECTED)
            continue;

        for (channel = client->channels;
                channel < &client->channels[client->channelCount];
                channel++)
        {
            if (gekkota_list_is_empty(&channel->incomingReliableMessages) &&
                    gekkota_list_is_empty(&channel->incomingUnreliableMessages))
                continue;

            switch (gekkota_xudpclient_receive(
                    client, (uint8_t) (channel - client->channels),
                    &packet, &remoteEndPoint))
            {
                case -1:    return -1;  /* error */
                case 0:     continue;   /* no packet received */
                case 1:     break;      /* packet received */
            }

            *event = _gekkota_event_new(
                    GEKKOTA_EVENT_TYPE_RECEIVE, client,
                    (uint8_t) (channel - client->channels),
                    packet, remoteEndPoint, FALSE);

            if (*event == NULL)
            {
                gekkota_packet_destroy(packet);
                gekkota_ipendpoint_destroy(remoteEndPoint);

                return -1;
            }

            xudp->lastServicedClient = client;
            return 1;
        }
    } while (client != xudp->lastServicedClient);

    return 0;
}

static GekkotaXudpMessageType
_gekkota_xudp_dispose_acknowledged_message(
        GekkotaXudpClient *restrict client,
        uint16_t sequenceNumber,
        uint8_t channelId)
{
    GekkotaListIterator iterator;
    GekkotaOutgoingMessage *outgoingMessage;
    GekkotaXudpMessageType messageType;

    for (iterator = gekkota_list_head(&client->sentReliableMessages);
            iterator != gekkota_list_tail(&client->sentReliableMessages);
            iterator = gekkota_list_next(iterator))
    {
        outgoingMessage = (GekkotaOutgoingMessage *) iterator;

        if (outgoingMessage->reliableSequenceNumber == sequenceNumber &&
                outgoingMessage->message.header.channelId == channelId)
            break;
    }

    if (iterator == gekkota_list_tail(&client->sentReliableMessages))
        return GEKKOTA_XUDP_MESSAGE_TYPE_UNDEFINED;

    messageType = outgoingMessage->message.header.messageType;
    gekkota_list_remove(&outgoingMessage->listNode);

    if (outgoingMessage->packet != NULL)
    {
        client->reliableDataInTransit -= outgoingMessage->fragmentLength;
        gekkota_packet_destroy(outgoingMessage->packet);
    }

    gekkota_memory_free(outgoingMessage);

    if (gekkota_list_is_empty(&client->sentReliableMessages))
        return messageType;

    outgoingMessage = (GekkotaOutgoingMessage *)
        gekkota_list_first(&client->sentReliableMessages);

    client->nextTimeout = outgoingMessage->sentTime
            + outgoingMessage->roundTripTimeout;

    return messageType;
}

static int32_t
_gekkota_xudp_get_message_handler_args(
        GekkotaXudp *xudp,
        GekkotaXudpMessageHandlerArgs *args)
{
    size_t messageSize;
    GekkotaXudpMessageType messageType;
    GekkotaXudpMessage *message;

    if (args->header == NULL)
    {
        /*
         * First call: initialize [args].
         */

        GekkotaXudpHeader *header;
        GekkotaXudpClient *client;
        size_t headerSize;
        uint16_t clientId;
        uint8_t headerFlags;

        if (xudp->receivedDataLength < sizeof(GekkotaXudpHeader))
            /*
             * Not an XUDP packet.
             */
            return 0;

        header = (GekkotaXudpHeader *) xudp->receivedData;

        /*
         * The protocol version takes just 6 bits;
         * the first 2 MSB contain the header flags.
         */
        headerFlags = header->version & GEKKOTA_XUDP_HEADER_FLAG_MASK;

        if (header->protocolId != xudp->protocolId)
            /*
             * Not an XUDP packet.
             */
            return 0;

        if ((header->version >> 2) > GEKKOTA_XUDP_VERSION)
            /*
             * XUDP version might not be compatible.
             */
            return 0;

        clientId = gekkota_net_to_host_16(header->clientId);

        if (clientId == 0xFFFF &&
                !gekkota_bit_isset(headerFlags, GEKKOTA_XUDP_HEADER_FLAG_MULTICAST))
            /*
             * The received data contains a unicast connect message.
             */
            client = NULL;
        else if (clientId < xudp->clientCount ||
                gekkota_bit_isset(headerFlags, GEKKOTA_XUDP_HEADER_FLAG_MULTICAST))
        {
            if (gekkota_bit_isset(headerFlags, GEKKOTA_XUDP_HEADER_FLAG_MULTICAST))
            {
                /*
                 * The received data contains a message targeting a multicast
                 * group.
                 */
                for (client = xudp->clients;
                        client < &xudp->clients[xudp->clientCount];
                        client++)
                    if (client->isMulticastGroupMember &&
                            client->remoteClientId == clientId)
                        break;

                if (client >= &xudp->clients[xudp->clientCount])
                    /*
                     * Multicast client not found.
                     */
                    return 0;
            }
            else
            {
                /*
                 * The received data contains a message targeting a connected
                 * unicast client.
                 */
                client = &xudp->clients[clientId];

                if (client->state == GEKKOTA_CLIENT_STATE_DISCONNECTED ||
                        client->state == GEKKOTA_CLIENT_STATE_ZOMBIE ||
                        !gekkota_ipaddress_equals(
                            xudp->remoteEndPoint->address,
                            client->remoteEndPoint->address))
                    return 0;
            }

#ifdef CRC32_ENABLED
            {
                uint32_t checksum = header->sessionId;
                GekkotaBuffer buffer;

                header->sessionId = client->sessionId;

                buffer.data = xudp->receivedData;
                buffer.length = xudp->receivedDataLength;

                if (gekkota_crc32_calculate(&buffer, 1) != checksum)
                {
                    errno = GEKKOTA_ERROR_MESSAGE_CORRUPTED;
                    return -1;
                }
            }
#else
            if (header->sessionId != client->sessionId)
                return 0;
#endif /* CRC32_ENABLED */

            client->incomingDataTotal += (uint32_t) xudp->receivedDataLength;
        }
        else
        {
            /*
             * The received data cannot be assigned to any client.
             */
            return 0;
        }

        headerSize = gekkota_bit_isset(headerFlags, GEKKOTA_XUDP_HEADER_FLAG_SENT_TIME)
            ? sizeof(GekkotaXudpHeader)
            : (size_t) &((GekkotaXudpHeader *) 0)->sentTime;

        args->header = header;
        args->client = client;
        args->data = xudp->receivedData + headerSize;
    }

    /*
     * Get the next message and check it.
     */

    message = (GekkotaXudpMessage *) args->data;

    if (args->data + sizeof(GekkotaXudpMessageHeader) > &xudp->receivedData[
            xudp->receivedDataLength])
        return 0;

    messageType = message->header.messageType;

    if (messageType == GEKKOTA_XUDP_MESSAGE_TYPE_UNDEFINED ||
            messageType >= sizeof(messageSizes) / sizeof(0))
        return 0;

    if (args->client == NULL && messageType != GEKKOTA_XUDP_MESSAGE_TYPE_CONNECT)
        /*
         * Non-connect messages cannot have client id 0xFFFF.
         */
        return 0;

    messageSize = messageSizes[messageType];

    if (args->data + messageSize > &xudp->receivedData[xudp->receivedDataLength])
        return 0;

    args->data += messageSize;

    message->header.sequenceNumber = gekkota_net_to_host_16(
            message->header.sequenceNumber);

    args->message = message;
    return 1;
}

static int32_t
_gekkota_xudp_notify_connect(
        GekkotaXudp *restrict xudp,
        GekkotaXudpClient *restrict client,
        GekkotaEvent **restrict event)
{
    xudp->reconfigureBandwidth = TRUE;

    if (event == NULL)
        client->state = client->state == GEKKOTA_CLIENT_STATE_CONNECTING
            ? GEKKOTA_CLIENT_STATE_CONNECTION_SUCCEEDED
            : GEKKOTA_CLIENT_STATE_CONNECTION_PENDING;
    else
    {
        if ((*event = _gekkota_event_new(
                GEKKOTA_EVENT_TYPE_CONNECT, client,
                0, NULL,
                client->remoteEndPoint, TRUE)) == NULL)
            return -1;

        client->state = GEKKOTA_CLIENT_STATE_CONNECTED;
    }

    return 0;
}

static int32_t
_gekkota_xudp_notify_disconnect(
        GekkotaXudp *restrict xudp,
        GekkotaXudpClient *client,
        GekkotaEvent **restrict event)
{
    if (client->state >= GEKKOTA_CLIENT_STATE_CONNECTION_PENDING)
        xudp->reconfigureBandwidth = TRUE;

    if (client->state != GEKKOTA_CLIENT_STATE_CONNECTING &&
            client->state < GEKKOTA_CLIENT_STATE_CONNECTION_SUCCEEDED)
        gekkota_xudpclient_destroy(client);
    else if (event == NULL)
        client->state = GEKKOTA_CLIENT_STATE_ZOMBIE;
    else
    {
        *event = _gekkota_event_new(
                GEKKOTA_EVENT_TYPE_DISCONNECT, client,
                0, NULL,
                client->remoteEndPoint, TRUE);
        gekkota_xudpclient_destroy(client);

        if (*event == NULL)
            return -1;
    }

    return 0;
}

static int32_t
_gekkota_xudp_on_acknowledge_message(
        GekkotaXudp *xudp,
        GekkotaXudpMessageHandlerArgs *args,
        GekkotaEvent **event)
{
    uint32_t roundTripTime, sentTime;
    uint16_t sequenceNumber;
    GekkotaXudpMessageType messageType;

    GekkotaXudpClient *client = args->client;
    GekkotaXudpMessage *message = args->message;

    sentTime = gekkota_net_to_host_16(message->acknowledge.sentTime);
    sentTime |= xudp->currentTime & 0xFFFF0000;

    if ((sentTime & 0x8000) > (xudp->currentTime & 0x8000))
        sentTime -= 0x10000;

    if (gekkota_time_compare(xudp->currentTime, sentTime) < 0)
        return 1;

    client->lastReceiveTime = xudp->currentTime;
    client->earliestTimeout = 0;

    roundTripTime = (uint32_t) gekkota_time_get_lag(xudp->currentTime, sentTime);

    _gekkota_xudpclient_throttle(client, roundTripTime);

    client->roundTripTimeVariance -= client->roundTripTimeVariance / 4;

    if (roundTripTime >= client->roundTripTime)
    {
        client->roundTripTime += (roundTripTime - client->roundTripTime) / 8;
        client->roundTripTimeVariance += (roundTripTime - client->roundTripTime) / 4;
    }
    else
    {
        client->roundTripTime -= (client->roundTripTime - roundTripTime) / 8;
        client->roundTripTimeVariance += (client->roundTripTime - roundTripTime) / 4;
    }

    if (client->roundTripTime < client->lowestRoundTripTime)
        client->lowestRoundTripTime = client->roundTripTime;

    if (client->roundTripTimeVariance > client->highestRoundTripTimeVariance)
        client->highestRoundTripTimeVariance = client->roundTripTimeVariance;

    if (client->packetThrottleEpoch == 0 ||
            gekkota_time_get_lag(
                xudp->currentTime,
                client->packetThrottleEpoch) >= client->packetThrottleInterval)
    {
        client->lastRoundTripTime = client->lowestRoundTripTime;
        client->lastRoundTripTimeVariance = client->highestRoundTripTimeVariance;
        client->lowestRoundTripTime = client->roundTripTime;
        client->highestRoundTripTimeVariance = client->roundTripTimeVariance;
        client->packetThrottleEpoch = xudp->currentTime;
    }

    sequenceNumber = gekkota_net_to_host_16(message->acknowledge.sequenceNumber);

    messageType = _gekkota_xudp_dispose_acknowledged_message(
            client, sequenceNumber, message->header.channelId);

    switch (client->state)
    {
        case GEKKOTA_CLIENT_STATE_ACKNOWLEDGING_CONNECT:
            if (messageType != GEKKOTA_XUDP_MESSAGE_TYPE_VALIDATE_CONNECT)
                return 0;

            if (_gekkota_xudp_notify_connect(xudp, client, event) != 0)
                return -1;

            break;

        case GEKKOTA_CLIENT_STATE_DISCONNECTING:
            if (messageType != GEKKOTA_XUDP_MESSAGE_TYPE_DISCONNECT)
                return 0;

            if (_gekkota_xudp_notify_disconnect(xudp, client, event) != 0)
                return -1;

            break;

        case GEKKOTA_CLIENT_STATE_DELAYING_DISCONNECT:
            if (gekkota_list_is_empty(&client->outgoingReliableMessages) &&
                    gekkota_list_is_empty(&client->outgoingUnreliableMessages) &&
                    gekkota_list_is_empty(&client->sentReliableMessages))
                if (_gekkota_xudpclient_close_gracefully(client) != 0)
                    return -1;

            break;
    }

    return 1;
}

static int32_t
_gekkota_xudp_on_connect_message(
        GekkotaXudp *xudp,
        GekkotaXudpMessageHandlerArgs *args,
        GekkotaEvent **event)
{
    uint16_t mtu;
    uint32_t windowSize;
    GekkotaChannel *channel;
    uint8_t channelCount;
    GekkotaXudpMessage validateConnectMessage;
    GekkotaXudpClient *client;

    GekkotaXudpHeader *header = args->header;
    GekkotaXudpMessage *message = args->message;

#ifdef CRC32_ENABLED
    uint32_t checksum = header->sessionId;
    GekkotaBuffer buffer;

    message->header.sequenceNumber = gekkota_host_to_net_16(
            message->header.sequenceNumber);

    header->sessionId = message->connect.sessionId;

    buffer.data = xudp->receivedData;
    buffer.length = xudp->receivedDataLength;

    if (gekkota_crc32_calculate(&buffer, 1) != checksum)
    {
        errno = GEKKOTA_ERROR_MESSAGE_CORRUPTED;
        return -1;
    }

    message->header.sequenceNumber = gekkota_net_to_host_16(
            message->header.sequenceNumber);
#endif /* CRC32_ENAMBED */
 
    channelCount = message->connect.channelCount;

    for (client = xudp->clients;
            client < &xudp->clients[xudp->clientCount];
            client++)
    {
        if (client->state != GEKKOTA_CLIENT_STATE_DISCONNECTED &&
                gekkota_ipendpoint_equals(xudp->remoteEndPoint, client->remoteEndPoint) &&
                client->sessionId == message->connect.sessionId)
            /*
             * Already connected.
             */
            return 0;
    }

    for (client = xudp->clients;
            client < &xudp->clients[xudp->clientCount];
            client++)
        if (client->state == GEKKOTA_CLIENT_STATE_DISCONNECTED)
            break;

    if (client >= &xudp->clients[xudp->clientCount])
        /*
         * Connection refused; max client already reached.
         */
        return 0;

    if ((client->channels = gekkota_memory_alloc(
            sizeof(GekkotaChannel) * channelCount, TRUE)) == NULL)
        return -1;

    client->sessionId = message->connect.sessionId;
    client->remoteClientId = gekkota_net_to_host_16(message->connect.clientId);
    client->remoteEndPoint = gekkota_ipendpoint_new_0(xudp->remoteEndPoint, FALSE);
    client->isMulticastGroupMember = FALSE;
    client->state = GEKKOTA_CLIENT_STATE_ACKNOWLEDGING_CONNECT;
    client->channelCount = channelCount;
    client->incomingBandwidth = gekkota_net_to_host_32(message->connect.incomingBandwidth);
    client->outgoingBandwidth = gekkota_net_to_host_32(message->connect.outgoingBandwidth);
    client->packetThrottleInterval = gekkota_net_to_host_32(message->connect.throttleInterval);
    client->packetThrottleAcceleration = gekkota_net_to_host_32(message->connect.throttleAcceleration);
    client->packetThrottleDeceleration = gekkota_net_to_host_32(message->connect.throttleDeceleration);
    client->compressionLevel = message->connect.compressionLevel;

    for (channel = client->channels;
            channel < &client->channels[channelCount];
            channel++)
    {
        gekkota_list_clear(&channel->incomingReliableMessages);
        gekkota_list_clear(&channel->incomingUnreliableMessages);
    }

    mtu = gekkota_net_to_host_16(message->connect.mtu);

    if (mtu < GEKKOTA_XUDP_MIN_MTU)
        mtu = GEKKOTA_XUDP_MIN_MTU;
    else if (mtu > GEKKOTA_XUDP_MAX_MTU)
        mtu = GEKKOTA_XUDP_MAX_MTU;

    client->mtu = mtu;

    if (xudp->outgoingBandwidth == 0 && client->incomingBandwidth == 0)
        client->windowSize = GEKKOTA_XUDP_MAX_WINDOW_SIZE;
    else
    {
        windowSize = (gekkota_utils_min(xudp->outgoingBandwidth, client->incomingBandwidth)
                / GEKKOTA_XUDP_CLIENT_WINDOW_SIZE_SCALE) * GEKKOTA_XUDP_MIN_WINDOW_SIZE;

        if (windowSize < GEKKOTA_XUDP_MIN_WINDOW_SIZE)
            windowSize = GEKKOTA_XUDP_MIN_WINDOW_SIZE;
        else if (windowSize > GEKKOTA_XUDP_MAX_WINDOW_SIZE)
            windowSize = GEKKOTA_XUDP_MAX_WINDOW_SIZE;

        client->windowSize = windowSize;
    }

    if (xudp->incomingBandwidth == 0)
        windowSize = GEKKOTA_XUDP_MAX_WINDOW_SIZE;
    else
    {
        windowSize = (xudp->incomingBandwidth / GEKKOTA_XUDP_CLIENT_WINDOW_SIZE_SCALE)
            * GEKKOTA_XUDP_MIN_WINDOW_SIZE;

        if (windowSize > gekkota_net_to_host_32(message->connect.windowSize))
            windowSize = gekkota_net_to_host_32(message->connect.windowSize);

        if (windowSize < GEKKOTA_XUDP_MIN_WINDOW_SIZE)
            windowSize = GEKKOTA_XUDP_MIN_WINDOW_SIZE;
        else if (windowSize > GEKKOTA_XUDP_MAX_WINDOW_SIZE)
            windowSize = GEKKOTA_XUDP_MAX_WINDOW_SIZE;
    }

    /* message header */
    validateConnectMessage.header.messageType = GEKKOTA_XUDP_MESSAGE_TYPE_VALIDATE_CONNECT;
    validateConnectMessage.header.channelId = 0xFF;
    validateConnectMessage.header.flags = GEKKOTA_XUDP_MESSAGE_FLAG_ACKNOWLEDGE;

    /* message body */
    validateConnectMessage.validateConnect.clientId =
        gekkota_host_to_net_16(client->localClientId);
    validateConnectMessage.validateConnect.channelCount = channelCount;
    validateConnectMessage.validateConnect.mtu =
        gekkota_host_to_net_16(client->mtu);
    validateConnectMessage.validateConnect.windowSize =
        gekkota_host_to_net_32(windowSize);
    validateConnectMessage.validateConnect.incomingBandwidth =
        gekkota_host_to_net_32(xudp->incomingBandwidth);
    validateConnectMessage.validateConnect.outgoingBandwidth =
        gekkota_host_to_net_32(xudp->outgoingBandwidth);
    validateConnectMessage.validateConnect.throttleInterval =
        gekkota_host_to_net_32(client->packetThrottleInterval);
    validateConnectMessage.validateConnect.throttleAcceleration =
        gekkota_host_to_net_32(client->packetThrottleAcceleration);
    validateConnectMessage.validateConnect.throttleDeceleration =
        gekkota_host_to_net_32(client->packetThrottleDeceleration);
    validateConnectMessage.validateConnect.compressionLevel = client->compressionLevel;

    if (_gekkota_xudpclient_queue_outgoing_message(
            client, &validateConnectMessage, NULL, 0, 0, NULL) != 0)
    {
        gekkota_xudpclient_destroy(client);
        return -1;
    }

    args->client = client;
    return 1;
}

static int32_t
_gekkota_xudp_on_validate_connect_message(
        GekkotaXudp *xudp,
        GekkotaXudpMessageHandlerArgs *args,
        GekkotaEvent **event)
{
    uint16_t mtu;
    uint32_t windowSize;

    GekkotaXudpMessage *message = args->message;
    GekkotaXudpClient *client = args->client;

    if (client->state != GEKKOTA_CLIENT_STATE_CONNECTING)
        return 0;

    if (message->validateConnect.channelCount != client->channelCount ||
            gekkota_net_to_host_32(
                message->validateConnect.throttleInterval) != client->packetThrottleInterval ||
            gekkota_net_to_host_32(
                message->validateConnect.throttleAcceleration) != client->packetThrottleAcceleration ||
            gekkota_net_to_host_32(
                message->validateConnect.throttleDeceleration) != client->packetThrottleDeceleration ||
            message->validateConnect.compressionLevel != client->compressionLevel)
    {
        client->state = GEKKOTA_CLIENT_STATE_ZOMBIE;
        return 0;
    }

    _gekkota_xudp_dispose_acknowledged_message(client, 1, 0xFF);

    client->remoteClientId =
        gekkota_net_to_host_16(message->validateConnect.clientId);

    mtu = gekkota_net_to_host_16(message->validateConnect.mtu);

    if (mtu < GEKKOTA_XUDP_MIN_MTU)
        mtu = GEKKOTA_XUDP_MIN_MTU;
    else if (mtu > GEKKOTA_XUDP_MAX_MTU)
        mtu = GEKKOTA_XUDP_MAX_MTU;

    if (mtu < client->mtu)
        client->mtu = mtu;

    windowSize = gekkota_net_to_host_32(message->validateConnect.windowSize);

    if (windowSize < GEKKOTA_XUDP_MIN_WINDOW_SIZE)
        windowSize = GEKKOTA_XUDP_MIN_WINDOW_SIZE;

    if (windowSize > GEKKOTA_XUDP_MAX_WINDOW_SIZE)
        windowSize = GEKKOTA_XUDP_MAX_WINDOW_SIZE;

    if (windowSize < client->windowSize)
        client->windowSize = windowSize;

    client->incomingBandwidth =
        gekkota_net_to_host_32(message->validateConnect.incomingBandwidth);
    client->outgoingBandwidth =
        gekkota_net_to_host_32(message->validateConnect.outgoingBandwidth);

    return _gekkota_xudp_notify_connect(xudp, client, event) != 0 ? -1 : 1;
}

static int32_t
_gekkota_xudp_on_disconnect_message(
        GekkotaXudp *xudp,
        GekkotaXudpMessageHandlerArgs *args,
        GekkotaEvent **event)
{
    GekkotaXudpMessage * message = args->message;
    GekkotaXudpClient *client = args->client;

    _gekkota_xudpclient_clear_message_queues(client);

    if (client->state == GEKKOTA_CLIENT_STATE_CONNECTION_SUCCEEDED)
        client->state = GEKKOTA_CLIENT_STATE_ZOMBIE;
    else if (client->state != GEKKOTA_CLIENT_STATE_CONNECTED &&
        client->state != GEKKOTA_CLIENT_STATE_DELAYING_DISCONNECT)
    {
        if (client->state == GEKKOTA_CLIENT_STATE_CONNECTION_PENDING)
            xudp->reconfigureBandwidth = TRUE;

        gekkota_xudpclient_destroy(client);
    }
    else if (gekkota_bit_isset(
            message->header.flags,
            GEKKOTA_XUDP_MESSAGE_FLAG_ACKNOWLEDGE))
        client->state = GEKKOTA_CLIENT_STATE_ACKNOWLEDGING_DISCONNECT;
    else
        client->state = GEKKOTA_CLIENT_STATE_ZOMBIE;

    return 1;
}

static int32_t
_gekkota_xudp_on_join_multicast_group_message(
        GekkotaXudp *xudp,
        GekkotaXudpMessageHandlerArgs *args,
        GekkotaEvent **event)
{
    if (event != NULL)
    {
        /*
         * [args->client] points to the client instance that joined the
         * multicast group.
         */
        if ((*event = _gekkota_event_new(
                GEKKOTA_EVENT_TYPE_JOIN_MULTICAST_GROUP, args->client,
                0, NULL,
                xudp->remoteEndPoint, TRUE)) == NULL)
            return -1;
    }

    return 1;
}

static int32_t
_gekkota_xudp_on_leave_multicast_group_message(
        GekkotaXudp *xudp,
        GekkotaXudpMessageHandlerArgs *args,
        GekkotaEvent **event)
{
    if (event != NULL)
    {
        /*
         * [args->client] points to the client instance that joined the
         * multicast group.
         */
        if ((*event = _gekkota_event_new(
                GEKKOTA_EVENT_TYPE_LEAVE_MULTICAST_GROUP, args->client,
                0, NULL,
                xudp->remoteEndPoint, TRUE)) == NULL)
            return -1;
    }

    return 1;
}

static int32_t
_gekkota_xudp_on_ping_message(
        GekkotaXudp *xudp,
        GekkotaXudpMessageHandlerArgs *args,
        GekkotaEvent **event)
{
    return 1;
}

static int32_t
_gekkota_xudp_on_reliable_data_message(
        GekkotaXudp *xudp,
        GekkotaXudpMessageHandlerArgs *args,
        GekkotaEvent **event)
{
    int32_t rc;
    size_t dataLength;
    GekkotaBuffer buffer;
    GekkotaPacket *packet;
    GekkotaPacketFlag flags = GEKKOTA_PACKET_FLAG_RELIABLE;

    GekkotaXudpMessage *message = args->message;
    GekkotaXudpClient *client = args->client;

    if (message->header.channelId >= client->channelCount ||
            (client->state != GEKKOTA_CLIENT_STATE_CONNECTED &&
            client->state != GEKKOTA_CLIENT_STATE_DELAYING_DISCONNECT))
        return -1;

    /*
     * Move to next message, if any.
     */
    dataLength = gekkota_net_to_host_16(message->reliableData.length);
    args->data += dataLength;

    if (args->data > &xudp->receivedData[xudp->receivedDataLength])
        return 0;

    buffer.data = (byte_t *) message + sizeof(GekkotaXudpReliableDataMessage);
    buffer.length = dataLength;

    if (gekkota_bit_isset(
            message->reliableData.header.flags,
            GEKKOTA_XUDP_MESSAGE_FLAG_COMPRESSED))
        gekkota_bit_set(flags, GEKKOTA_PACKET_FLAG_COMPRESSED);

    if ((packet = gekkota_packet_new_2(&buffer, flags)) == NULL)
        return -1;

    rc = _gekkota_xudpclient_queue_incoming_message(
            client, message, packet, 0, xudp->remoteEndPoint, NULL);

    gekkota_packet_destroy(packet);
    return rc;
}

static int32_t
_gekkota_xudp_on_unreliable_data_message(
        GekkotaXudp *xudp,
        GekkotaXudpMessageHandlerArgs *args,
        GekkotaEvent **event)
{
    int32_t rc;
    size_t dataLength;
    GekkotaBuffer buffer;
    GekkotaPacket *packet;
    GekkotaPacketFlag flags = GEKKOTA_PACKET_FLAG_NONE;

    GekkotaXudpMessage *message = args->message;
    GekkotaXudpClient *client = args->client;

    if (message->header.channelId >= client->channelCount ||
            (client->state != GEKKOTA_CLIENT_STATE_CONNECTED &&
             client->state != GEKKOTA_CLIENT_STATE_DELAYING_DISCONNECT))
        return 0;

    /*
     * Move to next message, if any.
     */
    dataLength = gekkota_net_to_host_16(message->unreliableData.length);
    args->data += dataLength;

    if (args->data > &xudp->receivedData[xudp->receivedDataLength])
        return 0;

    buffer.data = (byte_t *) message + sizeof(GekkotaXudpUnreliableDataMessage);
    buffer.length = dataLength;

    if (gekkota_bit_isset(
            message->unreliableData.header.flags,
            GEKKOTA_XUDP_MESSAGE_FLAG_COMPRESSED))
        flags = GEKKOTA_PACKET_FLAG_COMPRESSED;

    if ((packet = gekkota_packet_new_2(&buffer, flags)) == NULL)
        return -1;

    rc = _gekkota_xudpclient_queue_incoming_message(
            client, message, packet, 0, xudp->remoteEndPoint, NULL);

    gekkota_packet_destroy(packet);
    return rc;
}

static int32_t
_gekkota_xudp_on_unsequenced_data_message(
        GekkotaXudp *xudp,
        GekkotaXudpMessageHandlerArgs *args,
        GekkotaEvent **event)
{
    int32_t rc;
    size_t dataLength;
    uint16_t group, index;
    GekkotaBuffer buffer;
    GekkotaPacket *packet;
    GekkotaPacketFlag flags = GEKKOTA_PACKET_FLAG_UNSEQUENCED;

    GekkotaXudpMessage *message = args->message;
    GekkotaXudpClient *client = args->client;

    if (message->header.channelId >= client->channelCount ||
            (client->state != GEKKOTA_CLIENT_STATE_CONNECTED &&
             client->state != GEKKOTA_CLIENT_STATE_DELAYING_DISCONNECT))
        return 0;

    /*
     * Move to next message, if any.
     */
    dataLength = gekkota_net_to_host_16(message->unsequencedData.length);
    args->data += dataLength;

    if (args->data > &xudp->receivedData[xudp->receivedDataLength])
        return 0;

    group = gekkota_net_to_host_16(message->unsequencedData.group);
    index = group % GEKKOTA_XUDP_CLIENT_UNSEQUENCED_WINDOW_SIZE;

    if (group >= client->incomingUnsequencedGroup
            + GEKKOTA_XUDP_CLIENT_UNSEQUENCED_WINDOW_SIZE ||
            client->incomingUnsequencedGroup >= 0xF000 && group < 0x1000)
    {
        client->incomingUnsequencedGroup = group - index;
        memset(client->unsequencedWindow, 0x00, sizeof(client->unsequencedWindow));
    }
    else if (group < client->incomingUnsequencedGroup ||
            client->unsequencedWindow[index / 32] & (1 << (index % 32)))
        return 1;

    client->unsequencedWindow[index / 32] |= 1 << (index % 32);

    buffer.data = (byte_t *) message + sizeof(GekkotaXudpUnsequencedDataMessage);
    buffer.length = dataLength;

    if (gekkota_bit_isset(
            message->unsequencedData.header.flags,
            GEKKOTA_XUDP_MESSAGE_FLAG_COMPRESSED))
        gekkota_bit_set(flags, GEKKOTA_PACKET_FLAG_COMPRESSED);

    if ((packet = gekkota_packet_new_2(&buffer, flags)) == NULL)
        return -1;

    rc = _gekkota_xudpclient_queue_incoming_message(
            client, message, packet, 0, xudp->remoteEndPoint, NULL);

    gekkota_packet_destroy(packet);
    return rc;
}

static int32_t
_gekkota_xudp_on_data_fragment_message(
        GekkotaXudp *xudp,
        GekkotaXudpMessageHandlerArgs *args,
        GekkotaEvent **event)
{
    uint32_t fragmentNumber, fragmentCount, fragmentOffset, fragmentLength;
    uint32_t startSequenceNumber, totalLength;
    GekkotaListIterator iterator;
    GekkotaChannel *channel;
    GekkotaIncomingMessage *startMessage;

    GekkotaXudpMessage *message = args->message;
    GekkotaXudpClient *client = args->client;

    if (message->header.channelId >= client->channelCount ||
            (client->state != GEKKOTA_CLIENT_STATE_CONNECTED &&
             client->state != GEKKOTA_CLIENT_STATE_DELAYING_DISCONNECT))
        return 0;

    /*
     * Move to next message, if any.
     */
    fragmentLength = gekkota_net_to_host_16(message->dataFragment.length);
    args->data += fragmentLength;

    if (args->data > &xudp->receivedData[xudp->receivedDataLength])
        return 0;

    channel = &client->channels[message->header.channelId];
    startSequenceNumber =
        gekkota_net_to_host_16(message->dataFragment.startSequenceNumber);

    /*
     * If the sequence numbers received so far are all at the high end of the
     * 16-bit range, but a just received sequence number is at the low end,
     * assume it was a sequence number that wrapped around back to the low end
     * again and treat it as a number greater than 2e16, being values greater
     * than or equal to 0xF000 the high end, and values less than 0x1000 the
     * low end.
     */

    if (channel->incomingReliableSequenceNumber >= 0xF000 &&
            startSequenceNumber < 0x1000)
        startSequenceNumber += 0x10000;

    /*
     * Check if even after adjustment the sequence number is less than the ones
     * already received; it is almost impossible to have anywhere near to 2e16
     * packets in flight at any given time.
     */

    if (startSequenceNumber < channel->incomingReliableSequenceNumber ||
            (channel->incomingReliableSequenceNumber < 0x1000 &&
            (startSequenceNumber & 0x0000FFFF) >= 0xF000))
        return 1;

    fragmentNumber = gekkota_net_to_host_32(message->dataFragment.fragmentNumber);
    fragmentCount = gekkota_net_to_host_32(message->dataFragment.fragmentCount);
    fragmentOffset = gekkota_net_to_host_32(message->dataFragment.fragmentOffset);
    totalLength = gekkota_net_to_host_32(message->dataFragment.totalLength);

    if (fragmentOffset >= totalLength ||
        fragmentOffset + fragmentLength > totalLength ||
        fragmentNumber >= fragmentCount)
        return 0;

    /*
     * Search for already received fragments, if any.
     */
    for (iterator = gekkota_list_previous(gekkota_list_tail(&channel->incomingReliableMessages));
            iterator != gekkota_list_tail(&channel->incomingReliableMessages);
            iterator = gekkota_list_previous(iterator))
    {
        startMessage = (GekkotaIncomingMessage *) iterator;

        if (startMessage->message.header.messageType == GEKKOTA_XUDP_MESSAGE_TYPE_DATA_FRAGMENT &&
                startMessage->message.dataFragment.startSequenceNumber ==
                (startSequenceNumber & 0x0000FFFF))
            break;
    }

    if (iterator == gekkota_list_tail(&channel->incomingReliableMessages))
    {
        /*
         * First fragment: allocate enough room to reassemble the original
         * packet.
         */

        int32_t rc;
        GekkotaXudpMessage newMessage = *message;
        GekkotaPacket *packet;
        GekkotaPacketFlag flags = GEKKOTA_PACKET_FLAG_RELIABLE;

        newMessage.header.sequenceNumber = (uint16_t) (startSequenceNumber & 0x0000FFFF);
        newMessage.dataFragment.startSequenceNumber = (uint16_t) (startSequenceNumber & 0x0000FFFF);
        newMessage.dataFragment.length = (uint16_t) fragmentLength;
        newMessage.dataFragment.fragmentNumber = fragmentNumber;
        newMessage.dataFragment.fragmentCount = fragmentCount;
        newMessage.dataFragment.fragmentOffset = fragmentOffset;
        newMessage.dataFragment.totalLength = totalLength;

        if (gekkota_bit_isset(
                message->dataFragment.header.flags,
                GEKKOTA_XUDP_MESSAGE_FLAG_COMPRESSED))
            gekkota_bit_set(flags, GEKKOTA_PACKET_FLAG_COMPRESSED);
        
        if ((packet = gekkota_packet_new_1(totalLength, flags)) == NULL)
            return -1;

        rc = _gekkota_xudpclient_queue_incoming_message(
                client, &newMessage, packet, fragmentCount, xudp->remoteEndPoint, &startMessage);

        gekkota_packet_destroy(packet);
        if (rc < 0) return -1;
    }
    else if (totalLength != startMessage->packet->data.length ||
            fragmentCount != startMessage->fragmentCount)
        return 0;

    if ((startMessage->fragments[fragmentNumber / 32] & (1 << (fragmentNumber % 32))) == 0)
    {
        --startMessage->fragmentsRemaining;
        startMessage->fragments[fragmentNumber / 32] |= (1 << (fragmentNumber % 32));

        if (fragmentOffset + fragmentLength > startMessage->packet->data.length)
            fragmentLength = (uint16_t) (startMessage->packet->data.length - fragmentOffset);

        memcpy((byte_t *) startMessage->packet->data.data + fragmentOffset,
                (byte_t *) message + sizeof(GekkotaXudpDataFragmentMessage),
                fragmentLength);
    }

    return 1;
}

static int32_t
_gekkota_xudp_on_configure_bandwidth_message(
        GekkotaXudp *xudp,
        GekkotaXudpMessageHandlerArgs *args,
        GekkotaEvent **event)
{
    GekkotaXudpMessage *message = args->message;
    GekkotaXudpClient *client = args->client;

    client->incomingBandwidth = gekkota_net_to_host_32(
            message->configureBandwidth.incomingBandwidth);
    client->outgoingBandwidth = gekkota_net_to_host_32(
            message->configureBandwidth.outgoingBandwidth);

    if (client->incomingBandwidth == 0 && xudp->outgoingBandwidth == 0)
        client->windowSize = GEKKOTA_XUDP_MAX_WINDOW_SIZE;
    else
    {
        uint32_t windowSize =
            (gekkota_utils_min(client->incomingBandwidth, xudp->outgoingBandwidth)
                / GEKKOTA_XUDP_CLIENT_WINDOW_SIZE_SCALE)
                * GEKKOTA_XUDP_MIN_WINDOW_SIZE;

        if (windowSize < GEKKOTA_XUDP_MIN_WINDOW_SIZE)
            windowSize = GEKKOTA_XUDP_MIN_WINDOW_SIZE;
        else if (windowSize > GEKKOTA_XUDP_MAX_WINDOW_SIZE)
            windowSize = GEKKOTA_XUDP_MAX_WINDOW_SIZE;

        client->windowSize = windowSize;
    }

    return 1;
}

static int32_t
_gekkota_xudp_on_configure_throttle_message(
        GekkotaXudp *xudp,
        GekkotaXudpMessageHandlerArgs *args,
        GekkotaEvent **event)
{
    GekkotaXudpMessage *message = args->message;
    GekkotaXudpClient *client = args->client;

    client->packetThrottleInterval =  gekkota_net_to_host_32(
            message->configureThrottle.throttleInterval);
    client->packetThrottleAcceleration = gekkota_net_to_host_32(
            message->configureThrottle.throttleAcceleration);
    client->packetThrottleDeceleration = gekkota_net_to_host_32(
            message->configureThrottle.throttleDeceleration);

    return 1;
}
static int32_t
_gekkota_xudp_receive(
        GekkotaXudp *xudp,
        GekkotaEvent **event)
{
    GekkotaXudpMessageHandlerArgs args;
    GekkotaIPEndPoint *remoteEndPoint;
    GekkotaBuffer buffer;
    int32_t recv;

    if (event != NULL)
        *event = NULL;

    while (TRUE)
    {
        buffer.data = xudp->receivedData;
        buffer.length = sizeof(xudp->receivedData);

        recv = gekkota_socket_receive(
                xudp->socket,
                &buffer, 1,
                &remoteEndPoint);

        if (recv < 0)   return -1;
        if (recv == 0)  return 0;

        xudp->receivedDataLength = (uint16_t) recv;
        gekkota_ipendpoint_destroy(xudp->remoteEndPoint);
        xudp->remoteEndPoint = remoteEndPoint;

        memset(&args, 0x00, sizeof(GekkotaXudpMessageHandlerArgs));

        /*
         * The received data might contain more than one message.
         */
        while (args.data < &xudp->receivedData[xudp->receivedDataLength])
        {
            /*
             * Fill [args] with the next available message, if any.
             */
            switch (_gekkota_xudp_get_message_handler_args(xudp, &args))
            {
                case -1:    goto _gekkota_xudp_receive_error;
                case 0:     continue;   /* message discarded */
                default:    break;      /* message is valid */
            }

            /*
             * Invoke the appropriate message handler.
             */
            switch (messageHandlers[args.message->header.messageType](xudp, &args, event))
            {
                case -1:    goto _gekkota_xudp_receive_error;
                case 0:     continue;   /* message discarded */
                default:    break;      /* massage processed successfully */
            }

            /*
             * If the case, send back an acknowledgement.
             */
            switch (_gekkota_xudp_acknowledge(xudp, &args))
            {
                case -1:    goto _gekkota_xudp_receive_error;
                case 0:     continue;   /* no acknowledgement sent */
                default:    break;      /* acknowledgement sent */
            }
        }

        if (event != NULL && *event != NULL)
            return 1;
    }

_gekkota_xudp_receive_error:
    if (event != NULL && *event != NULL)
    {
        gekkota_event_destroy(*event);
        *event = NULL;
    }

    return -1;
}

static int32_t
_gekkota_xudp_send(
        GekkotaXudp *xudp,
        GekkotaEvent **event,
        bool_t checkForTimeouts)
{
    GekkotaXudpHeader header;
    GekkotaXudpClient *client;
    int32_t sent;       /* -1:  error */
                        /* 0+:  number of bytes sent */

    int32_t send = 1;   /* -1:  error */
                        /*  0:  stop sending queued messages */
                        /*  1:  continue sending queued messages */

    while (send)
    {
        for (send = 0, client = xudp->clients;
                client < &xudp->clients[xudp->clientCount];
                client++)
        {
            if (client->state == GEKKOTA_CLIENT_STATE_DISCONNECTED ||
                    client->state == GEKKOTA_CLIENT_STATE_ZOMBIE)
                continue;

            xudp->headerFlags = client->isMulticastGroupMember
                ? GEKKOTA_XUDP_HEADER_FLAG_MULTICAST
                : 0;

            xudp->messageCount = 0;
            xudp->bufferCount = 1;
            xudp->packetSize = sizeof(GekkotaXudpHeader);

            if (!gekkota_list_is_empty(&client->acknowledgements))
                if ((send = _gekkota_xudp_send_acknowledgements(xudp, client)) < 0)
                    return -1;

            if (checkForTimeouts &&
                    !gekkota_list_is_empty(&client->sentReliableMessages) &&
                    gekkota_time_compare(xudp->currentTime, client->nextTimeout) >= 0)
            {
                switch (_gekkota_xudp_check_for_timeouts(xudp, client, event))
                {
                    case -1:    return -1;  /* error */
                    case 0:     break;      /* no connection timeout occurrd */
                    case 1:     return 1;   /* connection timeout occurred */
                }
            }

            if (!gekkota_list_is_empty(&client->outgoingReliableMessages))
                if ((send = _gekkota_xudp_send_reliable(xudp, client)) < 0)
                    return -1;
            else if (gekkota_list_is_empty(&client->sentReliableMessages))
            {
                if (gekkota_time_get_lag(
                        xudp->currentTime,
                        client->lastReceiveTime) >= GEKKOTA_XUDP_CLIENT_PING_INTERVAL &&
                        client->mtu - xudp->packetSize >= sizeof(GekkotaXudpPingMessage))
                {
                    gekkota_xudpclient_ping(client);
                    if ((send = _gekkota_xudp_send_reliable(xudp, client)) < 0)
                        return -1;
                }
            }

            if (!gekkota_list_is_empty(&client->outgoingUnreliableMessages))
                if ((send = _gekkota_xudp_send_unreliable(xudp, client)) < 0)
                    return -1;

            if (xudp->messageCount == 0)
                continue;

            header.protocolId = xudp->protocolId;
            header.version = GEKKOTA_XUDP_VERSION << 2;
            gekkota_bit_set(header.version, xudp->headerFlags & GEKKOTA_XUDP_HEADER_FLAG_MASK);
            header.sessionId = client->sessionId;
            header.clientId = gekkota_host_to_net_16(client->remoteClientId);
            xudp->buffers->data = &header;

            if (gekkota_bit_isset(xudp->headerFlags, GEKKOTA_XUDP_HEADER_FLAG_SENT_TIME))
            {
                header.sentTime = gekkota_host_to_net_16((uint16_t) (xudp->currentTime & 0x0000FFFF));
                xudp->buffers->length = sizeof(GekkotaXudpHeader);
            }
            else
                xudp->buffers->length = (size_t) &((GekkotaXudpHeader *) 0)->sentTime;
 
#ifdef CRC32_ENABLED
            header.sessionId = gekkota_crc32_calculate(xudp->buffers, xudp->bufferCount);
#endif /* CRC32_ENABLED */

            sent = gekkota_socket_send(
                    xudp->socket, client->remoteEndPoint, xudp->buffers, xudp->bufferCount);

            _gekkota_xudpclient_clear_outgoing_message_queue(&client->sentUnreliableMessages);

            if (sent < 0)
                return -1;
        }
    }

    return 0;
}

static int32_t
_gekkota_xudp_send_acknowledgements(
        GekkotaXudp *restrict xudp,
        GekkotaXudpClient *restrict client)
{
    GekkotaXudpMessage *message = &xudp->messages[xudp->messageCount];
    GekkotaBuffer *buffer = &xudp->buffers[xudp->bufferCount];
    GekkotaAcknowledgement *acknowledgement;
    GekkotaListIterator iterator;
    int32_t done = 0;
  
    iterator = gekkota_list_head(&client->acknowledgements);

    while (iterator != gekkota_list_tail(&client->acknowledgements))
    {
        if (message >= &xudp->messages[sizeof(xudp->messages) / sizeof(GekkotaXudpMessage)] ||
                buffer >= &xudp->buffers[sizeof(xudp->buffers) / sizeof(GekkotaBuffer)] ||
                client->mtu - xudp->packetSize < sizeof(GekkotaXudpAcknowledgeMessage))
        {
            done = 1;
            break;
        }

        acknowledgement = (GekkotaAcknowledgement *) iterator;
        iterator = gekkota_list_next(iterator);

        buffer->data = message;
        buffer->length = sizeof(GekkotaXudpAcknowledgeMessage);

        xudp->packetSize += buffer->length;
 
        message->header.messageType = GEKKOTA_XUDP_MESSAGE_TYPE_ACKNOWLEDGE;
        message->header.channelId = acknowledgement->message.header.channelId;
        message->acknowledge.sequenceNumber = gekkota_host_to_net_16(
                acknowledgement->message.header.sequenceNumber);
        message->acknowledge.sentTime = gekkota_host_to_net_16(
                (uint16_t) acknowledgement->sentTime);

        if (acknowledgement->message.header.messageType == GEKKOTA_XUDP_MESSAGE_TYPE_DISCONNECT)
            client->state = GEKKOTA_CLIENT_STATE_ZOMBIE;

        gekkota_list_remove(&acknowledgement->listNode);
        gekkota_memory_free(acknowledgement);

        ++message;
        ++buffer;
    }

    xudp->messageCount = (uint16_t) (message - xudp->messages);
    xudp->bufferCount = (uint16_t) (buffer - xudp->buffers);

    return done;
}

static int32_t
_gekkota_xudp_send_reliable(
        GekkotaXudp *restrict xudp,
        GekkotaXudpClient *restrict client)
{
    GekkotaXudpMessage *message = &xudp->messages[xudp->messageCount];
    GekkotaBuffer *buffer = &xudp->buffers[xudp->bufferCount];
    GekkotaOutgoingMessage *outgoingMessage;
    GekkotaListIterator iterator;
    int32_t done = 0;

    iterator = gekkota_list_head(&client->outgoingReliableMessages);

    while (iterator != gekkota_list_tail(&client->outgoingReliableMessages))
    {
        size_t messageSize;

        outgoingMessage = (GekkotaOutgoingMessage *) iterator;
        messageSize = messageSizes[outgoingMessage->message.header.messageType];

        if (message >= &xudp->messages[sizeof(xudp->messages) / sizeof(GekkotaXudpMessage)] ||
                buffer + 1 >= &xudp->buffers[sizeof(xudp->buffers) / sizeof(GekkotaBuffer)] ||
                client->mtu - xudp->packetSize < messageSize)
        {
            done = 1;
            break;
        }

        iterator = gekkota_list_next(iterator);

        if (outgoingMessage->packet != NULL)
        {
            if (client->reliableDataInTransit
                    + outgoingMessage->fragmentLength > client->windowSize)
                break;

            if ((client->mtu - xudp->packetSize) <
                    (messageSize + outgoingMessage->fragmentLength))
            {
                done = 1;
                break;
            }
        }

        if (outgoingMessage->roundTripTimeout == 0)
        {
            outgoingMessage->roundTripTimeout = client->roundTripTime + 4
                * client->roundTripTimeVariance;
            outgoingMessage->roundTripTimeoutLimit = GEKKOTA_XUDP_CLIENT_TIMEOUT_LIMIT
                * outgoingMessage->roundTripTimeout;
        }

        if (gekkota_list_is_empty(&client->sentReliableMessages))
            client->nextTimeout = xudp->currentTime + outgoingMessage->roundTripTimeout;

        gekkota_list_add(
                &client->sentReliableMessages,
                gekkota_list_remove(&outgoingMessage->listNode));

        outgoingMessage->sentTime = xudp->currentTime;

        buffer->data = message;
        buffer->length = messageSize;

        xudp->packetSize += buffer->length;
        gekkota_bit_set(xudp->headerFlags, GEKKOTA_XUDP_HEADER_FLAG_SENT_TIME);
        *message = outgoingMessage->message;

        if (outgoingMessage->packet != NULL)
        {
            ++buffer;

            buffer->data = (byte_t *) outgoingMessage->packet->data.data
                + outgoingMessage->fragmentOffset;
            buffer->length = outgoingMessage->fragmentLength;

            xudp->packetSize += outgoingMessage->fragmentLength;
            client->reliableDataInTransit += outgoingMessage->fragmentLength;
        }

        ++message;
        ++buffer;
    }

    xudp->messageCount = (uint16_t) (message - xudp->messages);
    xudp->bufferCount = (uint16_t) (buffer - xudp->buffers);

    return done;
}

static int32_t
_gekkota_xudp_send_unreliable(
        GekkotaXudp *restrict xudp,
        GekkotaXudpClient *client)
{
    GekkotaXudpMessage *message = &xudp->messages[xudp->messageCount];
    GekkotaBuffer *buffer = &xudp->buffers[xudp->bufferCount];
    GekkotaOutgoingMessage *outgoingMessage;
    GekkotaListIterator iterator;
    int32_t done = 0;

    iterator = gekkota_list_head(&client->outgoingUnreliableMessages);

    while (iterator != gekkota_list_tail(&client->outgoingUnreliableMessages))
    {
        size_t messageSize;

        outgoingMessage = (GekkotaOutgoingMessage *) iterator;
        messageSize = messageSizes[outgoingMessage->message.header.messageType];

        if (message >= &xudp->messages[sizeof(xudp->messages) / sizeof(GekkotaXudpMessage)] ||
                buffer + 1 >= &xudp->buffers[sizeof(xudp->buffers) / sizeof (GekkotaBuffer)] ||
                client->mtu - xudp->packetSize < messageSize ||
                (outgoingMessage->packet != NULL &&
                client->mtu - xudp->packetSize < messageSize + outgoingMessage->packet->data.length))
        {
            done = 1;
            break;
        }

        iterator = gekkota_list_next(iterator);

        if (outgoingMessage->packet != NULL)
        {
            client->packetThrottleCounter += GEKKOTA_XUDP_CLIENT_PACKET_THROTTLE_COUNTER;
            client->packetThrottleCounter %= GEKKOTA_XUDP_CLIENT_PACKET_THROTTLE_SCALE;

            if (client->packetThrottleCounter > client->packetThrottle)
            {
                gekkota_packet_destroy(outgoingMessage->packet);
                gekkota_list_remove(&outgoingMessage->listNode);
                gekkota_memory_free(outgoingMessage);

                continue;
            }
        }

        buffer->data = message;
        buffer->length = messageSize;
        xudp->packetSize += buffer->length;
        *message = outgoingMessage->message;
        gekkota_list_remove(&outgoingMessage->listNode);

        if (outgoingMessage->packet != NULL)
        {
            ++buffer;

            buffer->data = outgoingMessage->packet->data.data;
            buffer->length = outgoingMessage->packet->data.length;

            xudp->packetSize += buffer->length;
            gekkota_list_add(&client->sentUnreliableMessages, outgoingMessage);
        }
        else
            gekkota_memory_free(outgoingMessage);

        ++message;
        ++buffer;
    }

    xudp->messageCount = (uint16_t) (message - xudp->messages);
    xudp->bufferCount = (uint16_t) (buffer - xudp->buffers);

    if (client->state == GEKKOTA_CLIENT_STATE_DELAYING_DISCONNECT &&
            gekkota_list_is_empty(&client->outgoingReliableMessages) &&
            gekkota_list_is_empty(&client->outgoingUnreliableMessages) &&
            gekkota_list_is_empty(&client->sentReliableMessages))
        if (_gekkota_xudpclient_close_gracefully(client) < 0)
            return -1;

    return done;
}

static int32_t
_gekkota_xudp_throttle_bandwidth(GekkotaXudp *restrict xudp)
{
    uint32_t currentTime, elapsedTime;
    uint32_t clientsTotal = 0, dataTotal = 0, clientsRemaining;
    uint32_t bandwidth, throttle = 0, bandwidthLimit = 0;
    bool_t adjustBandwidth;
    GekkotaXudpClient *client;
    GekkotaXudpMessage message;

    elapsedTime = currentTime = (uint32_t) gekkota_time_now();
    elapsedTime -= xudp->bandwidthThrottleEpoch;

    if (elapsedTime < GEKKOTA_XUDP_BANDWIDTH_THROTTLE_INTERVAL)
        return 0;

    for (client = xudp->clients;
            client < &xudp->clients[xudp->clientCount];
            client++)
    {
        if (client->state != GEKKOTA_CLIENT_STATE_CONNECTED &&
                client->state != GEKKOTA_CLIENT_STATE_DELAYING_DISCONNECT)
            continue;

        ++clientsTotal;
        dataTotal += client->outgoingDataTotal;
    }

    if (clientsTotal == 0)
        return 0;

    clientsRemaining = clientsTotal;
    adjustBandwidth = TRUE;

    if (xudp->outgoingBandwidth == 0)
        bandwidth = gekkota_bit_not(0);
    else
        bandwidth = (xudp->outgoingBandwidth * elapsedTime) / 1000;

    while (clientsRemaining > 0 && adjustBandwidth)
    {
        adjustBandwidth = FALSE;

        if (dataTotal < bandwidth)
            throttle = GEKKOTA_XUDP_CLIENT_PACKET_THROTTLE_SCALE;
        else
            throttle = (bandwidth * GEKKOTA_XUDP_CLIENT_PACKET_THROTTLE_SCALE)
                / dataTotal;

        for (client = xudp->clients;
                client < &xudp->clients[xudp->clientCount];
                client++)
        {
            uint32_t clientBandwidth;

            if ((client->state != GEKKOTA_CLIENT_STATE_CONNECTED &&
                    client->state != GEKKOTA_CLIENT_STATE_DELAYING_DISCONNECT) ||
                    client->incomingBandwidth == 0 ||
                    client->outgoingBandwidthThrottleEpoch == currentTime)
                continue;

            clientBandwidth = (xudp->incomingBandwidth * elapsedTime) / 1000;

            if ((throttle * client->outgoingDataTotal)
                    / GEKKOTA_XUDP_CLIENT_PACKET_THROTTLE_SCALE <= clientBandwidth)
                continue;

            client->packetThrottleLimit = (clientBandwidth
                    * GEKKOTA_XUDP_CLIENT_PACKET_THROTTLE_SCALE)
                    / client->outgoingDataTotal;

            if (client->packetThrottleLimit == 0)
                client->packetThrottleLimit = 1;

            if (client->packetThrottle > client->packetThrottleLimit)
                client->packetThrottle = client->packetThrottleLimit;

            client->outgoingBandwidthThrottleEpoch = currentTime;

            adjustBandwidth = TRUE;
            --clientsRemaining;
            bandwidth -= clientBandwidth;
            dataTotal -= clientBandwidth;
        }
    }

    if (clientsRemaining > 0)
    {
        for (client = xudp->clients;
                client < &xudp->clients[xudp->clientCount];
                client++)
        {
            if ((client->state != GEKKOTA_CLIENT_STATE_CONNECTED &&
                    client->state != GEKKOTA_CLIENT_STATE_DELAYING_DISCONNECT) ||
                    client->outgoingBandwidthThrottleEpoch == currentTime)
                continue;

            client->packetThrottleLimit = throttle;

            if (client->packetThrottle > client->packetThrottleLimit)
                client->packetThrottle = client->packetThrottleLimit;
        }
    }

    if (xudp->reconfigureBandwidth)
    {
        xudp->reconfigureBandwidth = FALSE;

        clientsRemaining = clientsTotal;
        bandwidth = xudp->incomingBandwidth;
        adjustBandwidth = TRUE;

        if (bandwidth == 0)
            bandwidthLimit = 0;
        else while (clientsRemaining > 0 && adjustBandwidth)
        {
            adjustBandwidth = FALSE;
            bandwidthLimit = bandwidth / clientsRemaining;

            for (client = xudp->clients;
                    client < &xudp->clients[xudp->clientCount];
                    client++)
            {
                if ((client->state != GEKKOTA_CLIENT_STATE_CONNECTED &&
                        client->state != GEKKOTA_CLIENT_STATE_DELAYING_DISCONNECT) ||
                        client->incomingBandwidthThrottleEpoch == currentTime)
                    continue;

                if (client->outgoingBandwidth > 0 &&
                        client->outgoingBandwidth >= bandwidthLimit)
                    continue;

                client->incomingBandwidthThrottleEpoch = currentTime;
                adjustBandwidth = TRUE;
                --clientsRemaining;
                bandwidth -= client->outgoingBandwidth;
            }
        }

        for (client = xudp->clients;
                client < &xudp->clients[xudp->clientCount];
                client++)
        {
            if (client->state != GEKKOTA_CLIENT_STATE_CONNECTED &&
                    client->state != GEKKOTA_CLIENT_STATE_DELAYING_DISCONNECT)
                continue;

            /* message header */
            message.header.messageType = GEKKOTA_XUDP_MESSAGE_TYPE_CONFIGURE_BANDWIDTH;
            message.header.channelId = 0xFF;
            message.header.flags = client->isMulticastGroupMember
                ? GEKKOTA_XUDP_MESSAGE_FLAG_UNSEQUENCED
                : GEKKOTA_XUDP_MESSAGE_FLAG_ACKNOWLEDGE;

            /* message body */
            message.configureBandwidth.outgoingBandwidth =
                gekkota_host_to_net_32(xudp->outgoingBandwidth);

            if (xudp->bandwidthThrottleEpoch == currentTime)
                message.configureBandwidth.incomingBandwidth =
                    gekkota_host_to_net_32(client->outgoingBandwidth);
            else
                message.configureBandwidth.incomingBandwidth =
                    gekkota_host_to_net_32(bandwidthLimit);

            if (_gekkota_xudpclient_queue_outgoing_message(
                    client, &message, NULL, 0, 0, NULL) != 0)
                return -1;
        }
    }

    xudp->bandwidthThrottleEpoch = currentTime;

    for (client = xudp->clients;
            client < &xudp->clients[xudp->clientCount];
            client++)
    {
        client->incomingDataTotal = 0;
        client->outgoingDataTotal = 0;
    }

    return 0;
}
