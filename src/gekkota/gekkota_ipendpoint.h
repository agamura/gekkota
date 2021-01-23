/******************************************************************************
 * @file    gekkota_ipendpoint.h
 * @date    27-Oct-2006
 * @author  <a href="mailto:giuseppe.greco@agamura.com">Giuseppe Greco</a>
 *
 * Copyright (C) 2006 Agamura, Inc. - http://www.agamura.com
 * All right reserved.
 ******************************************************************************/

#ifndef __GEKKOTA_IPENDPOINT_H__
#define __GEKKOTA_IPENDPOINT_H__

#include "gekkota/gekkota_ipaddress.h"
#include "gekkota/gekkota_types.h"

#define GEKKOTA_SIZEOF_SOCKADDR_IN6 28

typedef struct _GekkotaIPEndPoint GekkotaIPEndPoint;

typedef struct _GekkotaSocketAddress
{
    uint16_t            family;
    uint16_t            port;
    byte_t              data[GEKKOTA_SIZEOF_SOCKADDR_IN6 -
        /* family */    sizeof(uint16_t) -
        /* port   */    sizeof(uint16_t)];
} GekkotaSocketAddress;

GEKKOTA_API GekkotaIPEndPoint *
gekkota_ipendpoint_new(GekkotaIPAddress *address, uint16_t port);

GEKKOTA_API GekkotaIPEndPoint *
gekkota_ipendpoint_new_0(GekkotaIPEndPoint *restrict endPoint, bool_t deep);

GEKKOTA_API GekkotaIPEndPoint *
gekkota_ipendpoint_new_1(GekkotaAddressFamily addressFamily, uint16_t port);

GEKKOTA_API GekkotaIPEndPoint *
gekkota_ipendpoint_new_2(const GekkotaSocketAddress *socketAddress);

GEKKOTA_API int32_t
gekkota_ipendpoint_destroy(GekkotaIPEndPoint *endPoint);

GEKKOTA_API int32_t
gekkota_ipendpoint_copy(
        GekkotaIPEndPoint *restrict endPoint,
        GekkotaIPEndPoint *value);

GEKKOTA_API bool_t
gekkota_ipendpoint_equals(
        const GekkotaIPEndPoint *endPoint,
        const GekkotaIPEndPoint *value);

GEKKOTA_API GekkotaIPAddress *
gekkota_ipendpoint_get_address(const GekkotaIPEndPoint *endPoint);

GEKKOTA_API int32_t
gekkota_ipendpoint_set_address(
        GekkotaIPEndPoint *restrict endPoint,
        GekkotaIPAddress *address);

GEKKOTA_API int32_t
gekkota_ipendpoint_get_port(const GekkotaIPEndPoint *endPoint);

GEKKOTA_API int32_t
gekkota_ipendpoint_set_port(GekkotaIPEndPoint *restrict endPoint, uint16_t port);

GEKKOTA_API int32_t
gekkota_ipendpoint_to_socketaddress(
        const GekkotaIPEndPoint *endPoint,
        GekkotaSocketAddress *socketAddress);

#if defined (GEKKOTA_BUILDING_LIB) || defined (GEKKOTA_BUILDING_STATIC_LIB)
#include "gekkota_ipendpoint_internal.h"
#endif /* GEKKOTA_BUILDING_LIB || GEKKOTA_BUILDING_STATIC_LIB */

#endif /* !__GEKKOTA_IPENDPOINT_H__ */
