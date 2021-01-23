/******************************************************************************
 * @file    gekkota_xudpclient.c
 * @date    26-Jul-2007
 * @author  <a href="mailto:giuseppe.greco@agamura.com">Giuseppe Greco</a>
 *
 * Copyright (C) 2007 Agamura, Inc. - http://www.agamura.com
 * All right reserved.
 *****************************************************************************/

#include <errno.h>
#include <string.h>
#include "gekkota.h"
#include "gekkota_bit.h"
#include "gekkota_errors.h"
#include "gekkota_memory.h"
#include "gekkota_list.h"
#include "gekkota_utils.h"
#include "gekkota_xudp.h"
#include "gekkota_xudpclient.h"

static GekkotaPacket *
_gekkota_xudpclient_deflate(
        GekkotaXudpClient *client,
        const GekkotaPacket *packet);

static GekkotaPacket *
_gekkota_xudpclient_inflate(
        GekkotaXudpClient *client,
        const GekkotaPacket *packet);

static GekkotaLZF *
_gekkota_xudpclient_get_lzf(const GekkotaXudpClient *client);

#define _gekkota_xudpclient_clear_message_queue(type_t, queue) \
{ \
    type_t *message; \
    while (!gekkota_list_is_empty(queue)) \
    { \
        message = (type_t *) gekkota_list_remove(gekkota_list_head(queue)); \
        if (message->packet != NULL) \
            gekkota_packet_destroy(message->packet); \
        gekkota_memory_free(message); \
    } \
}

int32_t
gekkota_xudpclient_destroy(GekkotaXudpClient *client)
{
    GekkotaXudp *xudp;

    if (client == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return -1;
    }

    /*
     * Dispose of the remote endpoint.
     */
    if (client->remoteEndPoint != NULL && client->isMulticastGroupMember)
    {
        if (client->remoteEndPoint->port == gekkota_socket_get_local_endpoint(
                client->xudp->socket)->port)
        {
            gekkota_socket_leave_multicast_group_1(
                    client->xudp->socket,
                    client->remoteEndPoint->address,
                    client->multicastInterfaceIndex);
        }

        gekkota_ipendpoint_destroy(client->remoteEndPoint);
    }

    /*
     * Dispose of the compression algorithm, if any.
     */
    if (client->lzf != NULL)
        gekkota_lzf_destroy(client->lzf);

    /*
     * Dispose of all the still queued incoming and outgoing messages.
     */
    _gekkota_xudpclient_clear_message_queues(client);

    xudp = client->xudp;
    memset(client, 0x00, (size_t ) &((GekkotaXudpClient *) 0)->acknowledgements);
    client->xudp = xudp;

    /*
     * Reset the client.
     */
    _gekkota_xudpclient_reset(client);

    return 0;
}

int32_t
gekkota_xudpclient_close_1(GekkotaXudpClient *client, GekkotaCloseMode closeMode)
{
    if (client == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return -1;
    }

    switch (closeMode)
    {
        case GEKKOTA_CLOSE_MODE_GRACEFUL:
            return _gekkota_xudpclient_close_gracefully(client);

        case GEKKOTA_CLOSE_MODE_DELAYED:
            return _gekkota_xudpclient_close_later(client);

        case GEKKOTA_CLOSE_MODE_IMMEDIATE:
            return _gekkota_xudpclient_close_now(client);
    }

    errno = GEKKOTA_ERROR_ARGUMENT_NOT_VALID;
    return -1;
}

int32_t
gekkota_xudpclient_get_channel_count(const GekkotaXudpClient *client)
{
    if (client == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return -1;
    }

    return (int32_t) client->channelCount;
}

GekkotaCompressionLevel
gekkota_xudpclient_get_compression_level(const GekkotaXudpClient *client)
{
    if (client == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return GEKKOTA_COMPRESSION_LEVEL_UNDEFINED;
    }

    return client->compressionLevel;
}

GekkotaIPEndPoint *
gekkota_xudpclient_get_local_endpoint(const GekkotaXudpClient *client)
{
    if (client == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return NULL;
    }

    return gekkota_socket_get_local_endpoint(client->xudp->socket);
}

GekkotaIPEndPoint *
gekkota_xudpclient_get_remote_endpoint(const GekkotaXudpClient *client)
{
    if (client == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return NULL;
    }

    return client->remoteEndPoint;
}

GekkotaClientState
gekkota_xudpclient_get_state(const GekkotaXudpClient *client)
{
    if (client == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return GEKKOTA_CLIENT_STATE_UNDEFINED;
    }

    return client->state;
}

