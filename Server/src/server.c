#include "server.h"

#include <signal.h>

ENetAddress address;
ENetHost *server = NULL;
volatile sig_atomic_t server_running = 1;

void handle_sigint([[maybe_unused]] int sig) {
    server_running = 0;
}

void broadcast_packet(ENetHost *server, char *data) {
    ENetPacket *packet =
        enet_packet_create(data, strlen(data) + 1, ENET_PACKET_FLAG_RELIABLE);
    enet_host_broadcast(server, 0, packet);
}

void send_packet(ENetPeer *peer, char *data) {
    ENetPacket *packet =
        enet_packet_create(data, strlen(data) + 1, ENET_PACKET_FLAG_RELIABLE);
    enet_peer_send(peer, 0, packet);
}

void parse_data(ENetHost *server, int id, char *data, client_map *clients) {
    printf("Parsing...\n");

    int type = 0;
    sscanf(data, "%d|", &type);

    switch (type) {
    case PACKET_BROADCAST: {
        char msg[80];
        sscanf(data, "%*d|%79[^\n]", msg);

        char send_data[1024] = {'\0'};
        snprintf(send_data, sizeof(send_data), "1|%d|%s", id, msg);
        broadcast_packet(server, send_data);
        break;
    }
    case PACKET_JOIN: {
        char usr_name[80];
        sscanf(data, "2|%79[^\n]", usr_name);

        char send_data[1024] = {'\0'};
        snprintf(send_data, sizeof(send_data), "2|%d|%s", id, usr_name);

        printf("Sending...\n");
        client_data *new_client = malloc(sizeof(client_data));
        new_client->id = id;
        new_client->usr_name = strdup(usr_name);
        client_map_put(clients, id, new_client);
        broadcast_packet(server, send_data);
        break;
    }
    default:
        break;
    }
}

void server_init(client_map *clients) {
    client_map_init(clients, MAX_CLIENTS);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        client_map_put(clients, i, NULL);
    }

    if (enet_initialize()) {
        ERROR(stderr, "Failed to init ENet\n");
        exit(-1);
    }
    atexit(enet_deinitialize);

    signal(SIGINT, handle_sigint);

    address.host = ENET_HOST_ANY;
    address.port = 7777;

    server = enet_host_create(&address, MAX_CLIENTS, 1, 0, 0);
    if (!server) {
        ERROR(stderr, "Failed to create server host\n");
        exit(-1);
    }
}

void server_loop(client_map *clients) {
    while (server_running) {
        ENetEvent event;
        while (enet_host_service(server, &event, 1000) > 0) {
            switch (event.type) {
            case ENET_EVENT_TYPE_CONNECT: {
                char ip[46];
                enet_address_get_host_ip(&event.peer->address, ip, sizeof(ip));
                char *display_ip = ip;
                if (strncmp(ip, "::ffff:", 7) == 0) {
                    display_ip = ip + 7;
                }
                printf("A new client connected from %s:%u!\n", display_ip,
                       event.peer->address.port);

                int assigned_id = -1;
                for (int i = 0; i < MAX_CLIENTS; i++) {
                    client_data **slot = client_map_get(clients, i);
                    if (slot == NULL || *slot == NULL) {
                        assigned_id = i;
                        break;
                    }
                }

                if (assigned_id == -1) {
                    printf("Server is full!\n");
                    enet_peer_reset(event.peer);
                    break;
                }

                for (int i = 0; i < MAX_CLIENTS; i++) {
                    client_data **existing = client_map_get(clients, i);
                    if (existing && *existing &&
                        (*existing)->usr_name != NULL) {
                        char send_data[1024] = {'\0'};
                        snprintf(send_data, sizeof(send_data), "2|%d|%s", i,
                                 (*existing)->usr_name);
                        send_packet(event.peer, send_data);
                    }
                }

                client_data *new_client = malloc(sizeof(client_data));
                new_client->id = assigned_id;
                new_client->usr_name = NULL;
                client_map_put(clients, assigned_id, new_client);
                event.peer->data = new_client;

                char send_data[126] = {'\0'};
                snprintf(send_data, sizeof(send_data), "3|%d", assigned_id);
                send_packet(event.peer, send_data);
                break;
            }
            case ENET_EVENT_TYPE_RECEIVE: {
                printf("A packet of length %u was received on channel %u!\n",
                       (unsigned int)event.packet->dataLength, event.channelID);

                client_data *client = event.peer->data;
                parse_data(server, client->id, (char *)event.packet->data,
                           clients);
                enet_packet_destroy(event.packet);
                break;
            }
            case ENET_EVENT_TYPE_DISCONNECT: {
                client_data *client = event.peer->data;
                printf("%s disconnected!\n",
                       (*client_map_get(clients, client->id))->usr_name);
                char disconnected_data[126] = {'\0'};
                snprintf(disconnected_data, sizeof(disconnected_data), "4|%d",
                         client->id);
                broadcast_packet(server, disconnected_data);
                client_map_remove(clients, client->id);
                event.peer->data = NULL;
                break;
            }
            default:
                break;
            }
        }
    }
}

void server_destroy(client_map *clients) {
    printf("Server shutting down...\n");

    char data[] = "5|0";
    broadcast_packet(server, data);

    enet_host_flush(server);
    usleep(500000);

    for (int i = 0; i < MAX_CLIENTS; i++) {
        client_data **client = client_map_get(clients, i);
        if (client && *client) {
            enet_peer_disconnect_now(server->peers, 0); 
        }
    }

    enet_host_destroy(server);
    FOREACH_HM(client_map, client, clients) {
        if (client->state == HASH_OCCUPIED) {
            free(client->value->usr_name);
            free(client->value);
        }
    }
}
