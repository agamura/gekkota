/******************************************************************************
 * @file    gekkota_win32.c
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
    if (_gekkota_socket_startup() == -1)
        return -1;

    timeBeginPeriod(1);
    return 0;
}

int32_t
_gekkota_uninitialize(void_t)
{
    timeEndPeriod(1);
    return _gekkota_socket_cleanup();
}