#define ENET_IMPLEMENTATION
#include <enet.h>

#include <ncurses.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cpstd/cphash.h>

#include "client.h"
#include "window.h"

EXTERN_CLIENT_H_VARIABLES

HASHMAP_IMPL(int, client_data *, client_map)

client_map clients;

void program_loop();

int main(void) {
    printf("Please enter your username below:\n");
    scanf("%79[^\n]", usrname);

    client_init(&clients);

    char usr_data[80] = "2|";
    strcat(usr_data, usrname);
    send_packet(peer, usr_data);

    program_loop();

    client_destroy(&clients);
}

void program_loop() {
    screen_init();
    while (1) {
        char *msg = check_msg_input();
        if (strcmp(msg, "/exit") == 0) {
            free(msg);
            break;
        }
        if (strcmp(msg, "/id") == 0) {
            free(msg);
            char id_msg[16];
            snprintf(id_msg, sizeof(id_msg), "My ID=%d", client_id);
            print_announcement(id_msg);
            continue;
        }
        if (strcmp(msg, "/id_share") == 0) {
            free(msg);
            char id_msg[16];
            snprintf(id_msg, sizeof(id_msg), "My ID=%d", client_id);
            print_msg(usrname, id_msg);

            char msg_data[80] = "1|";
            strcat(msg_data, id_msg);
            send_packet(peer, msg_data);
            continue;
        }
        print_msg(usrname, msg);

        char msg_data[80] = "1|";
        strcat(msg_data, msg);
        send_packet(peer, msg_data);

        free(msg);
    }
    screen_close();
}
