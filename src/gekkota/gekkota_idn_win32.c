/******************************************************************************
 * @file    gekkota_idn_win32.c
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
#include "gekkota_idn.h"
#include "gekkota_utils.h"

typedef int (__stdcall *IdnToAsciiPtr)(DWORD, LPCWSTR, int, LPWSTR, int);
typedef int (__stdcall *IdnToUnicodePtr)(DWORD, LPCWSTR, int, LPWSTR, int);

static IdnToAsciiPtr idnToAsciiPtr;
static IdnToUnicodePtr idnToUnicodePtr;

int32_t
gekkota_idn_to_punycode(
        const char_t *utf8,
        char_t **restrict punycode,
        size_t *length)
{
    int32_t rc = -1;
    wchar_t *wstring, *pstring = NULL;
    size_t plength;

    /*
     * Convert the input character string to unicode, the format required by the
     * Microsoft IDN Mitigation API.
     */
    if (gekkota_utils_to_unicode(utf8, &wstring, NULL) != 0)
        return -1;

    /*
     * Get the required length (in wide chars) of the buffer that will hold
     * the punycode string; assume [wstring] is null-terminated.
     */
    plength = (idnToAsciiPtr)(0, wstring, -1, NULL, 0);

    /*
     * [plength] includes the null terminator.
     */
    if ((pstring = gekkota_memory_alloc(sizeof(wchar_t) * plength, FALSE)) == NULL)
        goto gekkota_idn_to_punycode_exit;

    if ((idnToAsciiPtr)(0, wstring, -1, pstring, (int32_t) plength) == 0)
    {
        errno = GEKKOTA_ERROR_CANNOT_TRANSLATE_NAME;
        goto gekkota_idn_to_punycode_exit;
    }

    /*
     * Convert back the punycode string to ASCII.
     */
    if ((rc = gekkota_utils_to_utf8(pstring, punycode, length) != 0))
        goto gekkota_idn_to_punycode_exit;

gekkota_idn_to_punycode_exit:
    gekkota_memory_free(wstring);
    gekkota_memory_free(pstring);

    return rc;
}

int32_t
gekkota_idn_to_utf8(
        const char_t *punycode,
        char_t **restrict utf8,
        size_t *length)
{
    int32_t rc = -1;
    wchar_t *pstring, *wstring = NULL;
    size_t wlength;

    /*
     * Convert the input punycode string to unicode, the format required by the
     * Microsoft IDN Mitigation API.
     */
    if (gekkota_utils_to_unicode(punycode, &pstring, NULL) != 0)
        return -1;

    /*
     * Get the required length (in wide chars) of the buffer that will hold
     * the unicode string; assume [pstring] is null-terminated.
     */
    wlength = (idnToUnicodePtr)(0, pstring, -1, NULL, 0);

    /*
     * [wlength] includes the null terminator.
     */
    if ((wstring = gekkota_memory_alloc(sizeof(wchar_t) * wlength, FALSE)) == NULL)
        goto gekkota_idn_to_utf8_exit;

    if ((idnToUnicodePtr)(0, pstring, -1, wstring, (int32_t) wlength) == 0)
    {
        errno = GEKKOTA_ERROR_CANNOT_TRANSLATE_NAME;
        goto gekkota_idn_to_utf8_exit;
    }

    /*
     * Convert back the unicode string to UTF8.
     */
    if ((rc = gekkota_utils_to_utf8(wstring, utf8, length) != 0))
        goto gekkota_idn_to_utf8_exit;

gekkota_idn_to_utf8_exit:
    gekkota_memory_free(pstring);
    gekkota_memory_free(wstring);

    return rc;
}

int32_t
gekkota_idn_initialize(const GekkotaModule module)
{
    if ((idnToAsciiPtr = (IdnToAsciiPtr) gekkota_module_get_function_ptr(
            module, "IdnToAscii")) == NULL ||
        (idnToUnicodePtr = (IdnToUnicodePtr) gekkota_module_get_function_ptr(
            module, "IdnToUnicode")) == NULL)
        return -1;

    return 0;
}
