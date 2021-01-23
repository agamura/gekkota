/******************************************************************************
 * @file    gekkota_test_server.c
 * @date    August 21, 2007
 * @author  Giuseppe Greco <giuseppe.greco@agamura.com>
 *
 * Copyright (C) 2007 Agamura, Inc. <http://www.agamura.com>
 * All right reserved.
 ******************************************************************************/

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include "gekkota/gekkota.h"

#define GEKKOTA_TEST_DEFAULT_SERVER "0.0.0.0"
#define GEKKOTA_TEST_DEFAULT_PORT 9050
#define GEKKOTA_TEST_DEFAULT_MULTICAST_GROUP "224.100.0.1"
#define GEKKOTA_TEST_DEFAULT_INCOMING_BANDWIDTH 0
#define GEKKOTA_TEST_DEFAULT_OUTGOING_BANDWIDTH 0
#define GEKKOTA_TEST_DEFAULT_TIMEOUT -1

static char_t *server = GEKKOTA_TEST_DEFAULT_SERVER;
static uint16_t port = GEKKOTA_TEST_DEFAULT_PORT;
static char_t *multicastGroup = GEKKOTA_TEST_DEFAULT_MULTICAST_GROUP;
static uint32_t incomingBandwidth = GEKKOTA_TEST_DEFAULT_INCOMING_BANDWIDTH;
static uint32_t outgoingBandwidth = GEKKOTA_TEST_DEFAULT_OUTGOING_BANDWIDTH;
static int32_t timeout = GEKKOTA_TEST_DEFAULT_TIMEOUT;

static int32_t
gekkota_test_listen(GekkotaXudp *xudp);

static void_t
gekkota_test_handle_connect(GekkotaEvent *event);

static void_t
gekkota_test_handle_receive(GekkotaEvent *event);

static void_t
gekkota_test_handle_disconnect(GekkotaEvent *event);

static void_t
gekkota_test_handle_join_multicast_address(GekkotaEvent *event);

static void_t
gekkota_test_handle_leave_multicast_address(GekkotaEvent *event);

static int32_t
gekkota_test_parse_arguments(int32_t argc, char_t **argv);

static void_t
gekkota_test_print_usage(char_t *programName);

static void_t
gekkota_test_print_parameters(void_t);

int32_t main(int32_t argc, char_t **argv)
{
    int32_t rc = -1;
    GekkotaXudp *xudp;
    GekkotaIPAddress *address;
    GekkotaIPEndPoint *endPoint;
    GekkotaXudpClient *client;

    if (argc > 1)
    {
        if (gekkota_test_parse_arguments(argc, argv) != 0)
        {
            gekkota_test_print_usage(argv[0]);
            return -1;
        }
    }

    gekkota_test_print_parameters();

    if (gekkota_initialize() != 0)
    {
        fprintf(stderr, "Error while initializing Gekkota - RC 0x%08X.\n",
                gekkota_get_last_error());
        return -1;
    }

    /*
     * Check whether or not the underlying platform supports
     * Internationalized Domain Names (IDNs).
     */
    fprintf(stdout, "IDN Support: %s\n\n",
        gekkota_platform_supports_feature(
            GEKKOTA_PLATFORM_FEATURE_IDN) ? "Yes" : "No");

    /*
     * Create the Gekkota server.
     */
    if ((xudp = gekkota_xudp_new_6(server, port, 0)) == NULL)
    {
        fprintf(stderr, "Error while creating Gekkota server - RC 0x%08X.\n",
                gekkota_get_last_error());
        goto main_exit;
    }

    if (incomingBandwidth != 0)
        gekkota_xudp_set_incoming_bandwidth(xudp, incomingBandwidth);

    if (outgoingBandwidth != 0)
        gekkota_xudp_set_outgoing_bandwidth(xudp, outgoingBandwidth);

    if ((address = gekkota_ipaddress_new(multicastGroup)) == NULL)
        goto main_exit;

    endPoint = gekkota_ipendpoint_new(address, port);
    gekkota_ipaddress_destroy(address);

    if (endPoint == NULL)
        goto main_exit;

    if ((client = gekkota_xudp_join_multicast_group(
            xudp, endPoint, 0, 0)) == NULL)
    {
        fprintf(stderr,
                "Error while joining multicast group % s - RC 0x%08X.\n",
                multicastGroup,
                gekkota_get_last_error());

        goto main_exit;
    }

    rc = gekkota_test_listen(xudp);

main_exit:
    gekkota_ipendpoint_destroy(endPoint);
    gekkota_xudp_destroy(xudp);
    gekkota_uninitialize();

    return rc;
}

