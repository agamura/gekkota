/******************************************************************************
 * @file    gekkota_memory.h
 * @date    18-Mar-2009
 * @author  <a href="mailto:giuseppe.greco@agamura.com">Giuseppe Greco</a>
 *
 * Copyright (C) 2007 Agamura, Inc. - http://www.agamura.com
 * All right reserved.
 ******************************************************************************/

#ifndef __GEKKOTA_MEMORY_H__
#define __GEKKOTA_MEMORY_H__

#include "gekkota/gekkota_types.h"

#define GEKKOTA_MEMORY_DEFAULT_BLOCK_SIZE 256
#define GEKKOTA_MEMORY_DEFAULT_BLOCK_COUNT 100

GEKKOTA_API int32_t
gekkota_memory_initialize(size_t blockSize, uint32_t initialBlockCount);

GEKKOTA_API int32_t
gekkota_memory_uninitialize(void_t);

GEKKOTA_API bool_t
gekkota_memory_is_initialized();

GEKKOTA_API size_t
gekkota_memory_get_block_size(void_t);

GEKKOTA_API uint32_t
gekkota_memory_get_block_count(void_t);

GEKKOTA_API void_t *
gekkota_memory_alloc(size_t size, bool_t initialize);

GEKKOTA_API void_t *
gekkota_memory_realloc(void_t *memory, size_t newSize, bool_t initialize);

GEKKOTA_API void_t
gekkota_memory_free(void_t *memory);

#endif /* !__GEKKOTA_MEMORY_H__ */