int32_t
gekkota_xudpclient_get_throttle_configuration(
        GekkotaXudpClient *client,
        uint32_t *restrict interval,
        uint32_t *restrict acceleration,
        uint32_t *restrict deceleration)
{
    if (client == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return -1;
    }

    if (interval != NULL)
        *interval = client->packetThrottleInterval;

    if (acceleration != NULL)
        *acceleration = client->packetThrottleAcceleration;

    if (deceleration != NULL)
        *deceleration = client->packetThrottleDeceleration;

    return 0;
}

int32_t
gekkota_xudpclient_set_throttle_configuration(
        GekkotaXudpClient *client,
        uint32_t interval,
        uint32_t acceleration,
        uint32_t deceleration)
{
    GekkotaXudpMessage message;

    if (client == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return -1;
    }

    client->packetThrottleInterval = interval;
    client->packetThrottleAcceleration = acceleration;
    client->packetThrottleDeceleration = deceleration;

    /* message header */
    message.header.messageType = GEKKOTA_XUDP_MESSAGE_TYPE_CONFIGURE_THROTTLE;
    message.header.channelId = 0xFF;
    message.header.flags = client->isMulticastGroupMember
        ? GEKKOTA_XUDP_MESSAGE_FLAG_UNSEQUENCED
        : GEKKOTA_XUDP_MESSAGE_FLAG_ACKNOWLEDGE;

    /* message body */
    message.configureThrottle.throttleInterval = gekkota_host_to_net_32(interval);
    message.configureThrottle.throttleAcceleration = gekkota_host_to_net_32(acceleration);
    message.configureThrottle.throttleDeceleration = gekkota_host_to_net_32(deceleration);

    return _gekkota_xudpclient_queue_outgoing_message(
            client, &message, NULL, 0, 0, NULL);
}

bool_t
gekkota_xudpclient_is_multicast_group_member(const GekkotaXudpClient *client)
{
    if (client == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return FALSE;
    }

    return client->isMulticastGroupMember;
}

int32_t
gekkota_xudpclient_receive(
        GekkotaXudpClient *client,
        uint8_t channelId,
        GekkotaPacket **packet,
        GekkotaIPEndPoint **remoteEndPoint)
{
    GekkotaChannel *channel;
    GekkotaIncomingMessage *incomingMessage = NULL;


    if (client == NULL || packet == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return -1;
    }

    if (channelId >= client->channelCount)
    {
        errno = GEKKOTA_ERROR_ARGUMENT_NOT_VALID;
        return -1;
    }

    channel = &client->channels[channelId];

    /*
     * Check for unreliable messages first.
     */
    if (!gekkota_list_is_empty(&channel->incomingUnreliableMessages))
    {
        incomingMessage = (GekkotaIncomingMessage *)
            gekkota_list_first(&channel->incomingUnreliableMessages);

        if (incomingMessage->message.header.messageType == GEKKOTA_XUDP_MESSAGE_TYPE_UNRELIABLE_DATA)
        {
            if (incomingMessage->reliableSequenceNumber != channel->incomingReliableSequenceNumber)
                incomingMessage = NULL;
            else
                channel->incomingUnreliableSequenceNumber = incomingMessage->unreliableSequenceNumber;
        }
    }

    /*
     * If there are no unreliable messages and the client is not member of a
     * multicast group, then check for reliable messages.
     */
    if (incomingMessage == NULL && !client->isMulticastGroupMember &&
            !gekkota_list_is_empty(&channel->incomingReliableMessages))
    {
        incomingMessage = (GekkotaIncomingMessage *)
            gekkota_list_first(&channel->incomingReliableMessages);

        if (incomingMessage->fragmentsRemaining > 0 ||
                incomingMessage->reliableSequenceNumber !=
                (uint16_t) (channel->incomingReliableSequenceNumber + 1))
            /*
             * Still waiting for data fragments...
             */
            return 0;

        channel->incomingReliableSequenceNumber = incomingMessage->reliableSequenceNumber;

        if (incomingMessage->fragmentCount > 0)
            channel->incomingReliableSequenceNumber
                += (uint16_t) incomingMessage->fragmentCount - 1;
    }

    if (incomingMessage == NULL)
        return 0;

    gekkota_list_remove(&incomingMessage->listNode);
    *packet = incomingMessage->packet;

    if (remoteEndPoint != NULL)
        *remoteEndPoint = incomingMessage->remoteEndPoint;
    else
        gekkota_ipendpoint_destroy(incomingMessage->remoteEndPoint);

    /*
     * If the GEKKOTA_PACKET_FLAG_COMPRESSED flag is on, inflate the packet.
     */
    if (gekkota_bit_isset(((GekkotaPacket *) *packet)->flags, GEKKOTA_PACKET_FLAG_COMPRESSED))
    {
        GekkotaPacket *inflated;

        inflated = _gekkota_xudpclient_inflate(client, *packet);
        gekkota_packet_destroy(*packet);
        *packet = inflated;
    }

    gekkota_memory_free(incomingMessage);
    return *packet != NULL ? 1 : -1;
}
    
