/******************************************************************************
 * @file    gekkota_idn_unix.c
 * @date    03-Aug-2007
 * @author  <a href="mailto:giuseppe.greco@agamura.com">Giuseppe Greco</a>
 *
 * Copyright (C) 2007 Agamura, Inc. - http://www.agamura.com
 * All right reserved.
 ******************************************************************************/

#include <errno.h>
#include <idna.h>
#include <string.h>
#include "gekkota_errors.h"
#include "gekkota_idn.h"

typedef int (*IdnToAscii)(const char *, char **, int);
typedef int (*IdnToUnicode)(const char *, char **, int);

static IdnToAscii idnToAscii;
static IdnToUnicode idnToUnicode;

int32_t
gekkota_idn_to_punycode(
        const char_t *utf8,
        char_t **restrict punycode,
        size_t *length)
{
    switch ((idnToAscii)(utf8, punycode, 0))
    {
        case IDNA_SUCCESS:
            if (length != NULL) *length = strlen(*punycode) + sizeof(char_t);
            return 0;
        case IDNA_MALLOC_ERROR:
            errno = GEKKOTA_ERROR_OUT_OF_MEMORY;
            break;
        default:
            errno = GEKKOTA_ERROR_CANNOT_TRANSLATE_NAME;
            break;
    }

    return -1;
}

int32_t
gekkota_idn_to_utf8(
        const char_t *punycode,
        char_t **restrict utf8,
        size_t *length)
{
    switch ((idnToUnicode)(punycode, utf8, 0))
    {   
        case IDNA_SUCCESS:
            if (length != NULL) *length = strlen(*utf8) + sizeof(char_t);
            return 0;
        case IDNA_MALLOC_ERROR:
            errno = GEKKOTA_ERROR_OUT_OF_MEMORY;
            break;
        default:
            errno = GEKKOTA_ERROR_CANNOT_TRANSLATE_NAME;
            break;
    }

    return -1;
}

int32_t
gekkota_idn_initialize(const GekkotaModule module)
{
    if ((idnToAscii = (IdnToAscii) gekkota_module_get_function_ptr(
            module, "idna_to_ascii_8z")) == NULL ||
        (idnToUnicode = (IdnToUnicode) gekkota_module_get_function_ptr(
            module, "idna_to_unicode_8z8z")) == NULL)
        return -1;

    return 0;
}
