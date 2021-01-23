/******************************************************************************
 * @file    gekkota_packet.h
 * @date    24-Jul-2007
 * @author  <a href="mailto:giuseppe.greco@agamura.com">Giuseppe Greco</a>
 *
 * Copyright (C) 2007 Agamura, Inc. - http://www.agamura.com
 * All right reserved.
 ******************************************************************************/

#ifndef __GEKKOTA_PACKET_H__
#define __GEKKOTA_PACKET_H__

#include "gekkota/gekkota_buffer.h"
#include "gekkota/gekkota_types.h"

typedef enum
{
    GEKKOTA_PACKET_FLAG_NONE        = 0,
    GEKKOTA_PACKET_FLAG_RELIABLE    = (1 << 0),
    GEKKOTA_PACKET_FLAG_UNSEQUENCED = (1 << 1),
    GEKKOTA_PACKET_FLAG_COMPRESSED  = (1 << 2),
    GEKKOTA_PACKET_FLAG_ENCRYPTED   = (1 << 3)
} GekkotaPacketFlag;

typedef struct _GekkotaPacket GekkotaPacket;

GEKKOTA_API GekkotaPacket *
gekkota_packet_new(size_t size);

GEKKOTA_API GekkotaPacket *
gekkota_packet_new_0(GekkotaPacket *packet, bool_t deep);

GEKKOTA_API GekkotaPacket *
gekkota_packet_new_1(size_t size, GekkotaPacketFlag flags);

GEKKOTA_API GekkotaPacket *
gekkota_packet_new_2(const GekkotaBuffer *data, GekkotaPacketFlag flags);

GEKKOTA_API int32_t
gekkota_packet_destroy(GekkotaPacket *packet);

GEKKOTA_API int32_t
gekkota_packet_copy(GekkotaPacket *restrict packet, const GekkotaPacket *value);

GEKKOTA_API int32_t
gekkota_packet_resize(GekkotaPacket *restrict packet, size_t newSize);

GEKKOTA_API bool_t
gekkota_packet_equals(const GekkotaPacket *packet, const GekkotaPacket *value);

GEKKOTA_API GekkotaBuffer *
gekkota_packet_get_data(const GekkotaPacket *packet);

GEKKOTA_API int32_t
gekkota_packet_set_data(GekkotaPacket *restrict packet, const GekkotaBuffer *data);

GEKKOTA_API GekkotaPacketFlag
gekkota_packet_get_flags(const GekkotaPacket *packet);

GEKKOTA_API int32_t
gekkota_packet_set_flags(GekkotaPacket *restrict packet, GekkotaPacketFlag flags);

GEKKOTA_API size_t
gekkota_packet_get_size(const GekkotaPacket *packet);

#if defined (GEKKOTA_BUILDING_LIB) || defined (GEKKOTA_BUILDING_STATIC_LIB)
#include "gekkota_packet_internal.h"
#endif /* GEKKOTA_BUILDING_LIB || GEKKOTA_BUILDING_STATIC_LIB */

#endif /* !__GEKKOTA_PACKET_H__ */
