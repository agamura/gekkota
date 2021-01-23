/******************************************************************************
 * @file    gekkota_time_win32.c
 * @date    11-Nov-2006
 * @author  <a href="mailto:giuseppe.greco@agamura.com">Giuseppe Greco</a>
 *
 * Copyright (C) 2006 Agamura, Inc. - http://www.agamura.com
 * All right reserved.
 ******************************************************************************/

#include "gekkota.h"

uint32_t
gekkota_time_now(void_t)
{
    return (uint32_t) timeGetTime();
}
