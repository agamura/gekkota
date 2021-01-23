/******************************************************************************
 * @file    gekkota_time.h
 * @date    11-Nov-2006
 * @author  <a href="mailto:giuseppe.greco@agamura.com">Giuseppe Greco</a>
 *
 * Copyright (C) 2006 Agamura, Inc. - http://www.agamura.com
 * All right reserved.
 ******************************************************************************/

#ifndef __GEKKOTA_TIME_H__
#define __GEKKOTA_TIME_H__

#include "gekkota/gekkota_types.h"

#define GEKKOTA_TIME_OVERFLOW 86400000

inline time_t gekkota_time_get_lag(time_t time1, time_t time2)
{
    return (time1 - time2) >= GEKKOTA_TIME_OVERFLOW
        ? (time2 - time1)
        : (time1 - time2);
}

GEKKOTA_API int32_t
gekkota_time_compare(time_t time1, time_t time2);

GEKKOTA_API time_t
gekkota_time_now();

#endif /* !__GEKKOTA_TIME_H__ */
