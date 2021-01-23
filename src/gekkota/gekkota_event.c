/******************************************************************************
 * @file    gekkota_event.c
 * @date    27-Jul-2007
 * @author  <a href="mailto:giuseppe.greco@agamura.com">Giuseppe Greco</a>
 *
 * Copyright (C) 2007 Agamura, Inc. - http://www.agamura.com
 * All right reserved.
 ******************************************************************************/

#include <errno.h>
#include "gekkota_memory.h"
#include "gekkota_errors.h"
#include "gekkota_event.h"

GekkotaEvent *
gekkota_event_new_0(GekkotaEvent *restrict event)
{
    if (event == NULL)
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
    else
        ++event->refCount;

    return event;
}
        
int32_t
gekkota_event_destroy(GekkotaEvent *event)
{
    if (event == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return -1;
    }

    if (--event->refCount == 0)
    {
        if (event->packet != NULL)
            gekkota_packet_destroy(event->packet);

        if (event->remoteEndPoint != NULL)
            gekkota_ipendpoint_destroy(event->remoteEndPoint);

        gekkota_memory_free(event);
    }
    else
        return event->refCount;

    return 0;
}

GekkotaEventType
gekkota_event_get_type(const GekkotaEvent *event)
{
    if (event == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return GEKKOTA_EVENT_TYPE_UNDEFINED;
    }

    return event->type;
}

GekkotaXudpClient *
gekkota_event_get_client(const GekkotaEvent *event)
{
    if (event == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return NULL;
    }

    return event->client;
}

int32_t
gekkota_event_get_channel_id(const GekkotaEvent *event)
{
    if (event == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return -1;
    }

    return (int32_t) event->channelId;
}

GekkotaPacket *
gekkota_event_get_packet(const GekkotaEvent *event)
{
    if (event == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return NULL;
    }

    return event->packet;
}

GekkotaIPEndPoint *
gekkota_event_get_remote_endpoint(const GekkotaEvent *event)
{
    if (event == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return NULL;
    }

    return event->remoteEndPoint;
}

GekkotaEvent *
_gekkota_event_new(
        GekkotaEventType type,
        GekkotaXudpClient *client,
        uint8_t channelId,
        GekkotaPacket *packet,
        GekkotaIPEndPoint *remoteEndPoint,
        bool_t copy)
{
    GekkotaEvent *event;

    if ((event = gekkota_memory_alloc(sizeof(GekkotaEvent), FALSE)) == NULL)
        return NULL;

    event->type = type;
    event->client = client;
    event->channelId = (uint8_t) (channelId & 0x000000FF);
    
    if ((event->packet = packet) != NULL && copy)
        event->packet = gekkota_packet_new_0(event->packet, FALSE);
    
    if ((event->remoteEndPoint = remoteEndPoint) != NULL && copy)
        event->remoteEndPoint = gekkota_ipendpoint_new_0(event->remoteEndPoint, FALSE);

    event->refCount = 1;
    return event;
}