int32_t
gekkota_xudpclient_send(
        GekkotaXudpClient *client,
        uint8_t channelId,
        GekkotaPacket *packet)
{
    int32_t rc = 0;
    GekkotaChannel *channel;
    GekkotaXudpMessage message;
    uint16_t fragmentLength;

    if (client == NULL || packet == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return -1;
    }

    if (packet->data.length > GEKKOTA_XUDP_CLIENT_MAX_PACKET_SIZE)
    {
        errno = GEKKOTA_ERROR_BUFFER_OVERFLOW;
        return -1;
    }

    if (channelId >= client->channelCount)
    {
        errno = GEKKOTA_ERROR_ARGUMENT_NOT_VALID;
        return -1;
    }

    if (client->state != GEKKOTA_CLIENT_STATE_CONNECTED)
    {
        errno = GEKKOTA_ERROR_CLIENT_NOT_CONNECTED;
        return -1;
    }

    if (client->isMulticastGroupMember &&
            gekkota_bit_isset(packet->flags, GEKKOTA_PACKET_FLAG_RELIABLE))
    {
        errno = GEKKOTA_ERROR_OPERATION_NOT_SUPPORTED;
        return -1;
    }

    /*
     * Reset message flags.
     */
    message.header.flags = 0;

    /*
     * If the GEKKOTA_PACKET_FLAG_COMPRESSED flag is on, deflate the packet.
     */
    if (gekkota_bit_isset(packet->flags, GEKKOTA_PACKET_FLAG_COMPRESSED))
    {
        GekkotaPacket *deflated;

        if ((deflated = _gekkota_xudpclient_deflate(client, packet)) == NULL)
            return -1;

        if (deflated->data.length >= packet->data.length)
        {
            gekkota_packet_destroy(deflated);
            gekkota_bit_unset(packet->flags, GEKKOTA_PACKET_FLAG_COMPRESSED);
        }
        else
        {
            packet = deflated;
            gekkota_bit_set(message.header.flags, GEKKOTA_XUDP_MESSAGE_FLAG_COMPRESSED);
        }
    }

    channel = &client->channels[channelId];

    fragmentLength = client->mtu
        - sizeof(GekkotaXudpHeader)
        - sizeof(GekkotaXudpDataFragmentMessage);

    if (packet->data.length > fragmentLength)
    {
        /*
         * Packet too long: fragment it.
         */

        uint16_t startSequenceNumber;
        uint32_t fragmentCount, fragmentNumber, fragmentOffset;

        if (client->isMulticastGroupMember)
        {
            /*
             * Fragments are always reliable and they cannot be multicast.
             */
            errno = GEKKOTA_ERROR_OPERATION_NOT_SUPPORTED;
            goto gekkota_xudpclient_send_exit;
        }

        startSequenceNumber = gekkota_host_to_net_16(
                channel->outgoingReliableSequenceNumber + 1);
        fragmentCount = gekkota_host_to_net_32(
                (uint32_t) (packet->data.length + fragmentLength - 1)
                / fragmentLength);

        gekkota_bit_set(packet->flags, GEKKOTA_PACKET_FLAG_RELIABLE);
        gekkota_bit_unset(packet->flags, GEKKOTA_PACKET_FLAG_UNSEQUENCED);

        for (fragmentNumber = 0, fragmentOffset = 0;
                fragmentOffset < packet->data.length;
                fragmentNumber++, fragmentOffset += fragmentLength)
        {
            if (packet->data.length - fragmentOffset < fragmentLength)
                fragmentLength = (uint16_t) (packet->data.length - fragmentOffset);

            /* message header */
            message.header.messageType = GEKKOTA_XUDP_MESSAGE_TYPE_DATA_FRAGMENT;
            message.header.channelId = channelId;
            gekkota_bit_set(message.header.flags, GEKKOTA_XUDP_MESSAGE_FLAG_ACKNOWLEDGE);

            /* message body */
            message.dataFragment.startSequenceNumber = startSequenceNumber;
            message.dataFragment.length = gekkota_host_to_net_16(fragmentLength);
            message.dataFragment.fragmentCount = fragmentCount;
            message.dataFragment.fragmentNumber = gekkota_host_to_net_32(fragmentNumber);
            message.dataFragment.totalLength = gekkota_host_to_net_32((uint32_t) packet->data.length);
            message.dataFragment.fragmentOffset = gekkota_net_to_host_32(fragmentOffset);

            if ((rc = _gekkota_xudpclient_queue_outgoing_message(
                    client, &message, packet, fragmentOffset, fragmentLength, NULL)) < 0)
            {
                /*
                 * Error: remove all the packet fragments queued so far into
                 * the outgoing messages queue.
                 */

                GekkotaOutgoingMessage * outgoingMessage;

                while (fragmentNumber > 0)
                {
                    outgoingMessage = (GekkotaOutgoingMessage *)
                        gekkota_list_remove(
                                gekkota_list_tail(&client->outgoingReliableMessages));

                    gekkota_packet_destroy(outgoingMessage->packet);
                    --fragmentNumber;
                }

                break;
            }
        }
    }
    else
    {
        /*
         * Packet fits with the current mtu.
         */

        message.header.channelId = channelId;

        if (gekkota_bit_isset(packet->flags, GEKKOTA_PACKET_FLAG_RELIABLE))
        {
            message.header.messageType = GEKKOTA_XUDP_MESSAGE_TYPE_RELIABLE_DATA;
            gekkota_bit_set(message.header.flags, GEKKOTA_XUDP_MESSAGE_FLAG_ACKNOWLEDGE);
            message.reliableData.length = gekkota_host_to_net_16((uint16_t) packet->data.length);
        }
        else if (gekkota_bit_isset(packet->flags, GEKKOTA_PACKET_FLAG_UNSEQUENCED))
        {
            message.header.messageType = GEKKOTA_XUDP_MESSAGE_TYPE_UNSEQUENCED_DATA;
            gekkota_bit_set(message.header.flags, GEKKOTA_XUDP_MESSAGE_FLAG_UNSEQUENCED);
            message.unsequencedData.group = gekkota_host_to_net_16(
                    client->outgoingUnsequencedGroup + 1);
            message.unsequencedData.length = gekkota_host_to_net_16((uint16_t) packet->data.length);
        }
        else
        {
            message.header.messageType = GEKKOTA_XUDP_MESSAGE_TYPE_UNRELIABLE_DATA;
            message.unreliableData.sequenceNumber = gekkota_host_to_net_16(
                    channel->outgoingUnreliableSequenceNumber + 1);
            message.unreliableData.length = gekkota_host_to_net_16((uint16_t) packet->data.length);
        }

        rc = _gekkota_xudpclient_queue_outgoing_message(
                client, &message, packet, 0, (uint16_t) packet->data.length, NULL);
    }

gekkota_xudpclient_send_exit:
    if (gekkota_bit_isset(packet->flags, GEKKOTA_PACKET_FLAG_COMPRESSED))
        gekkota_packet_destroy(packet);

    return rc;
}

