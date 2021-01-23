/******************************************************************************
 * @file    gekkota_ipaddress_unix.c
 * @date    27-Oct-2006
 * @author  <a href="mailto:giuseppe.greco@agamura.com">Giuseppe Greco</a>
 *
 * Copyright (C) 2006 Agamura, Inc. - http://www.agamura.com
 * All right reserved.
 ******************************************************************************/

#include <errno.h>
#include <arpa/inet.h>
#include "gekkota_buffer.h"
#include "gekkota_errors.h"

int32_t
_gekkota_ipaddress_string_to_address(
        const char_t *ipString,
        uint16_t family,
        void_t *address)
{
    if (inet_pton(family, ipString, address) != 1)
    {
        errno = GEKKOTA_ERROR_ARGUMENT_NOT_VALID;
        return -1;
    }
    
    return 0;
}

int32_t
_gekkota_ipaddress_address_to_string(
        const void_t *address,
        uint16_t family,
        GekkotaBuffer *restrict ipString)
{
    if (inet_ntop(family, address, ipString->data, ipString->length) == NULL)
    {
        errno = GEKKOTA_ERROR_ARGUMENT_NOT_VALID;
        return -1;
    }

    return 0;
}
