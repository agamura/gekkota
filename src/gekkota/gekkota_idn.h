/******************************************************************************
 * @file    gekkota_idn.h
 * @date    03-Aug-2007
 * @author  <a href="mailto:giuseppe.greco@agamura.com">Giuseppe Greco</a>
 *
 * Copyright (C) 2007 Agamura, Inc. - http://www.agamura.com
 * All right reserved.
 ******************************************************************************/

#ifndef __GEKKOTA_IDN_H__
#define __GEKKOTA_IDN_H__

#include "gekkota/gekkota_types.h"
#include "gekkota/gekkota_module.h"

extern int32_t
gekkota_idn_to_punycode(
        const char_t *utf8,
        char_t **restrict punycode,
        size_t *length);

extern int32_t
gekkota_idn_to_utf8(
        const char_t *punycode,
        char_t **restrict utf8,
        size_t *length);

extern int32_t
gekkota_idn_initialize(const GekkotaModule module);

#endif /* !__GEKKOTA_IDN_H__ */