int32_t
gekkota_xudpclient_ping(GekkotaXudpClient *client)
{
    GekkotaXudpMessage message;

    if (client == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return -1;
    }

    if (client->state != GEKKOTA_CLIENT_STATE_CONNECTED)
    {
        errno = GEKKOTA_ERROR_CLIENT_NOT_CONNECTED;
        return -1;
    }

    if (client->isMulticastGroupMember)
    {
        errno = GEKKOTA_ERROR_OPERATION_NOT_SUPPORTED;
        return -1;
    }

    /* message header */
    message.header.messageType = GEKKOTA_XUDP_MESSAGE_TYPE_PING;
    message.header.channelId = 0xFF;
    message.header.flags = GEKKOTA_XUDP_MESSAGE_FLAG_ACKNOWLEDGE;

    /* no message body */
   
    return _gekkota_xudpclient_queue_outgoing_message(
            client, &message, NULL, 0, 0, NULL);
}

int32_t
_gekkota_xudpclient_close_gracefully(GekkotaXudpClient *client)
{
    GekkotaXudpMessage message;

    if (client == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return -1;
    }

    if (client->state == GEKKOTA_CLIENT_STATE_DISCONNECTING ||
            client->state == GEKKOTA_CLIENT_STATE_DISCONNECTED ||
            client->state == GEKKOTA_CLIENT_STATE_ZOMBIE)
    {
        errno = GEKKOTA_ERROR_CLIENT_NOT_CONNECTED;
        return -1;
    }

    _gekkota_xudpclient_clear_message_queues(client);

    if (client->isMulticastGroupMember)
    {
        /* message header */
        message.header.messageType = GEKKOTA_XUDP_MESSAGE_TYPE_LEAVE_MULTICAST_GROUP;
        message.header.channelId = 0xFF;
        message.header.flags = GEKKOTA_XUDP_MESSAGE_FLAG_UNSEQUENCED;

        if (_gekkota_xudpclient_queue_outgoing_message(
                client, &message, NULL, 0, 0, NULL) != 0)
            return -1;
    }
    else
    {
        /* message header */
        message.header.messageType = GEKKOTA_XUDP_MESSAGE_TYPE_DISCONNECT;
        message.header.channelId = 0xFF;
        message.header.flags = client->state == GEKKOTA_CLIENT_STATE_CONNECTED ||
            client->state == GEKKOTA_CLIENT_STATE_DELAYING_DISCONNECT
            ? GEKKOTA_XUDP_MESSAGE_FLAG_ACKNOWLEDGE
            : GEKKOTA_XUDP_MESSAGE_FLAG_UNSEQUENCED;

        if (_gekkota_xudpclient_queue_outgoing_message(
                client, &message, NULL, 0, 0, NULL) != 0)
            return -1;

        if (client->state == GEKKOTA_CLIENT_STATE_CONNECTED ||
                client->state == GEKKOTA_CLIENT_STATE_DELAYING_DISCONNECT)
        {
            client->state = GEKKOTA_CLIENT_STATE_DISCONNECTING;
            return 0;
        }
    }

    return gekkota_utils_min(
            gekkota_xudp_flush(client->xudp),
            gekkota_xudpclient_destroy(client));
}

