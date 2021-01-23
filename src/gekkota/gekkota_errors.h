/******************************************************************************
 * @file    gekkota_errors.h
 * @date    27-Oct-2006
 * @author  <a href="mailto:giuseppe.greco@agamura.com">Giuseppe Greco</a>
 *
 * Copyright (C) 2006 Agamura, Inc. - http://www.agamura.com
 * All right reserved.
 ******************************************************************************/

#ifndef __GEKKOTA_ERRORS_H__
#define __GEKKOTA_ERRORS_H__

typedef enum
{
    GEKKOTA_ERROR_SUCCESS                           = 0x00000000,
    GEKKOTA_ERROR_ADDRESS_ALREADY_IN_USE            = 0x00000001,
    GEKKOTA_ERROR_ADDRESS_FAMILY_NOT_SUPPORTED      = 0x00000002,
    GEKKOTA_ERROR_ADDRESS_NOT_AVAILABLE             = 0x00000003,
    GEKKOTA_ERROR_ARGUMENT_NOT_VALID                = 0x00000004,
    GEKKOTA_ERROR_BUFFER_TOO_SMALL                  = 0x00000005,
    GEKKOTA_ERROR_BUFFER_OVERFLOW                   = 0x00000006,
    GEKKOTA_ERROR_CANNOT_RELEASE_RESOURCE           = 0x00000007,
    GEKKOTA_ERROR_CANNOT_RESOLVE_HOSTNAME           = 0x00000008,
    GEKKOTA_ERROR_CANNOT_TRANSLATE_NAME             = 0x00000009,
    GEKKOTA_ERROR_CLIENT_ALREADY_CONNECTED          = 0x0000000A,
    GEKKOTA_ERROR_CLIENT_NOT_CONNECTED              = 0x0000000B,
    GEKKOTA_ERROR_CONNECTION_ABORTED                = 0x0000000C,
    GEKKOTA_ERROR_CONNECTION_ABORTED_BY_NETWORK     = 0x0000000D,
    GEKKOTA_ERROR_CONNECTION_REFUSED                = 0x0000000E,
    GEKKOTA_ERROR_CONNECTION_RESET                  = 0x0000000F,
    GEKKOTA_ERROR_DESTINATION_ADDRESS_REQUIRED      = 0x00000010,
    GEKKOTA_ERROR_HOST_IS_DOWN                      = 0x00000011,
    GEKKOTA_ERROR_HOST_NOT_FOUND                    = 0x00000012,
    GEKKOTA_ERROR_HOST_NOT_REACHABLE                = 0x00000013,
    GEKKOTA_ERROR_LIB_ALREADY_INITIALIZED           = 0x00000014,
    GEKKOTA_ERROR_LIB_NOT_INITIALIZED               = 0x00000015,
    GEKKOTA_ERROR_MAX_CLIENT_REACHED                = 0x00000016,
    GEKKOTA_ERROR_MEMORY_ALREADY_INITIALIZED        = 0x00000017,
    GEKKOTA_ERROR_MEMORY_NOT_INITIALIZED            = 0x00000018,
    GEKKOTA_ERROR_MESSAGE_CORRUPTED                 = 0x00000019,
    GEKKOTA_ERROR_MESSAGE_TOO_LONG                  = 0x0000001A,
    GEKKOTA_ERROR_MESSAGE_TRUNCATED                 = 0x0000001B,
    GEKKOTA_ERROR_MULTICAST_GROUP_ALREADY_JOINED    = 0x0000001C,
    GEKKOTA_ERROR_NAME_TOO_LONG                     = 0x0000001D,
    GEKKOTA_ERROR_NETWORK_FAILURE                   = 0x0000001E,
    GEKKOTA_ERROR_NETWORK_IS_DOWN                   = 0x0000001F,
    GEKKOTA_ERROR_NETWORK_NOT_REACHABLE             = 0x00000020,
    GEKKOTA_ERROR_NETWORK_NOT_READY                 = 0x00000021,
    GEKKOTA_ERROR_NO_BUFFER_SPACE_AVAILABLE         = 0x00000022,
    GEKKOTA_ERROR_NO_RESOURCE_AVAILABLE             = 0x00000023,
    GEKKOTA_ERROR_NULL_ARGUMENT                     = 0x00000024,
    GEKKOTA_ERROR_NULL_BUFFER                       = 0x00000025,
    GEKKOTA_ERROR_OPERATION_ALREADY_IN_PROGRESS     = 0x00000026,
    GEKKOTA_ERROR_OPERATION_IN_PROGRESS             = 0x00000027,
    GEKKOTA_ERROR_OPERATION_INTERRUPTED             = 0x00000028,
    GEKKOTA_ERROR_OPERATION_NOT_SUPPORTED           = 0x00000029,
    GEKKOTA_ERROR_OPERATION_NOT_VALID               = 0x0000002A,
    GEKKOTA_ERROR_OUT_OF_MEMORY                     = 0x0000002B,
    GEKKOTA_ERROR_PERMISSION_DENIED                 = 0x0000002C,
    GEKKOTA_ERROR_PLATFORM_NOT_SUPPORTED            = 0x0000002D,
    GEKKOTA_ERROR_RESOURCE_NOT_AVAILABLE            = 0x0000002E,
    GEKKOTA_ERROR_SOCKET_ALREADY_CONNECTED          = 0x0000002F,
    GEKKOTA_ERROR_SOCKET_ALREADY_SHUTDOWN           = 0x00000030,
    GEKKOTA_ERROR_SOCKET_LIMIT_REACHED              = 0x00000031,
    GEKKOTA_ERROR_SOCKET_NOT_CONNECTED              = 0x00000032,
    GEKKOTA_ERROR_SOCKET_TASKS_LIMIT_REACHED        = 0x00000033,
    GEKKOTA_ERROR_SOCKET_TYPE_NOT_SUPPORTED         = 0x00000034,
    GEKKOTA_ERROR_SOCKET_VERSION_NOT_SUPPORTED      = 0x00000035,
    GEKKOTA_ERROR_TIMEOUT                           = 0x00000036,
    GEKKOTA_ERROR_TOO_MANY_OPEN_SOCKETS             = 0x00000037,
    GEKKOTA_ERROR_TRY_AGAIN                         = 0x00000038,
    GEKKOTA_ERROR_UNKNOWN_PLATFORM                  = 0x00000039,
    GEKKOTA_ERROR_ZERO_LENGTH_BUFFER                = 0x0000003A
} GekkotaError;

#endif /* !__GEKKOTA_ERRORS_H__ */