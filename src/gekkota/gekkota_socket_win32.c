/******************************************************************************
 * @file    gekkota_socket_win32.c
 * @date    22-Jan-2007
 * @author  <a href="mailto:giuseppe.greco@agamura.com">Giuseppe Greco</a>
 *
 * Copyright (C) 2007 Agamura, Inc. - http://www.agamura.com
 * All right reserved.
 ******************************************************************************/

#include <errno.h>
#include "gekkota.h"
#include "gekkota_bit.h"
#include "gekkota_errors.h"
#include "gekkota_platform.h"
#include "gekkota_socket.h"

static inline int32_t
_gekkota_socket_select(
        socket_t socket,
        GekkotaSelectMode selectMode,
        int32_t timeout);

static int32_t
_gekkota_socket_transcode_error(int32_t error, int32_t defaultError);

int32_t
_gekkota_socket_startup(void_t)
{
    WSADATA wsaData;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData))
    {
        errno = _gekkota_socket_transcode_error(
                WSAGetLastError(), GEKKOTA_ERROR_NETWORK_FAILURE);

        return -1;
    }

    if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
    {
        WSACleanup();
        errno = GEKKOTA_ERROR_SOCKET_VERSION_NOT_SUPPORTED;
        return -1;
    }

    return 0;
}

int32_t
_gekkota_socket_cleanup(void_t)
{
    if (WSACleanup())
    {
        errno = _gekkota_socket_transcode_error(
                WSAGetLastError(), GEKKOTA_ERROR_NETWORK_FAILURE);

        return -1;
    }

    return 0;
}