int32_t
_gekkota_xudpclient_close_later(GekkotaXudpClient *client)
{
    if (client == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return -1;
    }

    if ((client->state == GEKKOTA_CLIENT_STATE_CONNECTED ||
            client->state == GEKKOTA_CLIENT_STATE_DELAYING_DISCONNECT) &&
            !(gekkota_list_is_empty(&client->outgoingReliableMessages) &&
                gekkota_list_is_empty(&client->outgoingUnreliableMessages) &&
                gekkota_list_is_empty(&client->sentReliableMessages)))
        client->state = GEKKOTA_CLIENT_STATE_DELAYING_DISCONNECT;
    else
        return _gekkota_xudpclient_close_gracefully(client);

    return 0;
}

int32_t
_gekkota_xudpclient_close_now(GekkotaXudpClient *client)
{
    GekkotaXudpMessage message;

    if (client == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return -1;
    }

    if (client->state == GEKKOTA_CLIENT_STATE_DISCONNECTED)
    {
        errno = GEKKOTA_ERROR_CLIENT_NOT_CONNECTED;
        return -1;
    }

    if (client->state != GEKKOTA_CLIENT_STATE_ZOMBIE &&
            client->state != GEKKOTA_CLIENT_STATE_DISCONNECTING)
    {
        _gekkota_xudpclient_clear_message_queues(client);

        /* message header */
        message.header.messageType = client->isMulticastGroupMember
            ? GEKKOTA_XUDP_MESSAGE_TYPE_LEAVE_MULTICAST_GROUP
            : GEKKOTA_XUDP_MESSAGE_TYPE_DISCONNECT;
        message.header.channelId = 0xFF;
        message.header.flags = GEKKOTA_XUDP_MESSAGE_FLAG_UNSEQUENCED;

        if (_gekkota_xudpclient_queue_outgoing_message(
                client, &message, NULL, 0, 0, NULL) != 0)
            return -1;

        if (gekkota_xudp_flush(client->xudp) != 0)
            return -1;
    }

    return gekkota_xudpclient_destroy(client);
}

void_t
_gekkota_xudpclient_clear_message_queues(GekkotaXudpClient *client)
{
    GekkotaChannel *channel;

    /*
     * Dispose of acknowledgements.
     */
    while (!gekkota_list_is_empty(&client->acknowledgements))
        gekkota_memory_free(gekkota_list_remove(gekkota_list_head(&client->acknowledgements)));

    /*
     * Dispose of outgoing messages.
     */
    _gekkota_xudpclient_clear_outgoing_message_queue(&client->sentReliableMessages);
    _gekkota_xudpclient_clear_outgoing_message_queue(&client->sentUnreliableMessages);
    _gekkota_xudpclient_clear_outgoing_message_queue(&client->outgoingReliableMessages);
    _gekkota_xudpclient_clear_outgoing_message_queue(&client->outgoingUnreliableMessages);

    /*
     * Dispose of incoming messages.
     */
    if (client->channels != NULL && client->channelCount > 0)
    {
        for (channel = client->channels;
                channel < &client->channels[client->channelCount];
                channel++)
        {
            _gekkota_xudpclient_clear_incoming_message_queue(&channel->incomingReliableMessages);
            _gekkota_xudpclient_clear_incoming_message_queue(&channel->incomingUnreliableMessages);
        }

        gekkota_memory_free(client->channels);
    }

    client->channels = NULL;
    client->channelCount = 0;
}

void_t
_gekkota_xudpclient_clear_incoming_message_queue(GekkotaList *queue)
{
    _gekkota_xudpclient_clear_message_queue(GekkotaIncomingMessage, queue);
}

void_t
_gekkota_xudpclient_clear_outgoing_message_queue(GekkotaList *queue)
{
    _gekkota_xudpclient_clear_message_queue(GekkotaOutgoingMessage, queue);
}

