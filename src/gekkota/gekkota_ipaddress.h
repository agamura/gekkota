/******************************************************************************
 * @file    gekkota_ipaddress.h
 * @date    27-Oct-2006
 * @author  <a href="mailto:giuseppe.greco@agamura.com">Giuseppe Greco</a>
 *
 * Copyright (C) 2006 Agamura, Inc. - http://www.agamura.com
 * All right reserved.
 ******************************************************************************/

#ifndef __GEKKOTA_IPADDRESS_H__
#define __GEKKOTA_IPADDRESS_H__

#include "gekkota/gekkota_buffer.h"
#include "gekkota/gekkota_types.h"

typedef enum
{
    GEKKOTA_ADDRESS_FAMILY_UNDEFINED    = 0,
    GEKKOTA_ADDRESS_FAMILY_INET         = 2,
    GEKKOTA_ADDRESS_FAMILY_INET6        = 23
} GekkotaAddressFamily;

typedef struct _GekkotaIPAddress GekkotaIPAddress;

GEKKOTA_API const byte_t *gekkota_ipaddress_unspecified;
GEKKOTA_API const byte_t *gekkota_ipaddress_loopback;

GEKKOTA_API GekkotaIPAddress *
gekkota_ipaddress_new(const char_t *ipString);

GEKKOTA_API GekkotaIPAddress *
gekkota_ipaddress_new_0(GekkotaIPAddress *address, bool_t deep);

GEKKOTA_API GekkotaIPAddress *
gekkota_ipaddress_new_1(const GekkotaBuffer *hostByteOrderAddress);

GEKKOTA_API GekkotaIPAddress *
gekkota_ipaddress_new_2(const GekkotaBuffer *netByteOrderAddress);

GEKKOTA_API int32_t
gekkota_ipaddress_destroy(GekkotaIPAddress *address);

GEKKOTA_API int32_t
gekkota_ipaddress_copy(
        GekkotaIPAddress *address,
        const GekkotaIPAddress *value);

GEKKOTA_API GekkotaAddressFamily
gekkota_ipaddress_get_family(const GekkotaIPAddress *address);

GEKKOTA_API bool_t
gekkota_ipaddress_equals(
        const GekkotaIPAddress *address,
        const GekkotaIPAddress *value);

GEKKOTA_API bool_t
gekkota_ipaddress_is_ipv4_compatible(const GekkotaIPAddress *address);

GEKKOTA_API bool_t
gekkota_ipaddress_is_ipv4_mapped(const GekkotaIPAddress *address);

GEKKOTA_API bool_t
gekkota_ipaddress_is_broadcast(const GekkotaIPAddress *address);

GEKKOTA_API bool_t
gekkota_ipaddress_is_multicast(const GekkotaIPAddress *address);

GEKKOTA_API bool_t
gekkota_ipaddress_is_loopback(const GekkotaIPAddress *address);

GEKKOTA_API bool_t
gekkota_ipaddress_is_unspecified(const GekkotaIPAddress *address);

GEKKOTA_API byte_t *
gekkota_ipaddress_get_bytes(
        const GekkotaIPAddress *address,
        size_t *restrict addressLength);

GEKKOTA_API uint32_t
gekkota_ipaddress_get_scope_id(const GekkotaIPAddress *address);

GEKKOTA_API int32_t
gekkota_ipaddress_set_scope_id(
        GekkotaIPAddress *restrict address,
        uint32_t scopeID);

GEKKOTA_API int32_t
gekkota_ipaddress_to_string(
        const GekkotaIPAddress *address,
        GekkotaBuffer *ipString);

#if defined (GEKKOTA_BUILDING_LIB) || defined (GEKKOTA_BUILDING_STATIC_LIB)
#include "gekkota_ipaddress_internal.h"
#endif /* GEKKOTA_BUILDING_LIB || GEKKOTA_BUILDING_STATIC_LIB */

#endif /* !__GEKKOTA_IPADDRESS_H__ */
