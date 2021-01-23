/******************************************************************************
 * @file    gekkota_ipendpoint_internal.h
 * @date    27-Oct-2006
 * @author  <a href="mailto:giuseppe.greco@agamura.com">Giuseppe Greco</a>
 *
 * Copyright (C) 2006 Agamura, Inc. - http://www.agamura.com
 * All right reserved.
 ******************************************************************************/

#ifndef __GEKKOTA_IPENDPOINT_INTERNAL_H__
#define __GEKKOTA_IPENDPOINT_INTERNAL_H__

#include "gekkota/gekkota_ipaddress.h"
#include "gekkota/gekkota_types.h"

struct _GekkotaIPEndPoint
{
    GekkotaIPAddress    *address;
    uint16_t            port;
    uint32_t            refCount;
};

#endif /* !__GEKKOTA_IPENDPOINT_INTERNAL_H__ */