int32_t
_gekkota_xudpclient_queue_acknowledgement(
        GekkotaXudpClient *restrict client,
        const GekkotaXudpMessage *message,
        uint32_t sentTime,
        GekkotaAcknowledgement **acknowledgement)
{
    GekkotaAcknowledgement *newAcknowledgement;

    if ((newAcknowledgement = gekkota_memory_alloc(
            sizeof(GekkotaAcknowledgement), FALSE)) == NULL)
        return -1;

    client->outgoingDataTotal += sizeof(GekkotaXudpAcknowledgeMessage);

    newAcknowledgement->sentTime = sentTime;
    newAcknowledgement->message = *message;

    gekkota_list_add(&client->acknowledgements, newAcknowledgement);

    if (acknowledgement != NULL)
        *acknowledgement = newAcknowledgement;

    return 0;
}

int32_t
_gekkota_xudpclient_queue_incoming_message(
        GekkotaXudpClient *client,
        const GekkotaXudpMessage *message,
        GekkotaPacket *packet,
        uint32_t fragmentCount,
        GekkotaIPEndPoint *remoteEndPoint,
        GekkotaIncomingMessage **incomingMessage)
{
    GekkotaChannel *channel;
    GekkotaIncomingMessage *newIncomingMessage;
    GekkotaListIterator iterator;
    uint32_t unreliableSequenceNumber = 0, reliableSequenceNumber = 0;
    size_t memSize;

    channel = &client->channels[message->header.channelId];

    if (client->state == GEKKOTA_CLIENT_STATE_DELAYING_DISCONNECT)
        return -1;

    if (message->header.messageType != GEKKOTA_XUDP_MESSAGE_TYPE_UNSEQUENCED_DATA)
    {
        reliableSequenceNumber = message->header.sequenceNumber;

        /*
         * If the sequence numbers received so far are all at the high end of the
         * 16-bit range, but a just received sequence number is at the low end,
         * assume it was a sequence number that wrapped around back to the low end
         * again and treat it as a number greater than 2e16, being values greater
         * than or equal to 0xF000 the high end, and values less than 0x1000 the
         * low end.
         */

        if (channel->incomingReliableSequenceNumber >= 0xF000 &&
                reliableSequenceNumber < 0x1000)
            reliableSequenceNumber += 0x10000;

        /*
         * Check if even after adjustment the sequence number is less than the
         * ones already received; it is almost impossible to have anywhere near
         * to 2e16 packets in flight at any given time.
         */

        if (reliableSequenceNumber < channel->incomingReliableSequenceNumber ||
                (channel->incomingReliableSequenceNumber < 0x1000 &&
                (reliableSequenceNumber & 0x0000FFFF) >= 0xF000))
            return -1;
    }

    switch (message->header.messageType)
    {
        case GEKKOTA_XUDP_MESSAGE_TYPE_DATA_FRAGMENT:
        case GEKKOTA_XUDP_MESSAGE_TYPE_RELIABLE_DATA:
            if (reliableSequenceNumber == channel->incomingReliableSequenceNumber)
                return -1;

            for (iterator = gekkota_list_previous(
                        gekkota_list_tail(&channel->incomingReliableMessages));
                    iterator != gekkota_list_tail(&channel->incomingReliableMessages);
                    iterator = gekkota_list_previous(iterator))
            {
                newIncomingMessage = (GekkotaIncomingMessage *) iterator;

                if (reliableSequenceNumber >= 0x10000 &&
                        newIncomingMessage->reliableSequenceNumber < 0xF000)
                    reliableSequenceNumber -= 0x10000;

                if (newIncomingMessage->reliableSequenceNumber <= reliableSequenceNumber)
                {
                    if (newIncomingMessage->reliableSequenceNumber < reliableSequenceNumber)
                        break;

                    return -1;
                }
            }
            break;

        case GEKKOTA_XUDP_MESSAGE_TYPE_UNRELIABLE_DATA:
            unreliableSequenceNumber = gekkota_net_to_host_16(
                    message->unreliableData.sequenceNumber);

            if (channel->incomingUnreliableSequenceNumber >= 0xF000 &&
                    unreliableSequenceNumber < 0x1000)
                unreliableSequenceNumber += 0x10000;

            if (unreliableSequenceNumber <= channel->incomingUnreliableSequenceNumber ||
                    (channel->incomingUnreliableSequenceNumber < 0x1000 &&
                    (unreliableSequenceNumber & 0xFFFF) >= 0xF000))
                return -1;

            for (iterator = gekkota_list_previous(
                        gekkota_list_tail(&channel->incomingUnreliableMessages));
                    iterator != gekkota_list_tail(&channel->incomingUnreliableMessages);
                    iterator = gekkota_list_previous(iterator))
            {
                newIncomingMessage = (GekkotaIncomingMessage *) iterator;

                if (newIncomingMessage->message.header.messageType !=
                        GEKKOTA_XUDP_MESSAGE_TYPE_UNRELIABLE_DATA)
                    continue;

                if (unreliableSequenceNumber >= 0x10000 &&
                        newIncomingMessage->unreliableSequenceNumber < 0xF000)
                    unreliableSequenceNumber -= 0x10000;

                if (newIncomingMessage->unreliableSequenceNumber <= unreliableSequenceNumber)
                {
                    if (newIncomingMessage->unreliableSequenceNumber < unreliableSequenceNumber)
                        break;

                    return -1;
                }
            }
            break;

        case GEKKOTA_XUDP_MESSAGE_TYPE_UNSEQUENCED_DATA:
            iterator = gekkota_list_tail(&channel->incomingUnreliableMessages);
            break;

        default:
            return -1;
    }

    memSize = sizeof(GekkotaIncomingMessage);

    if (fragmentCount > 0)
        memSize += ((fragmentCount + 31) / 32 * sizeof(uint32_t));

    if ((newIncomingMessage = gekkota_memory_alloc(memSize, TRUE)) == NULL)
        return -1;

    if (fragmentCount > 0)
        newIncomingMessage->fragments = (uint32_t *) (newIncomingMessage + 1);

    newIncomingMessage->reliableSequenceNumber = message->header.sequenceNumber;
    newIncomingMessage->unreliableSequenceNumber = (uint16_t) (unreliableSequenceNumber & 0xFFFF);
    newIncomingMessage->message = *message;
    newIncomingMessage->packet = gekkota_packet_new_0(packet, FALSE);
    newIncomingMessage->fragmentCount = fragmentCount;
    newIncomingMessage->fragmentsRemaining = fragmentCount;
    newIncomingMessage->remoteEndPoint = gekkota_ipendpoint_new_0(remoteEndPoint, FALSE);

    gekkota_list_insert(gekkota_list_next(iterator), newIncomingMessage);

    if (incomingMessage != NULL)
        *incomingMessage = newIncomingMessage;

    return 0;
}

