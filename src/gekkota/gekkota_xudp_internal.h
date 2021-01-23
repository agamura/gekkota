/******************************************************************************
 * @file    gekkota_xudp_internal.h
 * @date    26-Jul-2007
 * @author  <a href="mailto:giuseppe.greco@agamura.com">Giuseppe Greco</a>
 *
 * Copyright (C) 2007 Agamura, Inc. - http://www.agamura.com
 * All right reserved.
 ******************************************************************************/

#ifndef __GEKKOTA_XUDP_INTERNAL_H__
#define __GEKKOTA_XUDP_INTERNAL_H__

#include "gekkota/gekkota_buffer.h"
#include "gekkota/gekkota_ipendpoint.h"
#include "gekkota/gekkota_socket.h"
#include "gekkota/gekkota_types.h"
#include "gekkota/gekkota_xudpclient.h"

#define GEKKOTA_XUDP_ID                             "XUDP"
#define GEKKOTA_XUDP_VERSION                        1
#define GEKKOTA_XUDP_MIN_MTU                        576
#define GEKKOTA_XUDP_MAX_MTU                        4096
#define GEKKOTA_XUDP_DEFAULT_MTU                    1400
#define GEKKOTA_XUDP_MAX_MESSAGES                   32
#define GEKKOTA_XUDP_MIN_WINDOW_SIZE                4096
#define GEKKOTA_XUDP_MAX_WINDOW_SIZE                32768
#define GEKKOTA_XUDP_DEFAULT_CHANNEL_COUNT          1
#define GEKKOTA_XUDP_DEFAULT_CLIENT_COUNT           16
#define GEKKOTA_XUDP_DEFAULT_POLL_TIMEOUT           1000
#define GEKKOTA_XUDP_BANDWIDTH_THROTTLE_INTERVAL    1000

#ifndef GEKKOTA_XUDP_MAX_BUFFERS
#define GEKKOTA_XUDP_MAX_BUFFERS (1 + 2 * GEKKOTA_XUDP_MAX_MESSAGES)
#endif /* !GEKKOTA_XUDP_MAX_BUFFERS */

typedef enum
{
    GEKKOTA_XUDP_MESSAGE_TYPE_UNDEFINED             = 0x00000000,
    GEKKOTA_XUDP_MESSAGE_TYPE_ACKNOWLEDGE           = 0x00000001,
    GEKKOTA_XUDP_MESSAGE_TYPE_CONNECT               = 0x00000002,
    GEKKOTA_XUDP_MESSAGE_TYPE_VALIDATE_CONNECT      = 0x00000003,
    GEKKOTA_XUDP_MESSAGE_TYPE_DISCONNECT            = 0x00000004,
    GEKKOTA_XUDP_MESSAGE_TYPE_JOIN_MULTICAST_GROUP  = 0x00000005,
    GEKKOTA_XUDP_MESSAGE_TYPE_LEAVE_MULTICAST_GROUP = 0x00000006,
    GEKKOTA_XUDP_MESSAGE_TYPE_PING                  = 0x00000007,
    GEKKOTA_XUDP_MESSAGE_TYPE_RELIABLE_DATA         = 0x00000008,
    GEKKOTA_XUDP_MESSAGE_TYPE_UNRELIABLE_DATA       = 0x00000009,
    GEKKOTA_XUDP_MESSAGE_TYPE_UNSEQUENCED_DATA      = 0x0000000A,
    GEKKOTA_XUDP_MESSAGE_TYPE_DATA_FRAGMENT         = 0x0000000B,
    GEKKOTA_XUDP_MESSAGE_TYPE_CONFIGURE_BANDWIDTH   = 0x0000000C,
    GEKKOTA_XUDP_MESSAGE_TYPE_CONFIGURE_THROTTLE    = 0x0000000D
} GekkotaXudpMessageType;

typedef enum
{
    GEKKOTA_XUDP_HEADER_FLAG_NONE                   = 0,
    GEKKOTA_XUDP_HEADER_FLAG_SENT_TIME              = (1 << 0),
    GEKKOTA_XUDP_HEADER_FLAG_MULTICAST              = (1 << 1),
    GEKKOTA_XUDP_HEADER_FLAG_MASK                   = 0x03
} GekkotaXudpHeaderFlag;

typedef enum
{
    GEKKOTA_XUDP_MESSAGE_FLAG_NONE                  = 0,
    GEKKOTA_XUDP_MESSAGE_FLAG_ACKNOWLEDGE           = (1 << 0),
    GEKKOTA_XUDP_MESSAGE_FLAG_UNSEQUENCED           = (1 << 1),
    GEKKOTA_XUDP_MESSAGE_FLAG_COMPRESSED            = (1 << 2),
    GEKKOTA_XUDP_MESSAGE_FLAG_ENCRYPTED             = (1 << 3)
} GekkotaXudpMessageFlag;

typedef struct _GekkotaXudpHeader
{
    uint16_t                protocolId;
    uint8_t                 version;                /* 6 LSB: protocol version */
                                                    /* 2 MSB: header flags */
    uint32_t                sessionId;
    uint16_t                clientId;
    uint16_t                sentTime;
} GekkotaXudpHeader;

typedef struct _GekkotaXudpMessageHeader
{
    uint8_t                 messageType;
    uint8_t                 channelId;
    uint8_t                 flags;
    uint16_t                sequenceNumber;
} GekkotaXudpMessageHeader;

typedef struct _GekkotaXudpAcknowledgeMessage
{
    GekkotaXudpMessageHeader header;
    uint16_t                sequenceNumber;
    uint16_t                sentTime;
} GekkotaXudpAcknowledgeMessage;

