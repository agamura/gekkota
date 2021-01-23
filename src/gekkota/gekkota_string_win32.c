/******************************************************************************
 * @file    gekkota_string_win32.c
 * @date    26-Dec-2006
 * @author  <a href="mailto:giuseppe.greco@agamura.com">Giuseppe Greco</a>
 *
 * Copyright (C) 2006 Agamura, Inc. - http://www.agamura.com
 * All right reserved.
 ******************************************************************************/

#include <errno.h>
#include "gekkota.h"
#include "gekkota_errors.h"
#include "gekkota_memory.h"

int32_t
_gekkota_string_to_unicode(
        const char_t *utf8,
        wchar_t **restrict unicode,
        size_t *length)
{
    wchar_t *wstring;
    size_t wlength;

    /*
     * Get the required length (in wide chars) of the buffer that will hold
     * the translated string.
     */
    wlength =  MultiByteToWideChar(CP_UTF8, 0, utf8, -1, NULL, 0);

    if ((wstring = gekkota_memory_alloc(sizeof(wchar_t) * wlength, FALSE)) == NULL)
        return -1;

    if (MultiByteToWideChar(
            CP_UTF8, 0, utf8, -1,
            wstring, (int32_t) wlength) == 0)
    {
        gekkota_memory_free(wstring);
        errno = GEKKOTA_ERROR_CANNOT_TRANSLATE_NAME;
        return -1;
    }

    if (length != NULL) *length = wlength * sizeof(wchar_t);
    *unicode = wstring;

    return 0;
}

int32_t
_gekkota_string_to_utf8(
        const wchar_t *unicode,
        char_t **restrict utf8,
        size_t *length)
{
    char_t *ustring;
    size_t ulength;

    /*
     * Get the required length (in bytes) of the buffer that will hold
     * the translated string.
     */
    ulength =  WideCharToMultiByte(CP_UTF8, 0, unicode, -1, NULL, 0, NULL, NULL);

    if ((ustring = gekkota_memory_alloc(ulength, FALSE)) == NULL)
        return -1;

    if (WideCharToMultiByte(
            CP_UTF8, 0, unicode, -1,
            ustring, (int32_t) ulength, NULL, NULL) == 0)
    {
        gekkota_memory_free(ustring);
        errno = GEKKOTA_ERROR_CANNOT_TRANSLATE_NAME;
        return -1;
    }

    if (length != NULL) *length = ulength;
    *utf8 = ustring;

    return 0;
}
