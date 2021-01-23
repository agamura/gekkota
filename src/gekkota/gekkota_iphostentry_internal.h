/******************************************************************************
 * @file    gekkota_iphostentry_internal.h
 * @date    09-Jan-2007
 * @author  <a href="mailto:giuseppe.greco@agamura.com">Giuseppe Greco</a>
 *
 * Copyright (C) 2007 Agamura, Inc. - http://www.agamura.com
 * All right reserved.
 ******************************************************************************/

#ifndef __GEKKOTA_IPHOSTENTRY_INTERNAL_H__
#define __GEKKOTA_IPHOSTENTRY_INTERNAL_H__

#include "gekkota/gekkota_iphostentry.h"
#include "gekkota/gekkota_list.h"
#include "gekkota/gekkota_types.h"

typedef struct _GekkotaIPHostAddress
{
    GekkotaListNode     listNode;
    GekkotaIPAddress    *address;
};

typedef struct _GekkotaIPHostAlias
{
    GekkotaListNode     listNode;
    char_t              *alias;
};

struct _GekkotaIPHostEntry
{
    GekkotaList         hostAddresses;
    GekkotaList         hostAliases;
    uint32_t            refCount;
    char_t              hostName[1];
};

extern GekkotaIPHostEntry *
_gekkota_iphostentry_new(const char_t *hostNameOrAddress);

#endif /* !__GEKKOTA_IPHOSTENTRY_INTERNAL_H__ */
