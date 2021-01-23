/******************************************************************************
 * @file    gekkota_xudpclient_internal.h
 * @date    26-Jul-2007
 * @author  <a href="mailto:giuseppe.greco@agamura.com">Giuseppe Greco</a>
 *
 * Copyright (C) 2007 Agamura, Inc. - http://www.agamura.com
 * All right reserved.
 ******************************************************************************/

#ifndef __GEKKOTA_XUDPCLIENT_INTERNAL_H__
#define __GEKKOTA_XUDPCLIENT_INTERNAL_H__

#include "gekkota_xudp_internal.h"
#include "gekkota/gekkota_list.h"
#include "gekkota/gekkota_lzf.h"
#include "gekkota/gekkota_packet.h"
#include "gekkota/gekkota_ipendpoint.h"
#include "gekkota/gekkota_types.h"
#include "gekkota/gekkota_xudpclient.h"

#define GEKKOTA_XUDP_CLIENT_DEFAULT_ROUND_TRIP_TIME         500
#define GEKKOTA_XUDP_CLIENT_DEFAULT_PACKET_THROTTLE         32
#define GEKKOTA_XUDP_CLIENT_PACKET_THROTTLE_SCALE           32
#define GEKKOTA_XUDP_CLIENT_PACKET_THROTTLE_COUNTER         7 
#define GEKKOTA_XUDP_CLIENT_PACKET_THROTTLE_INTERVAL        5000
#define GEKKOTA_XUDP_CLIENT_PACKET_THROTTLE_ACCELERATION    2
#define GEKKOTA_XUDP_CLIENT_PACKET_THROTTLE_DECELERATION    2
#define GEKKOTA_XUDP_CLIENT_MAX_PACKET_SIZE                 2e6 * 1024
#define GEKKOTA_XUDP_CLIENT_WINDOW_SIZE_SCALE               64 * 1024
#define GEKKOTA_XUDP_CLIENT_MIN_TIMEOUT                     5000
#define GEKKOTA_XUDP_CLIENT_MAX_TIMEOUT                     30000
#define GEKKOTA_XUDP_CLIENT_TIMEOUT_LIMIT                   32
#define GEKKOTA_XUDP_CLIENT_PING_INTERVAL                   500
#define GEKKOTA_XUDP_CLIENT_UNSEQUENCED_WINDOW_SIZE         4 * 32

typedef struct _GekkotaChannel
{
    uint16_t                outgoingReliableSequenceNumber;
    uint16_t                outgoingUnreliableSequenceNumber;
    uint16_t                incomingReliableSequenceNumber;
    uint16_t                incomingUnreliableSequenceNumber;
    GekkotaList             incomingReliableMessages;
    GekkotaList             incomingUnreliableMessages;
} GekkotaChannel;

typedef struct _GekkotaAcknowledgement
{
    GekkotaListNode         listNode;
    uint32_t                sentTime;
    GekkotaXudpMessage      message;
} GekkotaAcknowledgement;

typedef struct _GekkotaOutgoingMessage
{
    GekkotaListNode         listNode;
    uint16_t                reliableSequenceNumber;
    uint16_t                unreliableSequenceNumber;
    uint32_t                sentTime;
    uint32_t                roundTripTimeout;
    uint32_t                roundTripTimeoutLimit;
    uint32_t                fragmentOffset;
    uint16_t                fragmentLength;
    GekkotaPacket           *packet;
    GekkotaXudpMessage      message;
} GekkotaOutgoingMessage;

typedef struct _GekkotaIncomingMessage
{  
    GekkotaListNode         listNode;
    uint16_t                reliableSequenceNumber;
    uint16_t                unreliableSequenceNumber;
    uint32_t                fragmentCount;
    uint32_t                fragmentsRemaining;
    uint32_t                *fragments;
    GekkotaPacket           *packet;
    GekkotaXudpMessage      message;
    GekkotaIPEndPoint       *remoteEndPoint;
} GekkotaIncomingMessage;

