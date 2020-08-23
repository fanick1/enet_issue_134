/*
 * client.c
 *
 *  Created on: Aug 23, 2020
 *      Author: fanick
 */

#include <enet/enet.h>
#include <stdio.h>
int main() {

    ENetHost *client;
    client = enet_host_create(NULL /* create a client host */,
        1 /* only allow 1 outgoing connection */,
        2 /* allow up 2 channels to be used, 0 and 1 */,
        0 /* assume any amount of incoming bandwidth */,
        0 /* assume any amount of outgoing bandwidth */);
    if (client == NULL)
    {
        fprintf(stderr,
            "An error occurred while trying to create an ENet client host.\n");
        exit(EXIT_FAILURE);
    }
    ENetAddress address;
    ENetEvent event;
    ENetPeer *peer;
    /* Connect to some.server.net:1234. */
    enet_address_set_host(&address, "127.0.0.1");
    address.port = 1234;
    /* Initiate the connection, allocating the two channels 0 and 1. */
    peer = enet_host_connect(client, &address, 2, 0);
    if (peer == NULL)
    {
        fprintf(stderr,
            "No available peers for initiating an ENet connection.\n");
        exit(EXIT_FAILURE);
    }
    /* Wait up to 5 seconds for the connection attempt to succeed. */
    if (enet_host_service(client, &event, 5000) > 0 &&
        event.type == ENET_EVENT_TYPE_CONNECT)
            {
        puts("Connection to 127.0.0.1:1234 succeeded.");
        puts("Expecting to crash soon.");

        for (;;) {
            while (enet_host_service(client, &event, 500) > 0) {
                switch (event.type)
                {
                case ENET_EVENT_TYPE_NONE:
                    break;
                case ENET_EVENT_TYPE_CONNECT:
                    printf("Connect again... should not happen.\n");
                    break;

                case ENET_EVENT_TYPE_DISCONNECT:
                    printf("Disconnected... unexpected.\n");
                    break;
                case ENET_EVENT_TYPE_RECEIVE:
                    printf("A packet of length %lu was received on channel %u.\n",
                        event.packet->dataLength,
                        event.channelID);
                    /* Clean up the packet now that we're done using it. */
                    enet_packet_destroy(event.packet);

                    break;
                }
            }
        }
    }
    else
    {
        /* Either the 5 seconds are up or a disconnect event was */
        /* received. Reset the peer in the event the 5 seconds   */
        /* had run out without any significant event.            */
        enet_peer_reset(peer);
        puts("Connection to 127.0.0.1:1234 failed.");
    }

    return 0;
}

