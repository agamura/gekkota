/******************************************************************************
 * @file    gekkota_test_client.c
 * @date    August 21, 2007
 * @author  Giuseppe Greco <giuseppe.greco@agamura.com>
 *
 * Copyright (C) 2007 Agamura, Inc. <http://www.agamura.com>
 * All right reserved.
 ******************************************************************************/

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gekkota/gekkota.h"

#define GEKKOTA_TEST_DEFAULT_SERVER "0.0.0.0"
#define GEKKOTA_TEST_DEFAULT_PORT 9050
#define GEKKOTA_TEST_DEFAULT_MULTICAST_GROUP "224.100.0.1"
#define GEKKOTA_TEST_DEFAULT_INCOMING_BANDWIDTH 0
#define GEKKOTA_TEST_DEFAULT_OUTGOING_BANDWIDTH 0
#define GEKKOTA_TEST_DEFAULT_TIMEOUT 5000
#define GEKKOTA_TEST_DEFAULT_MESSAGE "Gekkota Test Message"

static char_t *server = GEKKOTA_TEST_DEFAULT_SERVER;
static uint16_t port = GEKKOTA_TEST_DEFAULT_PORT;
static char_t *multicastGroup = GEKKOTA_TEST_DEFAULT_MULTICAST_GROUP;
static uint32_t incomingBandwidth = GEKKOTA_TEST_DEFAULT_INCOMING_BANDWIDTH;
static uint32_t outgoingBandwidth = GEKKOTA_TEST_DEFAULT_OUTGOING_BANDWIDTH;
static int32_t timeout = GEKKOTA_TEST_DEFAULT_TIMEOUT;
static char_t *message = GEKKOTA_TEST_DEFAULT_MESSAGE;

static int32_t
gekkota_test_send(GekkotaXudp *xudp, GekkotaPacket *packet);

static int32_t
gekkota_test_multicast(GekkotaXudp *xudp, GekkotaPacket *packet);

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
    GekkotaPacket *packet;
    GekkotaBuffer data;

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
     * Create the Gekkota client.
     */
    if ((xudp = gekkota_xudp_new_3(1)) == NULL)
    {
        fprintf(stderr, "Error while creating the Gekkota client - RC 0x%08X.\n",
                gekkota_get_last_error());
        goto main_exit;
    }

    if (incomingBandwidth != 0)
        gekkota_xudp_set_incoming_bandwidth(xudp, incomingBandwidth);

    if (outgoingBandwidth != 0)
        gekkota_xudp_set_outgoing_bandwidth(xudp, outgoingBandwidth);

    /*
     * Initialize packet data.
     */
    data.data = message;
    data.length = strlen((char_t *) data.data) + sizeof(char_t);

    /*
     * Create the packet.
     */
    if ((packet = gekkota_packet_new_2(
            &data,
         // GEKKOTA_PACKET_FLAG_RELIABLE |
            GEKKOTA_PACKET_FLAG_COMPRESSED)) == NULL)
    {
        fprintf(stderr, "Error while creating packet - RC 0x%08X.\n",
                gekkota_get_last_error());
        goto main_exit;
    }

    //rc = gekkota_test_send(xudp, packet);

    // if ((rc = gekkota_test_send(xudp, packet)) == 0)
         rc = gekkota_test_multicast(xudp, packet);

main_exit:
    gekkota_packet_destroy(packet);
    gekkota_xudp_destroy(xudp);
    gekkota_uninitialize();

    return rc;
}