struct _GekkotaXudpClient
{
    struct _GekkotaXudp     *xudp;
    uint32_t                sessionId;
    uint16_t                localClientId;
    uint16_t                remoteClientId;
    GekkotaIPEndPoint       *remoteEndPoint;
    bool_t                  isMulticastGroupMember;
    uint32_t                multicastInterfaceIndex;
    GekkotaClientState      state;
    GekkotaChannel          *channels;
    uint8_t                 channelCount;
    GekkotaCompressionLevel compressionLevel;
    GekkotaLZF              *lzf;
    uint32_t                incomingBandwidth;
    uint32_t                outgoingBandwidth;
    uint32_t                incomingBandwidthThrottleEpoch;
    uint32_t                outgoingBandwidthThrottleEpoch;
    uint32_t                incomingDataTotal;
    uint32_t                outgoingDataTotal;
    uint32_t                lastReceiveTime;
    uint32_t                nextTimeout;
    uint32_t                earliestTimeout;
    uint32_t                packetThrottle;
    uint32_t                packetThrottleLimit;
    uint32_t                packetThrottleCounter;
    uint32_t                packetThrottleEpoch;
    uint32_t                packetThrottleAcceleration;
    uint32_t                packetThrottleDeceleration;
    uint32_t                packetThrottleInterval;
    uint32_t                roundTripTime;
    uint32_t                roundTripTimeVariance;
    uint32_t                lastRoundTripTime;
    uint32_t                lastRoundTripTimeVariance;
    uint32_t                lowestRoundTripTime;
    uint32_t                highestRoundTripTimeVariance;
    uint16_t                mtu;
    uint32_t                windowSize;
    uint32_t                reliableDataInTransit;
    uint16_t                outgoingReliableSequenceNumber;
    uint16_t                incomingUnsequencedGroup;
    uint16_t                outgoingUnsequencedGroup;
    uint32_t                unsequencedWindow[GEKKOTA_XUDP_CLIENT_UNSEQUENCED_WINDOW_SIZE / 32];
    GekkotaList             acknowledgements;
    GekkotaList             sentReliableMessages;
    GekkotaList             sentUnreliableMessages;
    GekkotaList             outgoingReliableMessages;
    GekkotaList             outgoingUnreliableMessages;
};

extern void_t
_gekkota_xudpclient_clear_incoming_message_queue(GekkotaList *queue);

extern void_t
_gekkota_xudpclient_clear_outgoing_message_queue(GekkotaList *queue);

extern int32_t
_gekkota_xudpclient_close_gracefully(GekkotaXudpClient *client);

extern int32_t
_gekkota_xudpclient_close_later(GekkotaXudpClient *client);

extern int32_t
_gekkota_xudpclient_close_now(GekkotaXudpClient *client);

extern void_t
_gekkota_xudpclient_clear_message_queues(GekkotaXudpClient *client);

extern int32_t
_gekkota_xudpclient_queue_acknowledgement(
        GekkotaXudpClient *restrict client,
        const GekkotaXudpMessage *message,
        uint32_t sentTime,
        GekkotaAcknowledgement **acknowledgement);

extern int32_t
_gekkota_xudpclient_queue_incoming_message(
        GekkotaXudpClient *client,
        const GekkotaXudpMessage *message,
        GekkotaPacket *packet,
        uint32_t fragmentCount,
        GekkotaIPEndPoint *remoteEndPoint,
        GekkotaIncomingMessage **incomingMessage);

extern int32_t
_gekkota_xudpclient_queue_outgoing_message(
        GekkotaXudpClient *restrict client,
        const GekkotaXudpMessage *message,
        GekkotaPacket *packet,
        uint32_t offset,
        uint16_t length,
        GekkotaOutgoingMessage **outgoingMessage);

extern void_t
_gekkota_xudpclient_reset(GekkotaXudpClient *restrict client);

extern int32_t
_gekkota_xudpclient_throttle(
        GekkotaXudpClient *restrict client,
        uint32_t roundTripTime);

#endif /* !__GEKKOTA_XUDPCLIENT_INTERNAL_H__ */
