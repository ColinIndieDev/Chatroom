#define ENET_IMPLEMENTATION
#include <enet.h>
#include <stdio.h>

// #include <cpstd/cphash.h>

#define ERROR fprintf

#define MAX_CLIENTS 32

typedef enum {
    PACKET_BROADCAST = 1,
    PACKET_JOIN,
    PACKET_RECEIVE,
    PACKET_DISCONNECT
} packet_types;

typedef struct {
    int id;
    char *usr_name;
} client_data;
// HASHMAP_DEF(int, client_data, client_map_t)

// client_map_t client_map;

client_data clients[MAX_CLIENTS];

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

void parse_data(ENetHost *server, int id, char *data) {
    printf("Parsing \"%s\"...\n", data);

    int type = 0;
    sscanf(data, "%d|", &type);

    switch (type) {
    case PACKET_BROADCAST: {
        char msg[80];
        sscanf(data, "%*d|%79[^\n]", msg);

        char send_data[1024] = {'\0'};
        sprintf(send_data, "1|%d|%s", id, msg);
        broadcast_packet(server, send_data);
        break;
    }
    case PACKET_JOIN: {
        char usr_name[80];
        sscanf(data, "2|%s", usr_name);

        char send_data[1024] = {'\0'};
        sprintf(send_data, "2|%d|%s", id, usr_name);

        printf("Send: \"%s\"\n", send_data);
        // client_map_t_put(&client_map, id,
        //                  (client_data){.id = id, .usr_name = usr_name});
        clients[id].id = id;
        clients[id].usr_name = strdup(usr_name);
        broadcast_packet(server, send_data);
        break;
    }
    default:
        break;
    }
}

int main(void) {
    // client_map_t_init(&client_map, MAX_CLIENTS);

    if (enet_initialize()) {
        ERROR(stderr, "Failed to init ENet\n");
        return -1;
    }
    atexit(enet_deinitialize);

    ENetAddress address;
    address.host = ENET_HOST_ANY;
    address.port = 7777;

    ENetHost *server = enet_host_create(&address, MAX_CLIENTS, 1, 0, 0);
    if (!server) {
        ERROR(stderr, "Failed to create server host\n");
        return -1;
    }

    int new_usr_id = 0;
    while (1) {
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
                printf("A new client connected from %s:%u.\n", display_ip,
                       event.peer->address.port);

                for (int i = 0; i < MAX_CLIENTS; i++) {
                    char send_data[1024] = {'\0'};
                    if (clients[i].usr_name) {
                        sprintf(send_data, "2|%d|%s", i, clients[i].usr_name);
                        broadcast_packet(server, send_data);
                    }
                }
                new_usr_id++;
                clients[new_usr_id].id = new_usr_id;
                event.peer->data = &clients[new_usr_id];

                char send_data[126] = {'\0'};
                sprintf(send_data, "3|%d", new_usr_id);
                send_packet(event.peer, send_data);

                break;
            }
            case ENET_EVENT_TYPE_RECEIVE: {
                printf(
                    "A packet of length %u containing \"%s\" was received from "
                    "%s on channel %u.\n",
                    (unsigned int)event.packet->dataLength, event.packet->data,
                    (char *)event.peer->data, event.channelID);

                client_data *client = event.peer->data;
                parse_data(server, client->id, (char *)event.packet->data);
                enet_packet_destroy(event.packet);
                break;
            }
            case ENET_EVENT_TYPE_DISCONNECT: {
                printf("%s disconnected.\n", (char *)event.peer->data);
                char disconnected_data[126] = {'\0'};
                client_data *client = event.peer->data;
                sprintf(disconnected_data, "4|%d", client->id);
                broadcast_packet(server, disconnected_data);
                event.peer->data = NULL;
                break;
            }
            default:
                break;
            }
        }
    }
    enet_host_destroy(server);

    // client_map_t_destroy(&client_map);
}