static int32_t
gekkota_test_send(GekkotaXudp *xudp, GekkotaPacket* packet)
{
    int32_t rc = -1;
    GekkotaXudpClient *client;
    GekkotaEvent *event;

    /*
     * Connect to the remote server.
     */
    if ((client = gekkota_xudp_connect_2(
            xudp, server, port,
            0,  /* default channel count */
            GEKKOTA_COMPRESSION_LEVEL_FAST)) == NULL)
    {
        fprintf(stderr, "Error while connectig to %s:%d - RC 0x%08X.\n",
                server, port, gekkota_get_last_error());
        return -1;
    }

    /*
     * Wait for remote server acknowledgement, up to [timeout] milliseconds.
     */
    switch (gekkota_xudp_poll(xudp, &event, timeout))
    {
        case -1:
            fprintf(stderr, "Error while connecting to %s:%d - RC 0x%08X.\n",
                server, port, gekkota_get_last_error());
            goto gekkota_test_send_exit;

        case 0:
            fprintf(stderr, "Connection to %s:%d failed.\n", server, port);
            goto gekkota_test_send_exit;

        default:
            if (event != NULL &&
                gekkota_event_get_type(event) == GEKKOTA_EVENT_TYPE_CONNECT)
                fprintf(stdout, "Connection to %s:%d succeeded.\n", server, port);
            break;
    }

    fprintf(stdout, "Sending message \"%s\"... ", message);

    /*
     * Queue the packet for later delivery.
     */
    if (gekkota_xudpclient_send(client, 0, packet) != 0)
        fprintf(stdout, "failed - RC 0x%08X.\n", gekkota_get_last_error());
    else
    {
        /*
         * Let the underlying protocol implementation physically send the
         * packet - one could alternatively use gekkota_xudp_poll() instead.
         */
        if ((rc = gekkota_xudp_flush(xudp)) == 0)
            fprintf(stdout, "done.\n");
        else
            fprintf(stdout, "failed - RC 0x%08X.\n", gekkota_get_last_error());
    }

    if (rc != 0 && stdout != stderr)
        fprintf(stderr, "Error while sending packet to %s:%d - RC 0x%08X.\n",
                server, port, gekkota_get_last_error());

gekkota_test_send_exit:
    gekkota_xudpclient_destroy(client);
    return rc;
}

static int32_t
gekkota_test_multicast(GekkotaXudp *xudp, GekkotaPacket* packet)
{
    int32_t rc = -1;
    GekkotaIPAddress *address;
    GekkotaIPEndPoint *endPoint;
    GekkotaXudpClient *client;

    if ((address = gekkota_ipaddress_new(multicastGroup)) == NULL)
        return -1;

    endPoint = gekkota_ipendpoint_new(address, port);
    gekkota_ipaddress_destroy(address);

    if (endPoint == NULL)
        return -1;

    if ((client = gekkota_xudp_join_multicast_group(
            xudp, endPoint, 0, 0)) == NULL)
    {
        fprintf(stderr,
                "Error while joining multicast group % s - RC 0x%08X.\n",
                multicastGroup,
                gekkota_get_last_error());

        goto gekkota_test_multicast_exit;
    }

    fprintf(stdout, "Multicasting message \"%s\"... ", message);

    if (gekkota_xudpclient_send(client, 0, packet) != 0)
        fprintf(stdout, "failed - RC 0x%08X.\n", gekkota_get_last_error());
    else
    {
        /*
         * Let the underlying protocol implementation physically send the
         * packet - one could alternatively use gekkota_xudp_poll() instead.
         */
        if ((rc = gekkota_xudp_flush(xudp)) == 0)
            fprintf(stdout, "done.\n");
        else
            fprintf(stdout, "failed - RC 0x%08X.\n", gekkota_get_last_error());
    }

    if (rc != 0 && stdout != stderr)
        fprintf(stderr, "Error while multicasting packet to %s - RC 0x%08X.\n",
                multicastGroup, gekkota_get_last_error());

gekkota_test_multicast_exit:
    gekkota_ipaddress_destroy(address);
    return rc;
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

                case 'm':
                    if (argv[i + 1])
                    {
                        if (argv[i + 1][0] != '-')
                        {
                            message = argv[++i];
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
            "\nGekkota Test Client.\n");
    fprintf(stderr,
            "\nUsage: %s %s %s %s\n\t%s %s %s %s\n\n",
            programName, "[-s remoteServer]", "[-p remotePort]", "[-j multicastGroup]",
            "[-i incomingBandwidth]", "[-o outgoingBandwidth]",
            "[-t timeout]", "[-m message]");
    fprintf(stderr,
            "  remoteServer      Server name or IP address. (default %s)\n",
            GEKKOTA_TEST_DEFAULT_SERVER);
    fprintf(stderr,
            "  remotePort        Port to which to connect. (default %d)\n",
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
    fprintf(stderr,
            "  message           Text message to be sent. (default %s)\n",
            GEKKOTA_TEST_DEFAULT_MESSAGE);
}

static void_t
gekkota_test_print_parameters(void_t)
{
    fprintf(stdout, "\nRemote Server: %s\n", server);
    fprintf(stdout, "Remote Port: %d\n", port);
    fprintf(stdout, "Multicast Group: %s\n", multicastGroup);
    fprintf(stdout, "Incoming bandwidth: %d", incomingBandwidth);
    if (incomingBandwidth == 0)
        fprintf(stdout, " (Unlimited)");
    fprintf(stdout, "\nOutgoing Bandwidth: %d", outgoingBandwidth);
    if (outgoingBandwidth == 0)
        fprintf(stdout, " (Unlimited)");
    fprintf(stdout, "\nTimeout: %d\n", timeout);
}
