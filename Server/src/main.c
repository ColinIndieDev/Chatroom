#include <stdio.h>
#include <string.h>

#define ENET_IMPLEMENTATION
#include <enet.h>

#include "server.h"

client_entry *clients = NULL;

int main(void) {
    server_init(&clients);
    server_loop(&clients);
    server_destroy(&clients);
}
