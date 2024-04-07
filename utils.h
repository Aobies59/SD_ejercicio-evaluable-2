#ifndef UTILS_H
#define UTILS_H
#define BUFSIZE sizeof(int) + 256 + sizeof(double) + 32*sizeof(double)
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

int socket_send(int socket, const char value1[256], const int *value2, const double *value3) {
    char buffer[BUFSIZE];
    snprintf(buffer, BUFSIZE, "%d, %lf, %s", *value2, *value3, value1);
    char answer[10];
    do {
        if (send(socket, buffer, BUFSIZE, 0) < 0) {
            return -1;
        }
        if (recv(socket, answer, sizeof(answer), 0) < 0) {
            return -1;
        }
    } while (strcmp(answer, "noerror") != 0);
    return 0;
}

int socket_recv(int socket, char value1[256], int *value2, double *value3) {
    char buffer[BUFSIZE];
    int items_read = 0;
    do {
        int bytes_received = recv(socket, buffer, BUFSIZE, 0);
        if (bytes_received <= 0) {
            fprintf(stderr, "Error: sending side closed connection\n");
            return -1;
        }

        buffer[bytes_received] = '\0';
        items_read = sscanf(buffer, "%d, %lf, %s", value2, value3, value1);
        if (items_read != 3) {
            send(socket, "error", sizeof("error"), 0);
        } else {
            send(socket, "noerror", sizeof("noerror"), 0);
        }
    } while (items_read != 3);
    return 0;
}


#endif