#pragma once

#include <enet.h>

#include <cpstd/cphash.h>

#define ERROR fprintf
#define MAX_CLIENTS 32

typedef enum {
    PACKET_BROADCAST = 1,
    PACKET_JOIN,
    PACKET_RECEIVE,
    PACKET_DISCONNECT,
    PACKET_SERVER_SHUTDOWN
} packet_types;

typedef struct {
    int id;
    char *usr_name;
} client_data;

HM_CREATE_ENTRY(int, client_data *, client_entry)

void server_init(client_entry **clients);
void server_destroy(client_entry **clients);
void server_loop(client_entry **clients);
