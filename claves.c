#include "utils.h"
#include <arpa/inet.h>
#include <netinet/in.h>

int client_socket;

int init () {
    struct sockaddr_in server;
    bzero((char *)&server, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr("127.0.0.0");
    server.sin_port = htons(PORT);
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
    }
    return 0;
}

static int connect_to_server() {
    if (connect(client_socket, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("connect");
        return -1;
    }
    return 0;
}

int set_value(int key, char *value1, int value2, double value3) {
    char answer[10];

    // send petition
    send(client_socket, "set", sizeof("set"), 0);
    sleep(0.1);
    send(client_socket, &key, sizeof(int), 0);
    socket_send(client_socket, value1, &value2, &value3);

    recv(client_socket, answer, 10, 0);
    if (strcmp(answer, "error") == 0) {
        fprintf(stderr, "Error setting value\n");
        return -1;
    } else if (strcmp(answer, "exist") == 0) {
        printf("Tuple with key %d already exists, use modify instead\n", key);
        return 0;
    }
    printf("Value set correctly\n");

    return 0;
}

int get_value(int key, char *value1, int *value2, double *value3) {
    send(client_socket, "get", sizeof("get"), 0);
    char answer[10];
    sleep(0.1);
    send(client_socket, &key, sizeof(int), 0);
    sleep(0.1);
    recv(client_socket, answer, sizeof(answer), 0);
    if (strcmp("error", answer) == 0) {
        fprintf(stderr, "Error: key doesn't exist or error getting value\n");
        return -1;
    }

    // receive tuple item from socket
    return socket_recv(client_socket, value1, value2, value3);
}

int modify_value(int key, char *value1, int value2, double value3) {
    send(client_socket, "modify", sizeof("modify"), 0);
    send(client_socket, &key, sizeof(int), 0);
    char answer[10];
    recv(client_socket, answer, sizeof(answer), 0);
    if (strcmp(answer, "noexist") == 0) {
        printf("Tuple with key %d does not exist, use set instead\n", key);
        return 0;
    }

    // send new tuple items to server
    socket_send(client_socket, value1, &value2, &value3);
    recv(client_socket, answer, sizeof(answer), 0);
    if (strcmp(answer, "error") == 0) {
        fprintf(stderr, "Error modifying value\n");
        return -1;
    }
    printf("Value modified correctly\n");

    return 0;
}

int delete_key(int key) {
    send(client_socket, "delete", sizeof("delete"), 0);
    send(client_socket, &key, sizeof(int), 0);
    char answer[10];
    recv(client_socket, answer, sizeof(answer), 0);
    if (strcmp("error", answer) == 0){
        fprintf(stderr, "Error: error deleting key %d\n", key);
        return -1;
    } else if (strcmp("noexist", answer) == 0) {
        printf("Key %d does not exist\n", key);
        return 0;
    }
    printf("Key %d deleted correctly\n", key);
    return 0;
}

int exist(int key) {
    send(client_socket, "exist", sizeof("exist"), 0);
    sleep(0.1);
    send(client_socket, &key, sizeof(int), 0);
    char answer[10];
    recv(client_socket, answer, sizeof(answer), 0);
    if (strcmp(answer, "exist") == 0) {
        return 1;
    } else if (strcmp(answer, "noexist") == 0) {
        return 0;
    }
    fprintf(stderr, "Error: error checking if key exists\n");
    return -1;
}

int close_server() {
    send(client_socket, "exit", sizeof("exit"), 0);
    close(client_socket);

    printf("Bye!\n");
    return 0;
}
;