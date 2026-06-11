#include <stdio.h>
#include <string.h>

#define ENET_IMPLEMENTATION
#include <enet.h>

#include <cpstd/cphash.h>

#include "server.h"

HASHMAP_IMPL(int, client_data *, client_map)

client_map clients;

int main(void) {
    server_init(&clients);
    server_loop(&clients);
    server_destroy(&clients);
}
