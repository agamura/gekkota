/******************************************************************************
 * @file    gekkota_bit.h
 * @date    26-Oct-2006
 * @author  <a href="mailto:giuseppe.greco@agamura.com">Giuseppe Greco</a>
 *
 * Copyright (C) 2006 Agamura, Inc. - http://www.agamura.com
 * All right reserved.
 ******************************************************************************/

#ifndef __GEKKOTA_BIT_H__
#define __GEKKOTA_BIT_H__

#define gekkota_bit_not(i)          (~i)
#define gekkota_bit_invert(i)       (i ~= i)
#define gekkota_bit_reset(i)        (i ^= i) 
#define gekkota_bit_set(i, j)       (i |= j)
#define gekkota_bit_unset(i, j)     (i &= (~j))
#define gekkota_bit_isset(i, j)     (i & j)
#define gekkota_bit_shiftl(i, j)    (i <<= j)
#define gekkota_bit_shiftr(i, j)    (i >>= j)

#endif /* !__GEKKOTA_BIT_H__ */
