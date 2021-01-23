/******************************************************************************
 * @file    gekkota_socket_unix.c
 * @date    10-Jan-2007
 * @author  <a href="mailto:giuseppe.greco@agamura.com">Giuseppe Greco</a>
 *
 * Copyright (C) 2007 Agamura, Inc. - http://www.agamura.com
 * All right reserved.
 ******************************************************************************/

#include <errno.h>
#include <string.h>
#include "gekkota_bit.h"
#include "gekkota_errors.h"
#include "gekkota_socket.h"

#ifdef HAVE_FCNTL
#include <fcntl.h>
#else
#include <sys/ioctl.h>
#endif /* HAVE_FCNTL */

#ifdef __APPLE__
#undef HAVE_POLL
#endif /* HAVE_POLL */

#ifdef HAVE_POLL
#include <sys/poll.h>
#endif /* HAVE_POLL */

#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif /* !MSG_NOSIGNAL */

#ifndef HAVE_POLL
static inline int32_t
_gekkota_socket_select(
        socket_t socket,
        GekkotaSelectMode selectMode,
        int32_t timeout);
#endif /* !HAVE_POLL */

static int32_t
_gekkota_socket_transcode_error(int32_t error, int32_t defaultError);

int32_t
_gekkota_socket_startup(void_t)
{
    /*
     * Add any startup code here.
     */
    return 0;
}

int32_t
_gekkota_socket_cleanup(void_t)
{
    /*
     * Add any cleanup code here.
     */
    return 0;
}

inline socket_t
_gekkota_socket_socket(int32_t family, int32_t type, int32_t protocol)
{
    socket_t client;

    if ((client = socket(family, type, protocol)) == -1)
    {
        errno = _gekkota_socket_transcode_error(errno, GEKKOTA_ERROR_NETWORK_FAILURE);
        return GEKKOTA_INVALID_SOCKET;
    }
    else
    {
#ifdef HAVE_FCNTL
        int32_t flags;
        if (((flags = fcntl(client, F_GETFL)) == -1) ||
                (fcntl(client, F_SETFL, O_NONBLOCK, flags) == -1))
#else
        int32_t nonBlocking = 1;
        if (ioctl(client, FIONBIO, &nonBlocking) == -1)
#endif /* HAVE_FCNTL */
        {
            errno = _gekkota_socket_transcode_error(errno, GEKKOTA_ERROR_NETWORK_FAILURE);
            close(client);
            return GEKKOTA_INVALID_SOCKET;
        }
    }

    return client;
}

inline int32_t
_gekkota_socket_close(socket_t socket)
{
    if (close(socket) == -1)
    {
        errno = _gekkota_socket_transcode_error(errno, GEKKOTA_ERROR_NETWORK_FAILURE);
        return -1;
    }

    return 0;
}

inline int32_t
_gekkota_socket_getsockopt(
        socket_t socket, int32_t level,
        int32_t name, void_t *value, size_t *length)
{
    if (getsockopt(socket, level, name, value, length) == -1)
    {
        errno = _gekkota_socket_transcode_error(errno, GEKKOTA_ERROR_NETWORK_FAILURE);
        return -1;
    }

    return 0;
}

inline int32_t
_gekkota_socket_setsockopt(
        socket_t socket, int32_t level,
        int32_t name, const void_t *value, size_t length)
{
    if (setsockopt(socket, level, name, value, length) == -1)
    {
        errno = _gekkota_socket_transcode_error(errno, GEKKOTA_ERROR_NETWORK_FAILURE);
        return -1;
    }

    return 0;
}

inline int32_t
_gekkota_socket_bind(
        socket_t socket,
        const GekkotaSocketAddress *localSocketAddress)
{
    if (bind(
            socket,
            (struct sockaddr *) localSocketAddress,
            sizeof(GekkotaSocketAddress)) == -1)
    {
        errno = _gekkota_socket_transcode_error(errno, GEKKOTA_ERROR_NETWORK_FAILURE);
        return -1;
    }

    return 0;
}

