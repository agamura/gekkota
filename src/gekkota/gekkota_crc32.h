/******************************************************************************
 * @file    gekkota_crc32.h
 * @date    29-Dec-2006
 * @author  <a href="mailto:giuseppe.greco@agamura.com">Giuseppe Greco</a>
 *
 * Copyright (C) 2006 Agamura, Inc. - http://www.agamura.com
 * All right reserved.
 ******************************************************************************/

#ifndef __GEKKOTA_CRC32_H__
#define __GEKKOTA_CRC32_H__

#include "gekkota/gekkota_buffer.h"
#include "gekkota/gekkota_types.h"

extern uint32_t
_gekkota_crc32_calculate(const GekkotaBuffer *buffers, size_t bufferCount);

#endif /* !__GEKKOTA_CRC32_H__ */
