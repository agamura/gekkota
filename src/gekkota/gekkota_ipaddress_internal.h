/******************************************************************************
 * @file    gekkota_ipaddress_internal.h
 * @date    27-Oct-2006
 * @author  <a href="mailto:giuseppe.greco@agamura.com">Giuseppe Greco</a>
 *
 * Copyright (C) 2006 Agamura, Inc. - http://www.agamura.com
 * All right reserved.
 ******************************************************************************/

#ifndef __GEKKOTA_IPADDRESS_INTERNAL_H__
#define __GEKKOTA_IPADDRESS_INTERNAL_H__

#include "gekkota/gekkota_buffer.h"
#include "gekkota/gekkota_types.h"

struct _GekkotaIPAddress
{
    union
    {
        uint8_t     word8[16];
        uint16_t    word16[8];
        uint32_t    word32[4];
    } ipAddress;

#define ipAddress8  ipAddress.word8
#define ipAddress16 ipAddress.word16
#define ipAddress32 ipAddress.word32

    uint16_t        family;
    uint32_t        scopeID;
    uint32_t        refCount;
};

extern int32_t
_gekkota_ipaddress_string_to_address(
        const char_t *ipString,
        uint16_t family,
        void_t *address);

extern int32_t
_gekkota_ipaddress_address_to_string(
        const void_t *address,
        uint16_t family,
        GekkotaBuffer *restrict ipString);

#endif /* !__GEKKOTA_IPADDRESS_INTERNAL_H__ */