inline socket_t
_gekkota_socket_socket(int32_t family, int32_t type, int32_t protocol)
{
    socket_t client;

    if ((client = WSASocket(
            family, type, protocol,
            NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
    {
        errno = _gekkota_socket_transcode_error(
                WSAGetLastError(), GEKKOTA_ERROR_NETWORK_FAILURE);

        return GEKKOTA_INVALID_SOCKET;
    }
    else
    {
        int32_t enable = 1;
        DWORD byteReturned;

        if (WSAIoctl(
                client, FIONBIO,
                &enable, sizeof(int32_t),   /* input data */
                NULL, 0,                    /* output data */
                &byteReturned, NULL, NULL) == SOCKET_ERROR)
        {
            errno = _gekkota_socket_transcode_error(
                    WSAGetLastError(), GEKKOTA_ERROR_NETWORK_FAILURE);

            closesocket(client);
            return GEKKOTA_INVALID_SOCKET;
        }
    }

    return client;
}

inline int32_t
_gekkota_socket_close(socket_t socket)
{
    if (closesocket(socket) == SOCKET_ERROR)
    {
        errno = _gekkota_socket_transcode_error(
                WSAGetLastError(), GEKKOTA_ERROR_NETWORK_FAILURE);

        return -1;
    }

    return 0;
}

inline int32_t
_gekkota_socket_getsockopt(
        socket_t socket, int32_t level,
        int32_t name, void_t *value, size_t *length)
{
    if (getsockopt(socket, level, name, value, (int32_t *) length) == SOCKET_ERROR)
    {
        errno = _gekkota_socket_transcode_error(
                WSAGetLastError(), GEKKOTA_ERROR_NETWORK_FAILURE);

        return -1;
    }

    return 0;
}

inline int32_t
_gekkota_socket_setsockopt(
        socket_t socket, int32_t level,
        int32_t name, const void_t *value, size_t length)
{
    if (setsockopt(socket, level, name, value, (int32_t) length) == SOCKET_ERROR)
    {
        errno = _gekkota_socket_transcode_error(
                WSAGetLastError(), GEKKOTA_ERROR_NETWORK_FAILURE);

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
            sizeof(GekkotaSocketAddress)) == SOCKET_ERROR)
    {
        errno = _gekkota_socket_transcode_error(
                WSAGetLastError(), GEKKOTA_ERROR_NETWORK_FAILURE);

        return -1;
    }

    return 0;
}

inline int32_t
_gekkota_socket_listen(socket_t socket)
{
    if (listen(socket, SOMAXCONN) == SOCKET_ERROR)
    {
        errno = _gekkota_socket_transcode_error(
                WSAGetLastError(), GEKKOTA_ERROR_NETWORK_FAILURE);

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

    if ((client = WSAAccept(
            socket,
            (struct sockaddr *) &remoteSocketAddress,
            &socklen, NULL, 0)) == INVALID_SOCKET)
    {
        errno = _gekkota_socket_transcode_error(
                WSAGetLastError(), GEKKOTA_ERROR_NETWORK_FAILURE);

        return GEKKOTA_INVALID_SOCKET;
    }

    return client;
}

inline int32_t
_gekkota_socket_connect(
        socket_t socket,
        const GekkotaSocketAddress *remoteSocketAddress)
{
    if (WSAConnect(
            socket,
            (struct sockaddr *) &remoteSocketAddress,
            sizeof(GekkotaSocketAddress),
            NULL, NULL, NULL, NULL) == SOCKET_ERROR)
    {
        errno = _gekkota_socket_transcode_error(
                WSAGetLastError(), GEKKOTA_ERROR_NETWORK_FAILURE);

        return -1;
    }

    return 0;
}

inline int32_t
_gekkota_socket_getsockname(
        socket_t socket,
        GekkotaSocketAddress *socketAddress)
{
    int socklen = sizeof (GekkotaSocketAddress);

    if (getsockname(
            socket,
            (struct sockaddr *) &socketAddress, &socklen) == SOCKET_ERROR)
    {
        errno = _gekkota_socket_transcode_error(
                WSAGetLastError(), GEKKOTA_ERROR_NETWORK_FAILURE);

        return -1;
    }

    return 0;
}

inline int32_t
_gekkota_socket_getpeername(
        socket_t socket,
        GekkotaSocketAddress *socketAddress)
{
    int socklen = sizeof (GekkotaSocketAddress);

    if (getpeername(
            socket,
            (struct sockaddr *) &socketAddress, &socklen) == SOCKET_ERROR)
    {
        errno = _gekkota_socket_transcode_error(
                WSAGetLastError(), GEKKOTA_ERROR_NETWORK_FAILURE);

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
#if WINVER >= 0x0600
    if (platform.majorVersion >= 6)
    {
        WSAPOLLFD pollSocket;
        int32_t pollCount;

        pollSocket.fd = socket;
        pollSocket.events = 0;

        if (selectMode == GEKKOTA_SELECT_MODE_READ)
            gekkota_bit_set(pollSocket.events, POLLIN);
        else if (selectMode == GEKKOTA_SELECT_MODE_WRITE)
            gekkota_bit_set(pollSocket.events, POLLOUT);

        if ((pollCount = WSAPoll(&pollSocket, 1, timeout)) == SOCKET_ERROR)
        {
            errno = _gekkota_socket_transcode_error(
                    WSAGetLastError(), GEKKOTA_ERROR_NETWORK_FAILURE);

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
    }
    else
#endif /* WINVER >= 0x0600 */
    return _gekkota_socket_select(socket, selectMode, timeout);
}

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
            (int32_t) socket + 1,
            &readSet, &writeSet, &errorSet,
            selectTimeout)) == SOCKET_ERROR)
    {
        errno = _gekkota_socket_transcode_error(
                WSAGetLastError(), GEKKOTA_ERROR_NETWORK_FAILURE);

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

inline int32_t
_gekkota_socket_send(
        socket_t socket,
        GekkotaSocketAddress *remoteSocketAddress,
        const GekkotaBuffer *buffers,
        size_t bufferCount)
{
    int32_t sent = 0;

    if (WSASendTo(
            socket,
            (LPWSABUF) buffers, (DWORD) bufferCount, (LPDWORD) &sent, 0,
            remoteSocketAddress != NULL ? (struct sockaddr *) remoteSocketAddress : NULL,
            remoteSocketAddress != NULL ? sizeof(GekkotaSocketAddress) : 0,
            NULL, NULL) == SOCKET_ERROR)
    {
        if ((errno = WSAGetLastError()) == WSAEWOULDBLOCK)
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
    DWORD recv = 0;
    DWORD flags = 0;
    INT socklen = sizeof(GekkotaSocketAddress);

    if (WSARecvFrom(
            socket,
            (LPWSABUF) buffers, (DWORD) bufferCount, (LPDWORD) &recv, &flags,
            remoteSocketAddress != NULL ? (struct sockaddr *) remoteSocketAddress : NULL,
            remoteSocketAddress != NULL ? &socklen : NULL,
            NULL, NULL) == SOCKET_ERROR)
    {
        errno = WSAGetLastError();

        if (errno == WSAEWOULDBLOCK || errno == WSAECONNRESET)
            return 0;

        errno = _gekkota_socket_transcode_error(errno, GEKKOTA_ERROR_NETWORK_FAILURE);
        return -1;
    }

    if (gekkota_bit_isset(flags, MSG_PARTIAL))
    {
        errno = GEKKOTA_ERROR_MESSAGE_TRUNCATED;
        return -1;
    }

    return (int32_t) recv;
}

static int32_t
_gekkota_socket_transcode_error(int32_t error, int32_t defaultError)
{
    switch (error)
    {
        case WSAHOST_NOT_FOUND:     return GEKKOTA_ERROR_HOST_NOT_FOUND;
        case WSA_NOT_ENOUGH_MEMORY: return GEKKOTA_ERROR_OUT_OF_MEMORY;
        case WSAEACCES:             return GEKKOTA_ERROR_PERMISSION_DENIED;
        case WSAEADDRINUSE:         return GEKKOTA_ERROR_ADDRESS_ALREADY_IN_USE;
        case WSAEADDRNOTAVAIL:      return GEKKOTA_ERROR_ADDRESS_NOT_AVAILABLE;
        case WSAEAFNOSUPPORT:       return GEKKOTA_ERROR_ADDRESS_FAMILY_NOT_SUPPORTED;
        case WSATRY_AGAIN:
        case WSAEWOULDBLOCK:        return GEKKOTA_ERROR_NO_RESOURCE_AVAILABLE;
        case WSAEINPROGRESS:        return GEKKOTA_ERROR_OPERATION_IN_PROGRESS;
        case WSAEALREADY:           return GEKKOTA_ERROR_OPERATION_ALREADY_IN_PROGRESS;
        case WSAECONNABORTED:       return GEKKOTA_ERROR_CONNECTION_ABORTED;
        case WSAECONNREFUSED:       return GEKKOTA_ERROR_CONNECTION_REFUSED;
        case WSAECONNRESET:         return GEKKOTA_ERROR_CONNECTION_RESET;
        case WSAEDESTADDRREQ:       return GEKKOTA_ERROR_DESTINATION_ADDRESS_REQUIRED;
        case WSAEHOSTDOWN:          return GEKKOTA_ERROR_HOST_IS_DOWN;
        case WSAEHOSTUNREACH:       return GEKKOTA_ERROR_HOST_NOT_REACHABLE;
        case WSAEINTR:              return GEKKOTA_ERROR_OPERATION_INTERRUPTED;
        case WSAEISCONN:            return GEKKOTA_ERROR_SOCKET_ALREADY_CONNECTED;
        case WSAEREFUSED:
        case WSAELOOP:              return GEKKOTA_ERROR_CANNOT_TRANSLATE_NAME;
        case WSAEMFILE:             return GEKKOTA_ERROR_TOO_MANY_OPEN_SOCKETS;
        case WSAEMSGSIZE:           return GEKKOTA_ERROR_MESSAGE_TOO_LONG;
        case WSAENAMETOOLONG:       return GEKKOTA_ERROR_NAME_TOO_LONG;
        case WSAETIMEDOUT:          return GEKKOTA_ERROR_TIMEOUT;
        case WSAENETDOWN:           return GEKKOTA_ERROR_NETWORK_IS_DOWN;
        case WSAENETRESET:          return GEKKOTA_ERROR_CONNECTION_ABORTED_BY_NETWORK;
        case WSAENETUNREACH:        return GEKKOTA_ERROR_NETWORK_NOT_REACHABLE;
        case WSAEFAULT:             return GEKKOTA_ERROR_BUFFER_OVERFLOW;
        case WSAENOBUFS:            return GEKKOTA_ERROR_NO_BUFFER_SPACE_AVAILABLE;
        case WSAENOTCONN:           return GEKKOTA_ERROR_SOCKET_NOT_CONNECTED;
        case WSAEOPNOTSUPP:         return GEKKOTA_ERROR_OPERATION_NOT_SUPPORTED;
        case WSAESHUTDOWN:          return GEKKOTA_ERROR_SOCKET_ALREADY_SHUTDOWN;
        case WSASYSNOTREADY:        return GEKKOTA_ERROR_NETWORK_NOT_READY;
        case WSAVERNOTSUPPORTED:    return GEKKOTA_ERROR_SOCKET_VERSION_NOT_SUPPORTED;
        case WSAEPROCLIM:           return GEKKOTA_ERROR_SOCKET_TASKS_LIMIT_REACHED;
        default: break;
	}

    return defaultError;
}