static int32_t
gekkota_test_listen(GekkotaXudp *xudp)
{
    GekkotaEvent *event;

    while (TRUE)
    {
        /*
         * Wait for incoming messages, up to [timeout] milliseconds.
         */
        switch (gekkota_xudp_poll(xudp, &event, timeout))
        {
            case -1:
                fprintf(stderr,
                        "Error while listening for incoming data - RC 0x%08X.\n",
                        gekkota_get_last_error());
                return -1;

            case 1:
                switch (gekkota_event_get_type(event))
                {
                    case GEKKOTA_EVENT_TYPE_CONNECT:
                        gekkota_test_handle_connect(event);
                        break;

                    case GEKKOTA_EVENT_TYPE_RECEIVE:
                        gekkota_test_handle_receive(event);
                        break;

                    case GEKKOTA_EVENT_TYPE_DISCONNECT:
                        gekkota_test_handle_disconnect(event);
                        break;

                    case GEKKOTA_EVENT_TYPE_JOIN_MULTICAST_GROUP:
                        gekkota_test_handle_join_multicast_address(event);
                        break;

                    case GEKKOTA_EVENT_TYPE_LEAVE_MULTICAST_GROUP:
                        gekkota_test_handle_leave_multicast_address(event);
                        break;
                }
                break;

            default:
                return 0;
        }
    }

    /*
     * Execution never ends here.
     */
}

static void_t
gekkota_test_handle_connect(GekkotaEvent *event)
{
    char_t ipString[46];
    GekkotaBuffer remoteAddress;
    GekkotaIPEndPoint *remoteEndPoint;

    /*
     * Get the endpoint that initiated the connection.
     */
    remoteEndPoint = gekkota_event_get_remote_endpoint(event);

    if (remoteEndPoint == NULL)
        goto gekkota_test_handle_connect_exit;
    
    remoteAddress.data = ipString;
    remoteAddress.length = sizeof(ipString);

    /*
     * Convert the IP address into its text representation.
     */
    if (gekkota_ipaddress_to_string(
            gekkota_ipendpoint_get_address(remoteEndPoint),
            &remoteAddress) < 0)
    {
        fprintf(stderr,
                "Error while converting the remote IP address - RC 0x%08X.\n",
                gekkota_get_last_error());
        goto gekkota_test_handle_connect_exit;
    }

    fprintf(stdout, "A new client connected from %s:%u.\n",
            (char_t *) remoteAddress.data,
            gekkota_ipendpoint_get_port(remoteEndPoint));

gekkota_test_handle_connect_exit:
    gekkota_event_destroy(event);
}

static void_t
gekkota_test_handle_receive(GekkotaEvent *event)
{
    char_t ipString[46];
    GekkotaBuffer remoteAddress;
    GekkotaIPEndPoint *remoteEndPoint;
    GekkotaPacket *packet;

    /*
     * Get the endpoint that sent the packet.
     */
    remoteEndPoint = gekkota_event_get_remote_endpoint(event);

    if (remoteEndPoint == NULL)
        goto gekkota_test_handle_receive_exit;
    
    remoteAddress.data = ipString;
    remoteAddress.length = sizeof(ipString);

    /*
     * Convert the IP address to its text representation.
     */
    if (gekkota_ipaddress_to_string(
            gekkota_ipendpoint_get_address(remoteEndPoint),
            &remoteAddress) < 0)
    {
        fprintf(stderr,
                "Error while converting the remote IP address - RC 0x%08X.\n",
                gekkota_get_last_error());
        goto gekkota_test_handle_receive_exit;
    }

    fprintf(stdout, "Received packet from %s:%u.\n",
            (char_t *) remoteAddress.data,
            gekkota_ipendpoint_get_port(remoteEndPoint));

    packet = gekkota_event_get_packet(event);

    if (packet != NULL)
    {
        GekkotaBuffer *data = gekkota_packet_get_data(packet);

        if (data->length > 0)
            fprintf(stdout, "%s\n\n", (char_t *) data->data);
    }

gekkota_test_handle_receive_exit:
    gekkota_event_destroy(event);
}

