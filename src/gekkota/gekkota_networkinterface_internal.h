/******************************************************************************
 * @file    gekkota_networkinterface_internal.h
 * @date    15-Sep-2007
 * @author  <a href="mailto:giuseppe.greco@agamura.com">Giuseppe Greco</a>
 *
 * Copyright (C) 2007 Agamura, Inc. - http://www.agamura.com
 * All right reserved.
 ******************************************************************************/

#ifndef __GEKKOTA_NETWORKINTERFACE_INTERNAL_H__
#define __GEKKOTA_NETWORKINTERFACE_INTERNAL_H__

#include "gekkota/gekkota_networkinterface.h"
#include "gekkota/gekkota_list.h"
#include "gekkota/gekkota_types.h"

struct _GekkotaNetworkInterface
{
    GekkotaListNode listNode;
    uint32_t        refCount;
};

#endif /* !__GEKKOTA_NETWORKINTERFACE_INTERNAL_H__ */
