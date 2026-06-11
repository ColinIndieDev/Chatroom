#include "client.h"
#include "window.h"

#include <pthread.h>
#include <stdio.h>

ENetHost *client = NULL;
ENetAddress address;
ENetEvent event;
ENetPeer *peer = NULL;
pthread_t msg_thread;
bool running = true;

char usrname[80];
int client_id = -1;

void send_packet(ENetPeer *peer, char *data) {
    ENetPacket *packet =
        enet_packet_create(data, strlen(data) + 1, ENET_PACKET_FLAG_RELIABLE);
    enet_peer_send(peer, 0, packet);
}

void parse_data(char *data, client_map *clients) {
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
            print_msg((*client_map_get(clients, id))->usr_name, msg);
        }
        break;
    case PACKET_JOIN:
        if (id != client_id) {
            char new_usr[80];
            sscanf(data, "%*d|%*d|%79[^\n]", new_usr);

            client_data *new_client = malloc(sizeof(client_data));
            new_client->id = id;
            new_client->usr_name = strdup(new_usr);
            client_map_put(clients, id, new_client);

            char msg[126];
            snprintf(msg, sizeof(msg), "%s joined", new_usr);
            print_announcement(msg);
        }
        break;
    case PACKET_RECEIVE:
        client_id = id;
        break;
    case PACKET_DISCONNECT: {
        char msg[126];
        snprintf(msg, sizeof(msg), "%s left",
                 (*client_map_get(clients, id))->usr_name);
        print_announcement(msg);
        client_map_remove(clients, id);
        break;
    }
    default:
        break;
    }
}

void *msg_loop(void *arg) {
    thread_data *data = (thread_data *)arg;
    while (running) {
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

char *read_file_to_string(char *name) {
    FILE *file = fopen(name, "rb");
    if (!file) {
        return NULL;
    }

    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return NULL;
    }
    long file_size = ftell(file);
    if (file_size == -1) {
        fclose(file);
        return NULL;
    }
    if (fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        return NULL;
    }

    char *buffer = malloc(file_size + 1);
    if (!buffer) {
        fclose(file);
        return NULL;
    }

    size_t bytes_read = fread(buffer, 1, file_size, file);
    if (bytes_read != file_size) {
        free(buffer);
        fclose(file);
        return NULL;
    }

    buffer[file_size] = '\0';
    fclose(file);
    return buffer;
}

void client_init(client_map *clients) {
    client_map_init(clients, MAX_CLIENTS);

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

    char *ip = read_file_to_string("ip.txt");
    if (!ip) {
        ERROR(stderr, "Cannot read ip in \"ip.txt\"\n");
        exit(-1);
    }
    ip[strcspn(ip, "\r\n")] = '\0';

    enet_address_set_host(&address, ip);
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

void client_destroy(client_map *clients) {
    FOREACH_HM(client_map, client, clients) {
        if (client->state == HASH_OCCUPIED) {
            free(client->value->usr_name);
            free(client->value);
        }
    }
    client_map_destroy(clients);

    running = false;
    pthread_cancel(msg_thread);
    pthread_join(msg_thread, NULL);

    enet_peer_disconnect(peer, 0);
    while (enet_host_service(client, &event, 3000) > 0) {
        if (event.type == ENET_EVENT_TYPE_RECEIVE) {
            enet_packet_destroy(event.packet);
        } else if (event.type == ENET_EVENT_TYPE_DISCONNECT) {
            printf("Disconnection successful\n");
            break;
        }
    }
    free(client);
    free(peer);
}
