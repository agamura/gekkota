/******************************************************************************
 * @file    gekkota.h
 * @date    27-Oct-2006
 * @author  <a href="mailto:giuseppe.greco@agamura.com">Giuseppe Greco</a>
 *
 * Copyright (C) 2006 Agamura, Inc. - http://www.agamura.com
 * All right reserved.
 ******************************************************************************/

#ifndef __GEKKOTA_H__
#define __GEKKOTA_H__

#include "gekkota/gekkota_types.h"

GEKKOTA_API int32_t
gekkota_initialize(void_t);

GEKKOTA_API int32_t
gekkota_uninitialize(void_t);

GEKKOTA_API bool_t
gekkota_is_initialized(void_t);

GEKKOTA_API time_t
gekkota_get_time(void_t);

GEKKOTA_API int32_t
gekkota_set_time(time_t time);

GEKKOTA_API uint16_t
gekkota_host_to_net_16(uint16_t host);

GEKKOTA_API uint32_t
gekkota_host_to_net_32(uint32_t host);

GEKKOTA_API uint64_t
gekkota_host_to_net_64(uint64_t host);

GEKKOTA_API uint16_t
gekkota_net_to_host_16(uint16_t net);

GEKKOTA_API uint32_t
gekkota_net_to_host_32(uint32_t net);

GEKKOTA_API uint64_t
gekkota_net_to_host_64(uint64_t net);

GEKKOTA_API uint16_t
gekkota_hash_16(const char_t *string);

GEKKOTA_API uint32_t
gekkota_hash_32(const char_t *string);

GEKKOTA_API int32_t
gekkota_get_last_error();

#if ! defined (GEKKOTA_BUILDING_LIB) && ! defined (GEKKOTA_BUILDING_STATIC_LIB)
#include "gekkota/gekkota_bit.h"
#include "gekkota/gekkota_buffer.h"
#include "gekkota/gekkota_dns.h"
#include "gekkota/gekkota_errors.h"
#include "gekkota/gekkota_event.h"
#include "gekkota/gekkota_memory.h"
#include "gekkota/gekkota_ipaddress.h"
#include "gekkota/gekkota_ipendpoint.h"
#include "gekkota/gekkota_iphostentry.h"
#include "gekkota/gekkota_list.h"
#include "gekkota/gekkota_lzf.h"
#include "gekkota/gekkota_module.h"
#include "gekkota/gekkota_networkinterface.h"
#include "gekkota/gekkota_packet.h"
#include "gekkota/gekkota_platform.h"
#include "gekkota/gekkota_socket.h"
#include "gekkota/gekkota_string.h"
#include "gekkota/gekkota_time.h"
#include "gekkota/gekkota_xudp.h"
#include "gekkota/gekkota_xudpclient.h"
#else
#include "gekkota_internal.h"
#endif /* !GEKKOTA_BUILDING_LIB && !GEKKOTA_BUILDING_STATIC_LIB */

#endif /* !__GEKKOTA_H__ */
