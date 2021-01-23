/******************************************************************************
 * @file    gekkota_time_unix.c
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
    struct timeval timeVal;

    gettimeofday(&timeVal, NULL);
    return timeVal.tv_sec * 1000 + timeVal.tv_usec / 1000;
}
