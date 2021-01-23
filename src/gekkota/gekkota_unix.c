/******************************************************************************
 * @file    gekkota_unix.c
 * @date    27-Oct-2006
 * @author  <a href="mailto:giuseppe.greco@agamura.com">Giuseppe Greco</a>
 *
 * Copyright (C) 2006 Agamura, Inc. - http://www.agamura.com
 * All right reserved.
 ******************************************************************************/

#include "gekkota_socket.h"

int32_t
_gekkota_initialize(void_t)
{
    return gekkota_socket_startup();
}

int32_t
_gekkota_uninitialize(void_t)
{
    return _gekkota_socket_cleanup();
}