int32_t
_gekkota_xudpclient_queue_outgoing_message(
        GekkotaXudpClient *restrict client,
        const GekkotaXudpMessage *message,
        GekkotaPacket *packet,
        uint32_t offset,
        uint16_t length,
        GekkotaOutgoingMessage **outgoingMessage)
{
    GekkotaChannel *channel;
    GekkotaOutgoingMessage *newOutgoingMessage;

    if ((newOutgoingMessage = gekkota_memory_alloc(
            sizeof(GekkotaOutgoingMessage), TRUE)) == NULL)
        return -1;

    channel = &client->channels[message->header.channelId];

    client->outgoingDataTotal += (uint32_t) _gekkota_xudp_message_size(
            message->header.messageType) + length;

    if (message->header.channelId == 0xFF)
    {
        ++client->outgoingReliableSequenceNumber;
        newOutgoingMessage->reliableSequenceNumber = client->outgoingReliableSequenceNumber;
    }
    else if (gekkota_bit_isset(message->header.flags, GEKKOTA_XUDP_MESSAGE_FLAG_ACKNOWLEDGE))
    {
        ++channel->outgoingReliableSequenceNumber;
        newOutgoingMessage->reliableSequenceNumber = channel->outgoingReliableSequenceNumber;
    }
    else if (gekkota_bit_isset(message->header.flags, GEKKOTA_XUDP_MESSAGE_FLAG_UNSEQUENCED))
    {
        ++client->outgoingUnsequencedGroup;
    }
    else
    {
        ++channel->outgoingUnreliableSequenceNumber;
        newOutgoingMessage->reliableSequenceNumber = channel->outgoingReliableSequenceNumber;
        newOutgoingMessage->unreliableSequenceNumber = channel->outgoingUnreliableSequenceNumber;
    }

    newOutgoingMessage->fragmentOffset = offset;
    newOutgoingMessage->fragmentLength = length;
    newOutgoingMessage->packet = gekkota_packet_new_0(packet, FALSE);
    newOutgoingMessage->message = *message;
    newOutgoingMessage->message.header.sequenceNumber = gekkota_host_to_net_16(
            newOutgoingMessage->reliableSequenceNumber);

    if (gekkota_bit_isset(message->header.flags, GEKKOTA_XUDP_MESSAGE_FLAG_ACKNOWLEDGE))
        gekkota_list_add(&client->outgoingReliableMessages, newOutgoingMessage);
    else
        gekkota_list_add(&client->outgoingUnreliableMessages, newOutgoingMessage);

    if (outgoingMessage != NULL)
        *outgoingMessage = newOutgoingMessage;

    return 0;
}

