/******************************************************************************
 * @file    gekkota_time.c
 * @date    11-Nov-2006
 * @author  <a href="mailto:giuseppe.greco@agamura.com">Giuseppe Greco</a>
 *
 * Copyright (C) 2006 Agamura, Inc. - http://www.agamura.com
 * All right reserved.
 ******************************************************************************/

#include "gekkota_time.h"

int32_t
gekkota_time_compare(time_t time1, time_t time2)
{
    if (time1 - time2 >= GEKKOTA_TIME_OVERFLOW)
        /*
         * [time1] is less than [time2].
         */
        return -1;

    if (time2 - time1 >= GEKKOTA_TIME_OVERFLOW)
        /*
         * [time1] is greater than [time2].
         */
        return 1;

    /*
     * [time1] and [time2] are equal.
     */
    return 0;
}