inline int32_t
_gekkota_socket_listen(socket_t socket)
{
    if (listen(socket, SOMAXCONN) == -1)
    {
        errno = _gekkota_socket_transcode_error(errno, GEKKOTA_ERROR_NETWORK_FAILURE);
        return -1;
    }

    return 0;
}

inline socket_t
_gekkota_socket_accept(
        socket_t socket,
        GekkotaSocketAddress *remoteSocketAddress)
{
    socket_t client;
    socklen_t socklen = sizeof(GekkotaSocketAddress);

    if ((client = accept(
            socket,
            (struct sockaddr *) remoteSocketAddress,
            &socklen)) == -1)
    {
        errno = _gekkota_socket_transcode_error(errno, GEKKOTA_ERROR_NETWORK_FAILURE);
        return GEKKOTA_INVALID_SOCKET;
    }

    return client;
}

inline int32_t
_gekkota_socket_connect(
        socket_t socket,
        const GekkotaSocketAddress *remoteSocketAddress)
{
    if (connect(
            socket,
            (struct sockaddr *) remoteSocketAddress,
            sizeof(GekkotaSocketAddress)) == -1)
    {
        errno = _gekkota_socket_transcode_error(errno, GEKKOTA_ERROR_NETWORK_FAILURE);
        return -1;
    }

    return 0;
}

inline int32_t
_gekkota_socket_getsockname(
        socket_t socket,
        GekkotaSocketAddress *socketAddress)
{
    socklen_t socklen = sizeof (GekkotaSocketAddress);

    if (getsockname(socket, (struct sockaddr *) &socketAddress, &socklen) == -1)
    {
        errno = _gekkota_socket_transcode_error(errno, GEKKOTA_ERROR_NETWORK_FAILURE);
        return -1;
    }

    return 0;
}

inline int32_t
_gekkota_socket_getpeername(
        socket_t socket,
        GekkotaSocketAddress *socketAddress)
{
    socklen_t socklen = sizeof (GekkotaSocketAddress);

    if (getpeername(socket, (struct sockaddr *) &socketAddress, &socklen) == -1)
    {
        errno = _gekkota_socket_transcode_error(errno, GEKKOTA_ERROR_NETWORK_FAILURE);
        return -1;
    }

    return 0;
}

inline int32_t
_gekkota_socket_poll(
        socket_t socket,
        GekkotaSelectMode selectMode,
        int32_t timeout)
{
#ifdef HAVE_POLL
    struct pollfd pollSocket;
    int32_t pollCount;

    pollSocket.fd = socket;
    pollSocket.events = 0;

    if (selectMode == GEKKOTA_SELECT_MODE_READ)
        gekkota_bit_set(pollSocket.events, POLLIN);
    else if (selectMode == GEKKOTA_SELECT_MODE_WRITE)
        gekkota_bit_set(pollSocket.events, POLLOUT);

    if ((pollCount = poll(&pollSocket, 1, timeout)) == -1)
    {
        errno = _gekkota_socket_transcode_error(errno, GEKKOTA_ERROR_NETWORK_FAILURE);
        return -1;
    }

    if (pollCount > 0)
    {
        if (gekkota_bit_isset(pollSocket.revents, POLLHUP))
        {
            errno = GEKKOTA_ERROR_SOCKET_NOT_CONNECTED;
            return -1;
        }

        if (selectMode == GEKKOTA_SELECT_MODE_READ)
        {
            if (gekkota_bit_isset(pollSocket.revents, POLLIN))
                return 1;
        }
        else if (selectMode == GEKKOTA_SELECT_MODE_WRITE)
        {
            if (gekkota_bit_isset(pollSocket.revents, POLLOUT))
                return 1;
        }
        else if (selectMode == GEKKOTA_SELECT_MODE_ERROR)
        {
            if (gekkota_bit_isset(pollSocket.revents, POLLERR) ||
                    gekkota_bit_isset(pollSocket.revents, POLLNVAL))
                return 1;
        }
    }

    return 0;
#else
    return _gekkota_socket_select(socket, selectMode, timeout);
#endif /* HAVE_POLL*/
}

