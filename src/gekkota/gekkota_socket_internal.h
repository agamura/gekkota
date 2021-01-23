/******************************************************************************
 * @file    gekkota_socket_internal.h
 * @date    27-Oct-2006
 * @author  <a href="mailto:giuseppe.greco@agamura.com">Giuseppe Greco</a>
 *
 * Copyright (C) 2006 Agamura, Inc. - http://www.agamura.com
 * All right reserved.
 ******************************************************************************/

#ifndef __GEKKOTA_SOCKET_INTERNAL_H__
#define __GEKKOTA_SOCKET_INTERNAL_H__

#include "gekkota/gekkota_ipendpoint.h"
#include "gekkota/gekkota_buffer.h"
#include "gekkota/gekkota_socket.h"
#include "gekkota/gekkota_types.h"

#ifdef WIN32
#include <Winsock2.h>
typedef SOCKET  socket_t;
#define GEKKOTA_INVALID_SOCKET  INVALID_SOCKET
#else
#include <sys/socket.h>
#ifndef HAVE_SOCKET_T
typedef int32_t socket_t;
#endif /* !HAVE_SOCKET_T */
#define GEKKOTA_INVALID_SOCKET  -1
#endif /* WIN32 */

#ifndef HAVE_SOCKLEN_T
typedef int32_t socklen_t;
#endif /* !HAVE_SOCKLEN_T */

struct _GekkotaSocket
{
    socket_t            client;
    int32_t             type;
    GekkotaIPEndPoint   *localEndPoint;
    GekkotaIPEndPoint   *remoteEndPoint;
    uint32_t            refCount;
};

extern int32_t
_gekkota_socket_startup(void_t);

extern int32_t
_gekkota_socket_cleanup(void_t);

extern inline socket_t
_gekkota_socket_socket(int32_t family, int32_t type, int32_t protocol);

extern inline int32_t
_gekkota_socket_close(socket_t socket);

extern inline int32_t
_gekkota_socket_getsockopt(
        socket_t socket, int32_t level,
        int32_t name, void_t *value, size_t *length);

extern inline int32_t
_gekkota_socket_setsockopt(
        socket_t socket, int32_t level,
        int32_t name, const void_t *value, size_t length);

extern inline int32_t
_gekkota_socket_bind(
        socket_t socket,
        const GekkotaSocketAddress *localSocketAddress);

extern inline int32_t
_gekkota_socket_listen(socket_t socket);

extern inline socket_t
_gekkota_socket_accept(
        socket_t socket,
        GekkotaSocketAddress *remoteSocketAddress);

extern inline int32_t
_gekkota_socket_connect(
        socket_t socket,
        const GekkotaSocketAddress *remoteSocketAddress);

extern inline int32_t
_gekkota_socket_getsockname(
        socket_t socket,
        GekkotaSocketAddress *socketAddress);

extern inline int32_t
_gekkota_socket_getpeername(
        socket_t socket,
        GekkotaSocketAddress *socketAddress);

extern inline int32_t
_gekkota_socket_poll(
        socket_t socket,
        GekkotaSelectMode selectMode,
        int32_t timeout);

extern inline int32_t
_gekkota_socket_send(
        socket_t socket,
        GekkotaSocketAddress *remoteSocketAddress,
        const GekkotaBuffer *buffers,
        size_t bufferCount);

extern inline int32_t
_gekkota_socket_receive(
        socket_t socket,
        GekkotaBuffer *buffers,
        size_t bufferCount,
        GekkotaSocketAddress *remoteSocketAddress);

#endif /* !__GEKKOTA_SOCKET_INTERNAL_H__ */
