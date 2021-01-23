/******************************************************************************
 * @file    gekkota_networkinterface.h
 * @date    15-Sep-2007
 * @author  <a href="mailto:giuseppe.greco@agamura.com">Giuseppe Greco</a>
 *
 * Copyright (C) 2007 Agamura, Inc. - http://www.agamura.com
 * All right reserved.
 ******************************************************************************/

#ifndef __GEKKOTA_NETWORKINTERFACE_H__
#define __GEKKOTA_NETWORKINTERFACE_H__

#include "gekkota/gekkota_types.h"
#include "gekkota/gekkota_list.h"

#define GEKKOTA_NETWORK_INTERFACE_MAX_INDEX  0xFFFFFF

typedef enum
{
    GEKKOTA_NETWORK_INTERFACE_TYPE_UNKNOWN  = 0,
    GEKKOTA_NETWORK_INTERFACE_TYPE_ETHERNET
} GekkotaNetworkInterfaceType;

typedef struct _GekkotaNetworkInterface GekkotaNetworkInterface;

GEKKOTA_API GekkotaList *
gekkota_networkinterface_get_interfaces(void_t);

#if defined (GEKKOTA_BUILDING_LIB) || defined (GEKKOTA_BUILDING_STATIC_LIB)
#include "gekkota_networkinterface_internal.h"
#endif /* GEKKOTA_BUILDING_LIB || GEKKOTA_BUILDING_STATIC_LIB */

#endif /* !__GEKKOTA_NETWORKINTERFACE_H__ */
