/******************************************************************************
 * @file    gekkota_packet_internal.h
 * @date    24-Jul-2007
 * @author  <a href="mailto:giuseppe.greco@agamura.com">Giuseppe Greco</a>
 *
 * Copyright (C) 2007 Agamura, Inc. - http://www.agamura.com
 * All right reserved.
 ******************************************************************************/

#ifndef __GEKKOTA_PACKET_INTERNAL_H__
#define __GEKKOTA_PACKET_INTERNAL_H__

#include "gekkota/gekkota_buffer.h"
#include "gekkota/gekkota_types.h"

struct _GekkotaPacket
{
    GekkotaBuffer       data;
    GekkotaPacketFlag   flags;
    uint32_t            refCount;
};

#endif /* !__GEKKOTA_PACKET_INTERNAL_H__ */