static void_t
gekkota_test_handle_disconnect(GekkotaEvent *event)
{
    char_t ipString[46];
    GekkotaBuffer remoteAddress;
    GekkotaIPEndPoint *remoteEndPoint;

    /*
     * Get the endpoint that initiated the connection.
     */
    remoteEndPoint = gekkota_event_get_remote_endpoint(event);

    if (remoteEndPoint == NULL)
        goto gekkota_test_handle_disconnect_exit;
    
    remoteAddress.data = ipString;
    remoteAddress.length = sizeof(ipString);

    /*
     * Convert the IP address to its text representation.
     */
    if (gekkota_ipaddress_to_string(
            gekkota_ipendpoint_get_address(remoteEndPoint),
            &remoteAddress) < 0)
    {
        fprintf(stderr,
                "Error while converting the remote IP address - RC 0x%08X.\n",
                gekkota_get_last_error());
        goto gekkota_test_handle_disconnect_exit;
    }

    fprintf(stdout, "%s:%d disconnected.\n",
            (char_t *) remoteAddress.data,
            gekkota_ipendpoint_get_port(remoteEndPoint));

gekkota_test_handle_disconnect_exit:
    gekkota_event_destroy(event);
}

static void_t
gekkota_test_handle_join_multicast_address(GekkotaEvent *event)
{
    char_t ipString[46];
    GekkotaBuffer remoteAddress;
    GekkotaIPEndPoint *remoteEndPoint;

    /*
     * Get the endpoint that initiated the connection.
     */
    remoteEndPoint = gekkota_event_get_remote_endpoint(event);

    if (remoteEndPoint == NULL)
        goto gekkota_test_handle_join_multicast_address_exit;
    
    remoteAddress.data = ipString;
    remoteAddress.length = sizeof(ipString);

    /*
     * Convert the IP address into its text representation.
     */
    if (gekkota_ipaddress_to_string(
            gekkota_ipendpoint_get_address(remoteEndPoint),
            &remoteAddress) < 0)
    {
        fprintf(stderr,
                "Error while converting the remote IP address - RC 0x%08X.\n",
                gekkota_get_last_error());
        goto gekkota_test_handle_join_multicast_address_exit;
    }

    fprintf(stdout, "A new client joined multicast group %s from %s:%u.\n",
            multicastGroup,
            (char_t *) remoteAddress.data,
            gekkota_ipendpoint_get_port(remoteEndPoint));

gekkota_test_handle_join_multicast_address_exit:
    gekkota_event_destroy(event);
}

static void_t
gekkota_test_handle_leave_multicast_address(GekkotaEvent *event)
{
    char_t ipString[46];
    GekkotaBuffer remoteAddress;
    GekkotaIPEndPoint *remoteEndPoint;

    /*
     * Get the endpoint that initiated the connection.
     */
    remoteEndPoint = gekkota_event_get_remote_endpoint(event);

    if (remoteEndPoint == NULL)
        goto gekkota_test_handle_leave_multicast_address_exit;
    
    remoteAddress.data = ipString;
    remoteAddress.length = sizeof(ipString);

    /*
     * Convert the IP address into its text representation.
     */
    if (gekkota_ipaddress_to_string(
            gekkota_ipendpoint_get_address(remoteEndPoint),
            &remoteAddress) < 0)
    {
        fprintf(stderr,
                "Error while converting the remote IP address - RC 0x%08X.\n",
                gekkota_get_last_error());
        goto gekkota_test_handle_leave_multicast_address_exit;
    }

    fprintf(stdout, "%s:%d left multicast group %s.\n",
            (char_t *) remoteAddress.data,
            gekkota_ipendpoint_get_port(remoteEndPoint),
            multicastGroup);

gekkota_test_handle_leave_multicast_address_exit:
    gekkota_event_destroy(event);
}