#ifndef HAVE_POLL
static inline int32_t
_gekkota_socket_select(
        socket_t socket,
        GekkotaSelectMode selectMode,
        int32_t timeout)
{
    fd_set readSet, writeSet, errorSet;
    struct timeval timeval, *selectTimeout = NULL;
    int32_t selectCount;

    if (timeout > -1)
    {
        if (timeout > 0)
        {
            timeval.tv_sec = timeout / 1000;
            timeval.tv_usec = (timeout % 1000) * 1000;
        }

        selectTimeout = &timeval;
    }

    FD_ZERO(&readSet);
    FD_ZERO(&writeSet);
    FD_ZERO(&errorSet);

    if (selectMode == GEKKOTA_SELECT_MODE_READ)
        FD_SET(socket, &readSet);
    else if (selectMode == GEKKOTA_SELECT_MODE_WRITE)
        FD_SET(socket, &writeSet);
    else if (selectMode == GEKKOTA_SELECT_MODE_ERROR)
        FD_SET(socket, &errorSet);

    if ((selectCount = select(
            socket + 1,
            &readSet, &writeSet, &errorSet,
            selectTimeout)) == -1)
    {
        errno = _gekkota_socket_transcode_error(errno, GEKKOTA_ERROR_NETWORK_FAILURE);
        return -1;
    }

    if (selectCount > 0)
    {
        if (selectMode == GEKKOTA_SELECT_MODE_READ)
        {
            if (FD_ISSET(socket, &readSet))
                return 1;
        }
        else if (selectMode == GEKKOTA_SELECT_MODE_WRITE)
        {
            if (FD_ISSET(socket, &writeSet))
                return 1;
        }
        else if (selectMode == GEKKOTA_SELECT_MODE_ERROR)
        {
            if (FD_ISSET(socket, &errorSet))
                return 1;
        }
    }

    return 0;
}
#endif /* !HAVE_POLL */

inline int32_t
_gekkota_socket_send(
        socket_t socket,
        GekkotaSocketAddress *remoteSocketAddress,
        GekkotaBuffer *buffers,
        size_t bufferCount)
{
    int32_t sent = 0;
    struct msghdr msg;
    memset(&msg, 0x00, sizeof(struct msghdr));

    if (remoteSocketAddress != NULL)
    {
        msg.msg_name = remoteSocketAddress;
        msg.msg_namelen = sizeof(GekkotaSocketAddress);
    }

    msg.msg_iov = (struct iovec *) buffers;
    msg.msg_iovlen = bufferCount;

    if ((sent = sendmsg(socket, &msg, MSG_NOSIGNAL)) == -1)
    {
        if (errno == EWOULDBLOCK)
            return 0;

        errno = _gekkota_socket_transcode_error(errno, GEKKOTA_ERROR_NETWORK_FAILURE);
        return -1;
    }

    return sent;
}

inline int32_t
_gekkota_socket_receive(
        socket_t socket,
        GekkotaBuffer *buffers,
        size_t bufferCount,
        GekkotaSocketAddress *remoteSocketAddress)
{
    int32_t recv = 0;
    struct msghdr msg;
    memset(&msg, 0x00, sizeof(struct msghdr));

    if (remoteSocketAddress != NULL)
    {
        msg.msg_name = remoteSocketAddress;
        msg.msg_namelen = sizeof(GekkotaSocketAddress);
    }

    msg.msg_iov = (struct iovec *) buffers;
    msg.msg_iovlen = bufferCount;

    if ((recv = recvmsg(socket, &msg, MSG_NOSIGNAL)) == -1)
    {
        if (errno == EWOULDBLOCK)
            return 0;

        errno = _gekkota_socket_transcode_error(errno, GEKKOTA_ERROR_NETWORK_FAILURE);
        return -1;
    }

#ifdef HAVE_MSGHDR_FLAGS
    if (gekkota_bit_isset(msg.msg_flags, MSG_TRUNC))
    {
        errno = GEKKOTA_ERROR_MESSAGE_TRUNCATED;
        return -1;
    }
#endif /* HAVE_MSGHDR_FLAGS */

    return recv;
}