void_t
_gekkota_xudpclient_reset(GekkotaXudpClient *restrict client)
{
    client->localClientId = (uint16_t) (client - client->xudp->clients);
    client->remoteClientId = 0xFFFF;
    client->state = GEKKOTA_CLIENT_STATE_DISCONNECTED;
    client->compressionLevel = GEKKOTA_COMPRESSION_LEVEL_UNDEFINED;
    client->packetThrottle = GEKKOTA_XUDP_CLIENT_DEFAULT_PACKET_THROTTLE;
    client->packetThrottleLimit = GEKKOTA_XUDP_CLIENT_PACKET_THROTTLE_SCALE;
    client->packetThrottleAcceleration = GEKKOTA_XUDP_CLIENT_PACKET_THROTTLE_ACCELERATION;
    client->packetThrottleDeceleration = GEKKOTA_XUDP_CLIENT_PACKET_THROTTLE_DECELERATION;
    client->packetThrottleInterval = GEKKOTA_XUDP_CLIENT_PACKET_THROTTLE_INTERVAL;
    client->lastRoundTripTime = GEKKOTA_XUDP_CLIENT_DEFAULT_ROUND_TRIP_TIME;
    client->lowestRoundTripTime = GEKKOTA_XUDP_CLIENT_DEFAULT_ROUND_TRIP_TIME;
    client->roundTripTime = GEKKOTA_XUDP_CLIENT_DEFAULT_ROUND_TRIP_TIME;
    client->mtu = client->xudp->mtu;
    client->windowSize = GEKKOTA_XUDP_MAX_WINDOW_SIZE;
}

int32_t
_gekkota_xudpclient_throttle(
        GekkotaXudpClient *restrict client,
        uint32_t roundTripTime)
{
    if (client->lastRoundTripTime <= client->lastRoundTripTimeVariance)
        client->packetThrottle = client->packetThrottleLimit;
    else if (roundTripTime < client->lastRoundTripTime)
    {
        client->packetThrottle += client->packetThrottleAcceleration;

        if (client->packetThrottle > client->packetThrottleLimit)
            client->packetThrottle = client->packetThrottleLimit;

        return 1;
    }
    else if (roundTripTime > client->lastRoundTripTime + 2 * client->lastRoundTripTimeVariance)
    {
        if (client->packetThrottle > client->packetThrottleDeceleration)
            client->packetThrottle -= client->packetThrottleDeceleration;
        else
            client->packetThrottle = 0;

        return -1;
    }

    return 0;
}

static GekkotaPacket *
_gekkota_xudpclient_deflate(
        GekkotaXudpClient *client,
        const GekkotaPacket *packet)
{
    int32_t length;
    GekkotaPacket *deflated;

    if (client->lzf == NULL)
        if ((client->lzf = _gekkota_xudpclient_get_lzf(client)) == NULL)
            return NULL;

    length = gekkota_lzf_get_max_deflated_length(client->lzf, &packet->data);

    if ((deflated = gekkota_packet_new(length)) == NULL)
        return NULL;

    if ((length = gekkota_lzf_deflate(
            client->lzf,
            &packet->data,
            &deflated->data)) < 0)
    {
        gekkota_packet_destroy(deflated);
        return NULL;
    }

    deflated->data.length = length;
    deflated->flags = packet->flags;

    return deflated;
}

static GekkotaPacket *
_gekkota_xudpclient_inflate(
        GekkotaXudpClient * client,
        const GekkotaPacket *packet)
{
    GekkotaPacket *inflated;

    if (client->lzf == NULL)
        if ((client->lzf = _gekkota_xudpclient_get_lzf(client)) == NULL)
            return NULL;

    if ((inflated = gekkota_packet_new_1(
            gekkota_lzf_get_inflated_length(client->lzf, &packet->data),
            packet->flags)) == NULL)
        return NULL;

    if (gekkota_lzf_inflate(client->lzf, &packet->data, &inflated->data) < 0)
    {
        gekkota_packet_destroy(inflated);
        return NULL;
    }

    return inflated;
}

static GekkotaLZF *
_gekkota_xudpclient_get_lzf(const GekkotaXudpClient *client)
{
    GekkotaLZF *lzf = NULL;
    GekkotaXudpClient *currentClient;

    /*
     * Search for an existing LZF instance that provides
     * the required compression level.
     */
    for (currentClient = client->xudp->clients;
            currentClient < &client->xudp->clients[client->xudp->clientCount];
            currentClient++)
    {
        if (currentClient->lzf != NULL &&
                currentClient->compressionLevel == client->compressionLevel)
        {
            /*
             * LZF instance found: increase reference count.
             */
            lzf = gekkota_lzf_new_0(currentClient->lzf);
            break;
        }
    }

    if (lzf == NULL)
        /*
         * LZF instance not found: create it.
         */
        lzf = gekkota_lzf_new_1(client->compressionLevel);

    return lzf;
}
