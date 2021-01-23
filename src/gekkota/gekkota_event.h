/******************************************************************************
 * @file    gekkota_event.h
 * @date    27-Jul-2007
 * @author  <a href="mailto:giuseppe.greco@agamura.com">Giuseppe Greco</a>
 *
 * Copyright (C) 2007 Agamura, Inc. - http://www.agamura.com
 * All right reserved.
 ******************************************************************************/

#ifndef __GEKKOTA_EVENT_H__
#define __GEKKOTA_EVENT_H__

#include "gekkota/gekkota_ipendpoint.h"
#include "gekkota/gekkota_packet.h"
#include "gekkota/gekkota_types.h"
#include "gekkota/gekkota_xudpclient.h"

typedef enum
{
    GEKKOTA_EVENT_TYPE_UNDEFINED                = 0x00000000,
    GEKKOTA_EVENT_TYPE_CONNECT                  = 0x00000001,
    GEKKOTA_EVENT_TYPE_DISCONNECT               = 0x00000002,
    GEKKOTA_EVENT_TYPE_JOIN_MULTICAST_GROUP     = 0x00000003,
    GEKKOTA_EVENT_TYPE_LEAVE_MULTICAST_GROUP    = 0x00000004,
    GEKKOTA_EVENT_TYPE_RECEIVE                  = 0x00000005
} GekkotaEventType;

typedef struct _GekkotaEvent GekkotaEvent;

GEKKOTA_API GekkotaEvent *
gekkota_event_new_0(GekkotaEvent *restrict event);
        
GEKKOTA_API int32_t
gekkota_event_destroy(GekkotaEvent *event);

GEKKOTA_API GekkotaEventType
gekkota_event_get_type(const GekkotaEvent *event);

GEKKOTA_API GekkotaXudpClient *
gekkota_event_get_client(const GekkotaEvent *event);

GEKKOTA_API int32_t
gekkota_event_get_channel_id(const GekkotaEvent *event);

GEKKOTA_API GekkotaPacket *
gekkota_event_get_packet(const GekkotaEvent *event);

GEKKOTA_API GekkotaIPEndPoint *
gekkota_event_get_remote_endpoint(const GekkotaEvent *event);

#if defined (GEKKOTA_BUILDING_LIB) || defined (GEKKOTA_BUILDING_STATIC_LIB)
#include "gekkota_event_internal.h"
#endif /* GEKKOTA_BUILDING_LIB || GEKKOTA_BUILDING_STATIC_LIB */

#endif /* !__GEKKOTA_EVENT_H__ */
