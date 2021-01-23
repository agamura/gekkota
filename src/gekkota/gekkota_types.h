/******************************************************************************
 * @file    gekkota_types.h
 * @date    20-Oct-2006
 * @author  <a href="mailto:giuseppe.greco@agamura.com">Giuseppe Greco</a>
 *
 * Copyright (C) 2006 Agamura, Inc. - http://www.agamura.com
 * All right reserved.
 ******************************************************************************/

#ifndef __GEKKOTA_TYPES_H__
#define __GEKKOTA_TYPES_H__

#ifdef WIN32
#include <crtdefs.h>
typedef signed char         int8_t;
typedef unsigned char		uint8_t;
typedef signed short		int16_t;
typedef unsigned short		uint16_t;
typedef signed long         int32_t;
typedef unsigned long		uint32_t;
typedef signed long long	int64_t;
typedef unsigned long long	uint64_t;
#else
#include <unistd.h>
#include <stdint.h>
#include <wchar.h>
#endif /* WIN32 */

typedef void                void_t;
typedef unsigned char       byte_t;
typedef int                 bool_t;
typedef char                char_t;
typedef double              double_t;
typedef float               float_t;

#ifdef WIN32
#define GEKKOTA_CALLBACK __cdecl
#define GEKKOTA_CALLTYPE __stdcall
#else
#define GEKKOTA_CALLBACK
#define GEKKOTA_CALLTYPE
#endif /* WIN32 */

#ifndef TRUE
#define TRUE 1
#endif /* !TRUE */

#ifndef FALSE
#define FALSE 0
#endif /* !FALSE */

#ifndef NULL
#define NULL 0
#endif /* !NULL */

#ifdef WIN32
#define inline __inline
#define restrict
#endif /* WIN32 */

#ifndef GEKKOTA_API
#ifdef WIN32
#ifdef GEKKOTA_BUILDING_STATIC_LIB
#define GEKKOTA_API extern
#else
#ifdef GEKKOTA_DLL
#ifdef GEKKOTA_BUILDING_LIB
#define GEKKOTA_API __declspec(dllexport)
#else
#define GEKKOTA_API extern __declspec(dllimport)
#endif /* GEKKOTA_BUILDING_LIB */
#else
#define GEKKOTA_API extern
#endif /* GEKKOTA_DLL */
#endif /* GEKKOTA_BUILDING_STATIC_LIB */
#else
#define GEKKOTA_API extern
#endif /* WIN32 */
#endif /* !GEKKOTA_API */

#endif /* !__GEKKOTA_TYPES_H__ */
