#include "client.h"
#include "window.h"

#include <stdio.h>
#include <pthread.h>

ENetHost *client = NULL;
ENetAddress address;
ENetEvent event;
ENetPeer *peer = NULL;
pthread_t msg_thread;

char usrname[80];
int client_id = -1;

void send_packet(ENetPeer *peer, char *data) {
    ENetPacket *packet =
        enet_packet_create(data, strlen(data) + 1, ENET_PACKET_FLAG_RELIABLE);
    enet_peer_send(peer, 0, packet);
}

void parse_data(char *data, client_data **clients) {
    int type = 0;
    int id = 0;

    if (sscanf(data, "%d|%d", &type, &id) < 2) {
        return; 
    }

    if (id < 0 || id >= MAX_CLIENTS) {
        return;
    }

    switch (type) {
    case PACKET_BROADCAST:
        if (id != client_id) {
            char msg[80];
            sscanf(data, "%*d|%*d|%79[^\n]", msg);
            print_msg(clients[id]->usr_name, msg);
        }
        break;
    case PACKET_JOIN:
        if (id != client_id) {
            char new_usr[80];
            sscanf(data, "%*d|%*d|%s", new_usr);

            client_data *new_client = malloc(sizeof(client_data));
            new_client->id = id;
            new_client->usr_name = strdup(new_usr);;
            clients[id] = new_client;
        }
        break;
    case PACKET_RECEIVE:
        client_id = id;
        break;
    default:
        break;
    }
}

void *msg_loop(void *arg) {
    thread_data *data = (thread_data *)arg;
    while (1) {
        ENetEvent event;
        while (enet_host_service(data->client, &event, 0) > 0) {
            if (event.type == ENET_EVENT_TYPE_RECEIVE) {
                parse_data((char *)event.packet->data, data->clients);
                enet_packet_destroy(event.packet);
            }
        }
    }
    free(data);
    return NULL;
}

void client_init(client_data **clients) {
    if (enet_initialize()) {
        ERROR(stderr, "Failed to init ENet\n");
        exit(-1);
    }
    atexit(enet_deinitialize);

    client = enet_host_create(NULL, 1, 1, 0, 0);
    if (!client) {
        ERROR(stderr, "Failed to create client host\n");
        exit(-1);
    }

    enet_address_set_host(&address, "127.0.0.1");
    address.port = 7777;

    peer = enet_host_connect(client, &address, 1, 0);
    if (!peer) {
        ERROR(stderr, "No peers available for connection\n");
        exit(-1);
    }

    if (enet_host_service(client, &event, 5000) > 0 &&
        event.type == ENET_EVENT_TYPE_CONNECT) {
        printf("Connection successful\n");
    } else {
        enet_peer_reset(peer);
        printf("Connection failed\n");
        exit(0);
    }
    
    thread_data *data = malloc(sizeof(thread_data));
    data->clients = clients;
    data->client = client;
    pthread_create(&msg_thread, NULL, msg_loop, (void *)data);
}

void client_destroy() {
    pthread_cancel(msg_thread);
    pthread_join(msg_thread, NULL);

    enet_peer_disconnect(peer, 0);
    while (enet_host_service(client, &event, 3000) > 0) {
        if (event.type == ENET_EVENT_TYPE_RECEIVE) {
            enet_packet_destroy(event.packet);
        } else if (event.type == ENET_EVENT_TYPE_DISCONNECT) {
            printf("Disconnection successful\n");
        }
    }

}
