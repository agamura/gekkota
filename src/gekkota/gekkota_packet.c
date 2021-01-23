/******************************************************************************
 * @file    gekkota_packet.c
 * @date    24-Jul-2007
 * @author  <a href="mailto:giuseppe.greco@agamura.com">Giuseppe Greco</a>
 *
 * Copyright (C) 2007 Agamura, Inc. - http://www.agamura.com
 * All right reserved.
 ******************************************************************************/

#include <errno.h>
#include "gekkota_errors.h"
#include "gekkota_memory.h"
#include "gekkota_packet.h"

GekkotaPacket *
gekkota_packet_new(size_t size)
{
    GekkotaPacket *packet;

    if (size == 0)
    {
        errno = GEKKOTA_ERROR_ARGUMENT_NOT_VALID;
        return NULL;
    }

    if ((packet = gekkota_memory_alloc(sizeof(GekkotaPacket), FALSE)) == NULL)
        return NULL;


    if (gekkota_buffer_malloc(&packet->data, size, FALSE) != 0)
    {
        gekkota_memory_free(packet);
        return NULL;
    }

    packet->flags = GEKKOTA_PACKET_FLAG_NONE;
    packet->refCount = 1;

    return packet;
}

GekkotaPacket *
gekkota_packet_new_0(GekkotaPacket *packet, bool_t deep)
{
    if (packet == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return NULL;
    }

    if (deep)
    {
        GekkotaPacket *newPacket;

        if ((newPacket = gekkota_packet_new(packet->data.length)) == NULL)
            return NULL;
        
        gekkota_packet_copy(newPacket, packet);
        return newPacket;
    }

    ++packet->refCount;
    return packet;
}

GekkotaPacket *
gekkota_packet_new_1(size_t size, GekkotaPacketFlag flags)
{
    GekkotaPacket *packet;

    if ((packet = gekkota_packet_new(size)) != NULL)
        packet->flags = flags;

    return packet;
}

GekkotaPacket *
gekkota_packet_new_2(const GekkotaBuffer *data, GekkotaPacketFlag flags)
{
    GekkotaPacket *packet;

    if (data == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return NULL;
    }

    if (data->data == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_BUFFER;
        return NULL;
    }

    if (data->length == 0)
    {
        errno = GEKKOTA_ERROR_ZERO_LENGTH_BUFFER;
        return NULL;
    }

    if ((packet = gekkota_packet_new(data->length)) == NULL)
        return NULL;

    gekkota_buffer_copy(&packet->data, data);
    packet->flags = flags;

    return packet;
}

int32_t
gekkota_packet_destroy(GekkotaPacket *packet)
{
    if (packet == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return -1;
    }

    if (--packet->refCount == 0)
    {
        gekkota_memory_free(packet->data.data);
        gekkota_memory_free(packet);
    }
    else
        return packet->refCount;

    return 0;
}

int32_t
gekkota_packet_copy(GekkotaPacket *restrict packet, const GekkotaPacket *value)
{
    if (packet == NULL || value == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return -1;
    }

    if (packet != value)
    {
        if (gekkota_buffer_copy(&packet->data, &value->data) == 0)
            packet->flags = value->flags;
        else
            return -1;
    }

    return 0;
}

int32_t
gekkota_packet_resize(GekkotaPacket *restrict packet, size_t newSize)
{
    if (packet == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return -1;
    }

    return gekkota_buffer_resize(&packet->data, newSize, FALSE);
}

bool_t
gekkota_packet_equals(const GekkotaPacket *packet, const GekkotaPacket *value)
{
    if (packet == value)
        return TRUE;

    if (packet == NULL || value == NULL)
        return FALSE;

    return packet->flags == value->flags &&
        gekkota_buffer_equals(&packet->data, &value->data);
}

GekkotaBuffer *
gekkota_packet_get_data(const GekkotaPacket *packet)
{
    if (packet == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return NULL;
    }

    return (GekkotaBuffer *) &packet->data;
}

int32_t
gekkota_packet_set_data(GekkotaPacket *restrict packet, const GekkotaBuffer *data)
{
    if (packet == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return -1;
    }

    return gekkota_buffer_copy(&packet->data, data);
}

GekkotaPacketFlag
gekkota_packet_get_flags(const GekkotaPacket *packet)
{
    if (packet == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return GEKKOTA_PACKET_FLAG_NONE;
    }

    return packet->flags;
}

int32_t
gekkota_packet_set_flags(GekkotaPacket *restrict packet, GekkotaPacketFlag flags)
{
    if (packet == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return -1;
    }

    packet->flags = flags;
    return 0;
}

size_t
gekkota_packet_get_size(const GekkotaPacket *packet)
{
    if (packet == NULL)
    {
        errno = GEKKOTA_ERROR_NULL_ARGUMENT;
        return 0;
    }

    return packet->data.length;
}
