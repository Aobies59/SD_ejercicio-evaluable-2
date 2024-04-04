#ifndef UTILS_H
#define UTILS_H
#define PORT 5000
#define BUFSIZE 600
#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

struct tuple {
    int key;
    char value1[256];
    int value2;
    double value3;
};

struct petition {
    char operation[10];
    struct tuple operated_tuple;
    char queue_name[256];
};

struct answer {
    char error_code[50];
    struct tuple return_tuple;
};

int socket_send(int socket, const char *value1, const int *value2, const double *value3) {
    char buffer[BUFSIZE];
    snprintf(buffer, BUFSIZE, "%d, %lf, %s", *value2, *value3, value1);
    printf("Sending buffer:  %s...\n", buffer);
    sleep(0.1);
    if (send(socket, buffer, BUFSIZE, 0) < 0) {
        return -1;
    }
    return 0;
}

int socket_recv(int socket, char value1[256], int *value2, double *value3) {
    char buffer[BUFSIZE];
    sleep(0.1);
    int bytes_received = recv(socket, buffer, BUFSIZE, 0);
    if (bytes_received <= 0) {
        printf("Error: sending side closed connection\n");
        return -1;
    }
    printf("Received buffer: %s\n", buffer);

    buffer[bytes_received] = '\0';
    sscanf(buffer, "%d, %lf, %s", value2, value3, value1);
    return 0;
}


#endif