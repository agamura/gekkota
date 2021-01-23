/******************************************************************************
 * @file    gekkota_ipaddress_win32.c
 * @date    27-Oct-2006
 * @author  <a href="mailto:giuseppe.greco@agamura.com">Giuseppe Greco</a>
 *
 * Copyright (C) 2006 Agamura, Inc. - http://www.agamura.com
 * All right reserved.
 ******************************************************************************/

#include "gekkota.h"
#include "gekkota_buffer.h"
#include "gekkota_errors.h"

int32_t
_gekkota_ipaddress_string_to_address(
        const char_t *ipString,
        uint16_t family,
        void_t *address)
{
    struct sockaddr_in6 sockaddr;
    size_t sockaddrLength = sizeof(struct sockaddr_in6);

    if (WSAStringToAddressA(
            (LPSTR) ipString, family, NULL,
            (LPSOCKADDR) &sockaddr, (LPINT) &sockaddrLength) != 0)
    {
        errno = WSAGetLastError() == WSA_NOT_ENOUGH_MEMORY
            ? GEKKOTA_ERROR_OUT_OF_MEMORY
            : GEKKOTA_ERROR_ARGUMENT_NOT_VALID;

        return -1;
    }

    if (family == AF_INET)
        *((uint32_t *) address) =
        ((struct sockaddr_in *) &sockaddr)->sin_addr.S_un.S_addr;
    else
        memcpy(address, &sockaddr.sin6_addr, sizeof(struct in6_addr));

    return 0;
}

int32_t
_gekkota_ipaddress_address_to_string(
        const void_t *address,
        uint16_t family,
        GekkotaBuffer *restrict ipString)
{
    struct sockaddr_in6 sockaddr;

    memset(&sockaddr, 0x00, sizeof(struct sockaddr_in6));
    sockaddr.sin6_family = family;

    if (family == AF_INET)
        ((struct sockaddr_in *) &sockaddr)->sin_addr.S_un.S_addr =
        *((uint32_t *) address);
    else
        memcpy(&sockaddr.sin6_addr, address, sizeof(struct in6_addr));

    if (WSAAddressToStringA(
            (LPSOCKADDR) &sockaddr, (DWORD) sizeof(struct sockaddr_in6), NULL,
            ipString->data, (LPDWORD) &ipString->length) != 0)

    {
        errno = WSAGetLastError() == WSAENOBUFS
            ? GEKKOTA_ERROR_OUT_OF_MEMORY
            : GEKKOTA_ERROR_ARGUMENT_NOT_VALID;

        return -1;
    }

    return 0;
}
