#include <string.h>
#define ENET_IMPLEMENTATION
#include <enet.h>
#include <ncurses.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

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

char usrname[80];
int client_id = -1;

client_data *clients[MAX_CLIENTS];

// {{{ Window

int msg_y = 0;
WINDOW *window = NULL;

void screen_close() {
    delwin(window);
    endwin();
}

void screen_init() {
    initscr();
    cbreak();
    noecho();
    int max_x = 0;
    int max_y = 0;
    getmaxyx(stdscr, max_y, max_x);
    window = newwin(3, max_x, max_y - 3, 0);
    box(window, 0, 0);
    refresh();
    wrefresh(window);
}

char *check_msg_input() {
    wclear(window);
    box(window, 0, 0);
    wrefresh(window);

    char *msg = malloc(100);

    echo();
    mvwscanw(window, 1, 1, "%99[^\n]", msg);
    noecho();

    return msg;
}

void send_msg(char *usrname, char *msg) {
    mvprintw(msg_y, 1, "%s: %s", usrname, msg);
    refresh();
    wrefresh(window);
    msg_y++;
}

// }}}

void send_packet(ENetPeer *peer, char *data) {
    ENetPacket *packet =
        enet_packet_create(data, strlen(data) + 1, ENET_PACKET_FLAG_RELIABLE);
    enet_peer_send(peer, 0, packet);
}

void parse_data(char *data) {
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
            send_msg(clients[id]->usr_name, msg);
        }
        break;
    case PACKET_JOIN:
        if (id != client_id) {
            char new_usr[80];
            sscanf(data, "%*d|%*d|%s", new_usr);

            client_data *new_client = malloc(sizeof(client_data));
            new_client->id = id;
            new_client->usr_name = strdup(new_usr);;
            clients[id] = new_client;
        }
        break;
    case PACKET_RECEIVE:
        client_id = id;
        break;
    default:
        break;
    }
}

pthread_t thread;

void *msg_loop(void *arg) {
    ENetHost *client = (ENetHost *)arg;
    while (1) {
        ENetEvent event;
        while (enet_host_service(client, &event, 0) > 0) {
            if (event.type == ENET_EVENT_TYPE_RECEIVE) {
                parse_data((char *)event.packet->data);
                enet_packet_destroy(event.packet);
            }
        }
    }
}

int main(void) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        clients[i] = NULL;
    }
    printf("Please enter your username below (no spaces!):\n");
    scanf("%s", usrname);

    // {{{ ENet Init

    if (enet_initialize()) {
        ERROR(stderr, "Failed to init ENet\n");
        return -1;
    }
    atexit(enet_deinitialize);

    ENetHost *client = enet_host_create(NULL, 1, 1, 0, 0);
    if (!client) {
        ERROR(stderr, "Failed to create client host\n");
        return -1;
    }

    ENetAddress address;
    enet_address_set_host(&address, "127.0.0.1");
    address.port = 7777;

    ENetEvent event;

    ENetPeer *peer = enet_host_connect(client, &address, 1, 0);
    if (!peer) {
        ERROR(stderr, "No peers available for connection\n");
        return -1;
    }

    if (enet_host_service(client, &event, 5000) > 0 &&
        event.type == ENET_EVENT_TYPE_CONNECT) {
        printf("Connection successful\n");
    } else {
        enet_peer_reset(peer);
        printf("Connection failed\n");
        return 0;
    }

    // }}}

    pthread_create(&thread, NULL, msg_loop, (void *)client);

    char usr_data[80] = "2|";
    strcat(usr_data, usrname);
    send_packet(peer, usr_data);

    screen_init();
    while (1) {
        char *msg = check_msg_input();
        if (strcmp(msg, "/exit") == 0) {
            free(msg);
            break;
        }
        send_msg(usrname, msg);

        char msg_data[80] = "1|";
        snprintf(msg_data, sizeof(msg_data), "1|%s", msg);
        send_packet(peer, msg_data);

        free(msg);
    }
    screen_close();

    pthread_join(thread, NULL);

    // {{{ ENet Close

    enet_peer_disconnect(peer, 0);

    while (enet_host_service(client, &event, 3000) > 0) {
        if (event.type == ENET_EVENT_TYPE_RECEIVE) {
            enet_packet_destroy(event.packet);
        } else if (event.type == ENET_EVENT_TYPE_DISCONNECT) {
            printf("Disconnection successful\n");
        }
    }

    // }}}
}
