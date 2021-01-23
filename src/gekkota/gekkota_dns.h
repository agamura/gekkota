/*******************************************************************************
 * @file    gekkota_dns.h
 * @date    09-Jan-2007
 * @author  <a href="mailto:giuseppe.greco@agamura.com">Giuseppe Greco</a>
 *
 * Copyright (C) 2007 Agamura, Inc. - http://www.agamura.com
 * All right reserved.
 ******************************************************************************/

#ifndef __GEKKOTA_DNS_H__
#define __GEKKOTA_DNS_H__

#include "gekkota/gekkota_buffer.h"
#include "gekkota/gekkota_ipaddress.h"
#include "gekkota/gekkota_iphostentry.h"
#include "gekkota/gekkota_types.h"

GEKKOTA_API GekkotaIPHostEntry *
gekkota_dns_get_host_entry(const char_t *hostNameOrAddress);

GEKKOTA_API GekkotaIPHostEntry *
gekkota_dns_get_host_entry_1(const GekkotaIPAddress *address);

GEKKOTA_API int32_t
gekkota_dns_get_host_name(GekkotaBuffer *restrict hostName);

#endif /* !__GEKKOTA_DNS_H__ */
