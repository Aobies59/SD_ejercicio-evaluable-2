#ifndef UTILS_H
#define UTILS_H
#define BUFSIZE sizeof(int) + 256 + sizeof(double) + 32 * sizeof(double) + 32 // 32 is a margin for things like commas, etc
#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

struct tuple {
    int key;
    char value1[256];
    int N_Value2;
    double V_Value2[32];
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

int socket_send(int socket, const char value1[256], const int *N_Value2, const double V_Value2[32]) {
    char buffer[BUFSIZE];
    snprintf(buffer, BUFSIZE - 32 * sizeof(double) - 100, "%d, %s", *N_Value2, value1);
    for (int i = 0; i < *N_Value2; i++) {
        char double_str[32];  // allocate a much bigger size than needed to avoid warnings, we will use strncat later so it doesn't matter
        snprintf(double_str, sizeof(double_str), ",%lf", V_Value2[i]);
        strncat(buffer, double_str, strlen(double_str));  // write only the actual size of double_str
    }
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

int socket_recv(int socket, char value1[256], int *N_Value2, double *V_Value2) {
    char buffer[BUFSIZE];
    int items_read = 0;
    do {
        int bytes_received = recv(socket, buffer, BUFSIZE, 0);
        if (bytes_received <= 0) {
            fprintf(stderr, "Error: sending side closed connection\n");
            return -1;
        }

        buffer[bytes_received] = '\0';
        char str_value[BUFSIZE];
        items_read = sscanf(buffer, "%d, %s", N_Value2, str_value);
        if (items_read != 2) {
            send(socket, "error", sizeof("error"), 0);
        } else {
            strcpy(value1, strtok(str_value, ","));
            for (int i = 0; i < *N_Value2; i++) {
                char *token = strtok(NULL, ",");
                if (token == NULL) {
                    send(socket, "error", sizeof("error"), 0);
                    items_read = 0;
                    break;
                }
                V_Value2[i] = atof(token);
            }
            send(socket, "noerror", sizeof("noerror"), 0);
        }
    } while (items_read != 2);
    return 0;
}


#endif