static int32_t
_gekkota_socket_transcode_error(int32_t error, int32_t defaultError)
{
    switch (error)
    {
        case EACCES:
        case EPERM:         return GEKKOTA_ERROR_PERMISSION_DENIED;
        case EADDRINUSE:    return GEKKOTA_ERROR_ADDRESS_ALREADY_IN_USE;
        case EADDRNOTAVAIL: return GEKKOTA_ERROR_ADDRESS_NOT_AVAILABLE;
        case EAFNOSUPPORT:  return GEKKOTA_ERROR_ADDRESS_FAMILY_NOT_SUPPORTED;
#if EAGAIN != EWOULDBLOCK
        case EAGAIN:
#endif /* EAGAIN != EWOULDBLOCK */
        case EWOULDBLOCK:   return GEKKOTA_ERROR_NO_RESOURCE_AVAILABLE;
        case EALREADY:      return GEKKOTA_ERROR_OPERATION_ALREADY_IN_PROGRESS;
        case ECONNABORTED:  return GEKKOTA_ERROR_CONNECTION_ABORTED;
        case ENOENT:
        case ECONNREFUSED:  return GEKKOTA_ERROR_CONNECTION_REFUSED;
        case ECONNRESET:    return GEKKOTA_ERROR_CONNECTION_RESET;
        case EDESTADDRREQ:  return GEKKOTA_ERROR_DESTINATION_ADDRESS_REQUIRED;
        case EHOSTUNREACH:  return GEKKOTA_ERROR_HOST_NOT_REACHABLE;
        case EINTR:         return GEKKOTA_ERROR_OPERATION_INTERRUPTED;
        case EISCONN:       return GEKKOTA_ERROR_SOCKET_ALREADY_CONNECTED;
        case ELOOP:         return GEKKOTA_ERROR_CANNOT_TRANSLATE_NAME;
        case EMFILE:
        case ENFILE:        return GEKKOTA_ERROR_TOO_MANY_OPEN_SOCKETS;
        case EMSGSIZE:      return GEKKOTA_ERROR_MESSAGE_TOO_LONG;
        case ENAMETOOLONG:  return GEKKOTA_ERROR_NAME_TOO_LONG;
        case ETIMEDOUT:     return GEKKOTA_ERROR_TIMEOUT;
#ifdef ENOSR
        case ENOSR:
#endif /* ENOSR */
#if ERESTARTSYS
        case ERESTARTSYS:
#endif /* ERESTARTSYS */
        case ENETDOWN:      return GEKKOTA_ERROR_NETWORK_IS_DOWN;
        case ENETRESET:     return GEKKOTA_ERROR_CONNECTION_ABORTED_BY_NETWORK;
        case ENETUNREACH:   return GEKKOTA_ERROR_NETWORK_NOT_REACHABLE;
        case ENOMEM:
        case ENOBUFS:       return GEKKOTA_ERROR_NO_BUFFER_SPACE_AVAILABLE;
        case ENOTCONN:      return GEKKOTA_ERROR_SOCKET_NOT_CONNECTED;
        case EOPNOTSUPP:    return GEKKOTA_ERROR_OPERATION_NOT_SUPPORTED;
        case EFAULT:
        case EOVERFLOW:     return GEKKOTA_ERROR_BUFFER_OVERFLOW;
        case EPIPE:         return GEKKOTA_ERROR_SOCKET_ALREADY_SHUTDOWN;
        default: break;
	}

    return defaultError;
}
