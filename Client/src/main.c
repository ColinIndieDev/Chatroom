#define ENET_IMPLEMENTATION
#include <enet.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <ncurses.h>

#include "client.h"
#include "window.h"

EXTERN_CLIENT_H_VARIABLES

client_data *clients[MAX_CLIENTS];

void program();

int main(void) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        clients[i] = NULL;
    }

    printf("Please enter your username below (no spaces!):\n");
    scanf("%s", usrname);

    client_init(clients);

    char usr_data[80] = "2|";
    strcat(usr_data, usrname);
    send_packet(peer, usr_data);

    program();

    client_destroy();
}

void program() {
    screen_init();
    while (1) {
        char *msg = check_msg_input();
        if (strcmp(msg, "/exit") == 0) {
            free(msg);
            break;
        }
        print_msg(usrname, msg);

        char msg_data[80] = "1|";
        snprintf(msg_data, sizeof(msg_data), "1|%s", msg);
        send_packet(peer, msg_data);

        free(msg);
    }
    screen_close();
}
