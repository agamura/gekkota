/******************************************************************************
 * @file    gekkota_internal.h
 * @date    27-Oct-2006
 * @author  <a href="mailto:giuseppe.greco@agamura.com">Giuseppe Greco</a>
 *
 * Copyright (C) 2006 Agamura, Inc. - http://www.agamura.com
 * All right reserved.
 ******************************************************************************/

#ifndef __GEKKOTA_INTERNAL_H__
#define __GEKKOTA_INTERNAL_H__

#include "gekkota/gekkota_types.h"
#ifdef WIN32
#include <Winsock2.h>
#include <WS2tcpip.h>
#include <Windows.h>
#else
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/utsname.h>
#endif /* WIN32 */

extern int32_t
_gekkota_initialize(void_t);

extern int32_t
_gekkota_uninitialize(void_t);

#endif /* !__GEKKOTA_INTERNAL_H__ */
