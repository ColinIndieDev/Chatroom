#pragma once

#include <cpstd/cphash.h>
#include <enet.h>

#define ERROR fprintf
#define MAX_CLIENTS 32

#define EXTERN_CLIENT_H_VARIABLES                                              \
    extern ENetPeer *peer;                                                     \
    extern char usrname[80];                                                   \
    extern int client_id;

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
HASHMAP_DECL(int, client_data *, client_map)

typedef struct {
    ENetHost *client;
    client_map *clients;
} thread_data;

void send_packet(ENetPeer *peer, char *data);
void parse_data(char *data, client_map *clients);

void client_init(client_map *clients);
void client_destroy();