typedef struct _GekkotaXudpConnectMessage
{
    GekkotaXudpMessageHeader header;
    uint16_t                clientId;
    uint32_t                sessionId;
    uint8_t                 channelCount;
    uint16_t                mtu;
    uint32_t                windowSize;
    uint32_t                incomingBandwidth;
    uint32_t                outgoingBandwidth;
    uint32_t                throttleInterval;
    uint32_t                throttleAcceleration;
    uint32_t                throttleDeceleration;
    uint8_t                 compressionLevel;
} GekkotaXudpConnectMessage;

typedef struct _GekkotaXudpValidateConnectMessage
{
    GekkotaXudpMessageHeader header;
    uint16_t                clientId;
    uint8_t                 channelCount;
    uint16_t                mtu;
    uint32_t                windowSize;
    uint32_t                incomingBandwidth;
    uint32_t                outgoingBandwidth;
    uint32_t                throttleInterval;
    uint32_t                throttleAcceleration;
    uint32_t                throttleDeceleration;
    uint8_t                 compressionLevel;
} GekkotaXudpValidateConnectMessage;

typedef struct _GekkotaXudpDisconnectMessage
{
    GekkotaXudpMessageHeader header;
} GekkotaXudpDisconnectMessage;

typedef struct _GekkotaXudpJoinMulticastGroupMessage
{
    GekkotaXudpMessageHeader header;
} GekkotaXudpJoinMulticastGroupMessage;

typedef struct _GekkotaXudpLeaveMulticastGroupMessage
{
    GekkotaXudpMessageHeader header;
} GekkotaXudpLeaveMulticastGroupMessage;

typedef struct _GekkotaXudpPingMessage
{
    GekkotaXudpMessageHeader header;
} GekkotaXudpPingMessage;

typedef struct _GekkotaXudpReliabledDataMessage
{
    GekkotaXudpMessageHeader header;
    uint16_t                length;
} GekkotaXudpReliableDataMessage;

typedef struct _GekkotaXudpUnreliableDataMessage
{
    GekkotaXudpMessageHeader header;
    uint16_t                sequenceNumber;
    uint16_t                length;
} GekkotaXudpUnreliableDataMessage;

typedef struct _GekkotaXudpUnsequencedDataMessage
{
    GekkotaXudpMessageHeader header;
    uint16_t                group;
    uint16_t                length;
} GekkotaXudpUnsequencedDataMessage;

typedef struct _GekkotaXudpDataFragmentMessage
{
    GekkotaXudpMessageHeader header;
    uint16_t                startSequenceNumber;
    uint32_t                fragmentCount;
    uint32_t                fragmentNumber;
    uint32_t                fragmentOffset;
    uint32_t                totalLength;
    uint16_t                length;
} GekkotaXudpDataFragmentMessage;

typedef struct _GekkotaXudpConfigureBandwidthMessage
{
    GekkotaXudpMessageHeader header;
    uint32_t                incomingBandwidth;
    uint32_t                outgoingBandwidth;
} GekkotaXudpConfigureBandwidthMessage;

typedef struct _GekkotaXudpConfigureThrottleMessage
{
    GekkotaXudpMessageHeader header;
    uint32_t                throttleInterval;
    uint32_t                throttleAcceleration;
    uint32_t                throttleDeceleration;
} GekkotaXudpConfigureThrottleMessage;

typedef union
{
    GekkotaXudpMessageHeader                header;
    GekkotaXudpAcknowledgeMessage           acknowledge;
    GekkotaXudpConnectMessage               connect;
    GekkotaXudpValidateConnectMessage       validateConnect;
    GekkotaXudpDisconnectMessage            disconnect;
    GekkotaXudpJoinMulticastGroupMessage    joinMulticastGroup;
    GekkotaXudpLeaveMulticastGroupMessage   leaveMulticastGroup;
    GekkotaXudpPingMessage                  ping;
    GekkotaXudpReliableDataMessage          reliableData;
    GekkotaXudpUnreliableDataMessage        unreliableData;
    GekkotaXudpUnsequencedDataMessage       unsequencedData;
    GekkotaXudpDataFragmentMessage          dataFragment;
    GekkotaXudpConfigureBandwidthMessage    configureBandwidth;
    GekkotaXudpConfigureThrottleMessage     configureThrottle;
} GekkotaXudpMessage;

struct _GekkotaXudp
{
    uint16_t                protocolId;
    GekkotaSocket           *socket;
    uint32_t                currentTime;
    uint32_t                incomingBandwidth;
    uint32_t                outgoingBandwidth;
    uint32_t                bandwidthThrottleEpoch;
    uint16_t                mtu;
    bool_t                  reconfigureBandwidth;
    GekkotaXudpClient       *clients;
    uint16_t                clientCount;
    GekkotaXudpClient       *lastServicedClient;
    size_t                  packetSize;
    uint8_t                 headerFlags;
    GekkotaXudpMessage      messages[GEKKOTA_XUDP_MAX_MESSAGES];
    uint16_t                messageCount;
    GekkotaBuffer           buffers[GEKKOTA_XUDP_MAX_BUFFERS];
    uint16_t                bufferCount;
    GekkotaIPEndPoint       *remoteEndPoint;
    byte_t                  receivedData[GEKKOTA_XUDP_MAX_MTU];
    size_t                  receivedDataLength;
};

extern size_t
_gekkota_xudp_message_size(GekkotaXudpMessageType messageType);

#endif /* !__GEKKOTA_XUDP_INTERNAL_H__ */
