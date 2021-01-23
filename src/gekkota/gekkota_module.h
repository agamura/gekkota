/******************************************************************************
 * @file    gekkota_module.h
 * @date    08-Sep-2007
 * @author  <a href="mailto:giuseppe.greco@agamura.com">Giuseppe Greco</a>
 *
 * Copyright (C) 2007 Agamura, Inc. - http://www.agamura.com
 * All right reserved.
 ******************************************************************************/

#ifndef __GEKKOTA_MODULE_H__
#define __GEKKOTA_MODULE_H__

#include "gekkota/gekkota_types.h"

struct _GekkotaModule { int32_t handle; };
typedef struct _GekkotaModule *GekkotaModule;
typedef int32_t (GEKKOTA_CALLTYPE *GekkotaFunctionPtr)(void_t);

GEKKOTA_API GekkotaModule
gekkota_module_load(char_t *moduleName);

GEKKOTA_API int32_t
gekkota_module_unload(GekkotaModule module);

GEKKOTA_API GekkotaFunctionPtr
gekkota_module_get_function_ptr(GekkotaModule module, char_t *functionName);

#endif /* !__GEKKOTA_MODULE_H__ */
