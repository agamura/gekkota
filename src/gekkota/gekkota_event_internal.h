/******************************************************************************
 * @file    gekkota_event_internal.h
 * @date    27-Jul-2007
 * @author  <a href="mailto:giuseppe.greco@agamura.com">Giuseppe Greco</a>
 *
 * Copyright (C) 2007 Agamura, Inc. - http://www.agamura.com
 * All right reserved.
 ******************************************************************************/

#ifndef __GEKKOTA_EVENT_INTERNAL_H__
#define __GEKKOTA_EVENT_INTERNAL H__

#include "gekkota/gekkota_event.h"
#include "gekkota/gekkota_ipendpoint.h"
#include "gekkota/gekkota_packet.h"
#include "gekkota/gekkota_types.h"
#include "gekkota/gekkota_xudpclient.h"

struct _GekkotaEvent
{
    GekkotaEventType    type;
    GekkotaXudpClient   *client;
    uint8_t             channelId;
    GekkotaPacket       *packet;
    GekkotaIPEndPoint   *remoteEndPoint;
    uint32_t            refCount;
};

extern GekkotaEvent *
_gekkota_event_new(
        GekkotaEventType type,
        GekkotaXudpClient *client,
        uint8_t channelId,
        GekkotaPacket *packet,
        GekkotaIPEndPoint *remoteEndPoint,
        bool_t copy);

#endif /* !__GEKKOTA_EVENT_INTERNAL_H__ */
