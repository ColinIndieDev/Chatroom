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

void send_msg(char *msg);
void send_id();
void send_usr();

int main(void) {
    printf("Please enter your username below:\n");
    scanf("%79[^\n]", usrname);

    client_init(&clients);

    send_usr();

    program_loop();

    client_destroy(&clients);
}

void program_loop() {
    screen_init(usrname);
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
            send_id();
            continue;
        }
        print_msg("You", msg);
        send_msg(msg);
    }
    screen_close();
}

void send_msg(char *msg) {
    char msg_data[80] = "1|";
    char encrypted_msg[100] = {'\0'};
    encrypt_decrypt(msg, encrypted_msg, strlen(msg));
    char hex_buffer[160] = {'\0'};
    for (size_t i = 0; i < strlen(msg); i++) {
        sprintf(hex_buffer + (i * 2), "%02x", (unsigned char)encrypted_msg[i]);
    }
    free(msg);
    strcat(msg_data, hex_buffer);
    send_packet(peer, msg_data);
}

void send_id() {
    char id_msg[16];
    snprintf(id_msg, sizeof(id_msg), "My ID=%d", client_id);
    print_msg(usrname, id_msg);

    char msg_data[80] = "1|";
    char encrypted_msg[16];
    encrypt_decrypt(id_msg, encrypted_msg, strlen(id_msg));
    char hex_buffer[160] = {'\0'};
    for (size_t i = 0; i < strlen(id_msg); i++) {
        sprintf(hex_buffer + (i * 2), "%02x", (unsigned char)encrypted_msg[i]);
    }
    strcat(msg_data, hex_buffer);
    send_packet(peer, msg_data);
}

void send_usr() {
    char usr_data[80] = "2|";
    char encrypted[80];
    encrypt_decrypt(usrname, encrypted, strlen(usrname));
    char hex_buffer[160] = {'\0'};
    for (size_t i = 0; i < strlen(usrname); i++) {
        sprintf(hex_buffer + (i * 2), "%02x", (unsigned char)encrypted[i]);
    }
    strcat(usr_data, hex_buffer);
    send_packet(peer, usr_data);
}
