/******************************************************************************
 * @file    gekkota_iphostentry.h
 * @date    09-Jan-2007
 * @author  <a href="mailto:giuseppe.greco@agamura.com">Giuseppe Greco</a>
 *
 * Copyright (C) 2007 Agamura, Inc. - http://www.agamura.com
 * All right reserved.
 ******************************************************************************/

#ifndef __GEKKOTA_IPHOSTENTRY_H__
#define __GEKKOTA_IPHOSTENTRY_H__

#include "gekkota/gekkota_ipaddress.h"
#include "gekkota/gekkota_list.h"
#include "gekkota/gekkota_types.h"

typedef struct _GekkotaIPHostAddress GekkotaIPHostAddress;
typedef struct _GekkotaIPHostAlias GekkotaIPHostAlias;
typedef struct _GekkotaIPHostEntry GekkotaIPHostEntry;

GEKKOTA_API GekkotaIPHostEntry *
gekkota_iphostentry_new_0(GekkotaIPHostEntry *restrict hostEntry);

GEKKOTA_API int32_t
gekkota_iphostentry_destroy(GekkotaIPHostEntry *hostEntry);

GEKKOTA_API char_t *
gekkota_iphostentry_get_host_name(const GekkotaIPHostEntry *hostEntry);

GEKKOTA_API GekkotaList *
gekkota_iphostentry_get_addresses(const GekkotaIPHostEntry *hostEntry);

GEKKOTA_API GekkotaList *
gekkota_iphostentry_get_aliases(const GekkotaIPHostEntry *hostEntry);

GEKKOTA_API GekkotaIPAddress *
gekkota_iphostentry_address(GekkotaIPHostAddress *restrict hostAddress);

GEKKOTA_API char_t *
gekkota_iphostentry_alias(GekkotaIPHostAlias *restrict alias);

#if defined (GEKKOTA_BUILDING_LIB) || defined (GEKKOTA_BUILDING_STATIC_LIB)
#include "gekkota_iphostentry_internal.h"
#endif /* GEKKOTA_BUILDING_LIB || GEKKOTA_BUILDING_STATIC_LIB */

#endif /* !__GEKKOTA_IPHOSTENTRY_H__ */
