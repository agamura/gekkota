/******************************************************************************
 * @file    gekkota_string_unix.c
 * @date    26-Dec-2006
 * @author  <a href="mailto:giuseppe.greco@agamura.com">Giuseppe Greco</a>
 *
 * Copyright (C) 2006 Agamura, Inc. - http://www.agamura.com
 * All right reserved.
 ******************************************************************************/

#include <errno.h>
#include <stdlib.h>
#include "gekkota_errors.h"
#include "gekkota_hooks.h"

int32_t
_gekkota_string_to_unicode(
        const char_t *utf8,
        wchar_t **restrict unicode,
        size_t *length)
{
    return 0;
}

int32_t
_gekkota_string_to_utf8(
        const wchar_t *unicode,
        char_t **restrict utf8,
        size_t *length)
{
    return 0;
}
