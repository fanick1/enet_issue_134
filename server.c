/*
 * server.c
 *
 *  Created on: Aug 23, 2020
 *      Author: fanick
 */
// compile:
//  gcc ./server.c -lenet -o server
//  gcc ./client.c -lenet -o client
// establish "suitable" network conditions:
//
//  sudo tc qdisc add dev lo root netem gap 2 delay 10ms reorder 100
//                         ^-localhost      ^- some packet reordering (These values "work" for the demonstration)
//
// start the server:
//  ./server &
// [1] 30154
//start the client
//  ./client
//
//  Connection to 127.0.0.1:1234 succeeded.
//  Expecting to crash soon.
//  A new client connected from 100007f:48210.
//  A packet of length 2000 was received on channel 0.
//  Segmentation fault (core dumped)
// to clear the packet reordering (after the test):
//  sudo tc qdisc del dev lo root
//
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include <enet/enet.h>

/**
 * The code is nothing fancy.
 * Glued parts from the tutorial together to establish connection on the localhost and start sending
 * packets with 2K data and flags ENET_PACKET_FLAG_UNSEQUENCED | ENET_PACKET_FLAG_UNRELIABLE_FRAGMENT
 */

int main() {

    ENetAddress address;
    ENetHost *server;
    /* Bind the server to the default localhost.     */
    /* A specific host address can be specified by   */
    /* enet_address_set_host (& address, "x.x.x.x"); */
    address.host = ENET_HOST_ANY;
    /* Bind the server to port 1234. */
    address.port = 1234;
    server = enet_host_create(&address /* the address to bind the server host to */,
        32 /* allow up to 32 clients and/or outgoing connections */,
        2 /* allow up to 2 channels to be used, 0 and 1 */,
        0 /* assume any amount of incoming bandwidth */,
        0 /* assume any amount of outgoing bandwidth */);
    if (server == NULL)
    {
        fprintf(stderr,
            "An error occurred while trying to create an ENet server host.\n");
        exit(EXIT_FAILURE);
    }

    ENetPeer *peer;
    ENetEvent event;
    bool peerConnected = false;

    void crashPeer() {
        while (enet_host_service(server, &event, 0) > 0)
        {
            switch (event.type)
            {
            case ENET_EVENT_TYPE_NONE:
                break;
            case ENET_EVENT_TYPE_CONNECT:
                printf("A new client connected from %x:%u. (UNEXPECTED)\n",
                    event.peer->address.host,
                    event.peer->address.port);
                /* Store any relevant client information here. */
                event.peer->data = "The client";
                peer = event.peer;
                peerConnected = true;

                break;

            case ENET_EVENT_TYPE_DISCONNECT:
                printf("%s disconnected. (expected ?) \n", (char*) event.peer->data);
                /* Reset the peer's client information. */
                event.peer->data = NULL;
                peerConnected = false;
                break;
            case ENET_EVENT_TYPE_RECEIVE:
                printf("A packet of length %lu containing %s was received from %s on channel %u.\n",
                    event.packet->dataLength,
                    (char*) event.packet->data,
                    (char*) event.peer->data,
                    event.channelID);
                /* Clean up the packet now that we're done using it. */
                enet_packet_destroy(event.packet);

                break;
            }
        }

        // sending 2000 B data in the packet to be > MTU to trigger fragmentation
        // flags must be ENET_PACKET_FLAG_UNSEQUENCED | ENET_PACKET_FLAG_UNRELIABLE_FRAGMENT

        char *data = malloc(2000);
        memset(data, 42, 2000);
        data[1999] = 0;
        ENetPacket *packet = enet_packet_create(data,
            2000,
            ENET_PACKET_FLAG_UNSEQUENCED | ENET_PACKET_FLAG_UNRELIABLE_FRAGMENT);

        /* Send the packet to the peer over channel id 0. */
        enet_peer_send(peer, 0, packet);
    }

    void awaitConnection() {
        while (enet_host_service(server, &event, 1000) > 0)
        {
            switch (event.type)
            {
            case ENET_EVENT_TYPE_NONE:
                break;
            case ENET_EVENT_TYPE_CONNECT:
                printf("A new client connected from %x:%u.\n",
                    event.peer->address.host,
                    event.peer->address.port);
                /* Store any relevant client information here. */
                event.peer->data = "Client information";
                peer = event.peer;
                peerConnected = true;

                break;

            case ENET_EVENT_TYPE_DISCONNECT:
                printf("%s disconnected.\n", (char*) event.peer->data);
                /* Reset the peer's client information. */
                event.peer->data = NULL;
                peerConnected = false;
                break;
            case ENET_EVENT_TYPE_RECEIVE:
                printf("A packet of length %lu containing %s was received from %s on channel %u.\n",
                    event.packet->dataLength,
                    (char*) event.packet->data,
                    (char*) event.peer->data,
                    event.channelID);
                /* Clean up the packet now that we're done using it. */
                enet_packet_destroy(event.packet);

                break;
            }
        }
    }
    for (;;) {
        if (peerConnected) {
            crashPeer();
        } else {
            awaitConnection();
        }
    }

    enet_host_destroy(server);

    return 0;

}