static int32_t
gekkota_test_parse_arguments(int32_t argc, char_t **argv)
{
    int32_t i;

    for (i = 1; i < argc; i++)
    {
        if ((argv[i][0] == '-') || (argv[i][0] == '/') &&
            (argv[i][1] != 0) && (argv[i][2] == 0))
        {
            switch (tolower(argv[i][1]))
            {
                case 's':
                    if (argv[i + 1])
                    {
                        if (argv[i + 1][0] != '-')
                        {
                            server = argv[++i];
                            break;
                        }
                    }
                    return -1;

                case 'p':
                    if (argv[i + 1])
                    {
                        if (argv[i + 1][0] != '-')
                        {
                            port = atoi(argv[++i]);
                            break;
                        }
                    }
                    return -1;

                case 'j':
                    if (argv[i + 1])
                    {
                        if (argv[i + 1][0] != '-')
                        {
                            multicastGroup = argv[++i];
                            break;
                        }
                    }
                    return -1;

                case 'i':
                    if (argv[i + 1])
                    {
                        if (argv[i + 1][0] != '-')
                        {
                            incomingBandwidth = atoi(argv[++i]);
                            break;
                        }
                    }
                    return -1;

                case 'o':
                    if (argv[i + 1])
                    {
                        if (argv[i + 1][0] != '-')
                        {
                            outgoingBandwidth = atoi(argv[++i]);
                            break;
                        }
                    }
                    return -1;

                case 't':
                    if (argv[i + 1])
                    {
                        if (argv[i + 1][0] != '-')
                        {
                            timeout = atoi(argv[++i]);
                            break;
                        }
                    }
                    return -1;

                default:
                    return -1;
            }
        }
        else
            return -1;
    }

    return 0;
}

static void_t
gekkota_test_print_usage(char_t *programName)
{
    fprintf(stderr,
            "\nGekkota Test Server.\n");
    fprintf(stderr,
            "\nUsage: %s %s %s %s\n\t%s %s %s\n\n",
            programName, "[-s localServer]", "[-p localPort]", "[-j multicastGroup]",
            "[-i incomingBandwidth]", "[-o outgoingBandwidth]", "[-t timeout]");
    fprintf(stderr,
            "  localServer       Server name or IP address. (default %s)\n",
            GEKKOTA_TEST_DEFAULT_SERVER);
    fprintf(stderr,
            "  localPort         Port on which to listen for incoming data. (default %d)\n",
            GEKKOTA_TEST_DEFAULT_PORT);
    fprintf(stderr,
            "  multicastGroup    Multicast group to join. (default %s)\n",
            GEKKOTA_TEST_DEFAULT_MULTICAST_GROUP);
    fprintf(stderr,
            "  incomingBandwidth Amount of bits that can be received per second. (default %d)\n",
            GEKKOTA_TEST_DEFAULT_INCOMING_BANDWIDTH);
    fprintf(stderr,
            "  outgointBandwidth Amount of bits that can be sent per second. (default %d)\n",
            GEKKOTA_TEST_DEFAULT_OUTGOING_BANDWIDTH);
    fprintf(stderr,
            "  timeout           Amount of time, in milliseconds, to wait for events. (default %d)\n",
            GEKKOTA_TEST_DEFAULT_TIMEOUT);
}

static void_t
gekkota_test_print_parameters(void_t)
{
    fprintf(stdout, "\nLocal Server: %s\n", server);
    fprintf(stdout, "Local Port: %d\n", port);
    fprintf(stdout, "Multicast Group: %s\n", multicastGroup);
    fprintf(stdout, "Incoming Bandwidth: %d", incomingBandwidth);
    if (incomingBandwidth == 0)
        fprintf(stdout, " (Unlimited)");
    fprintf(stdout, "\nOutgoing Bandwidth: %d", outgoingBandwidth);
    if (outgoingBandwidth == 0)
        fprintf(stdout, " (Unlimited)");
    fprintf(stdout, "\nTimeout: %d", timeout);
    if (timeout == -1)
        fprintf(stdout, " (No Timeout)");
    fprintf(stdout, "\n");
}
