#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdbool.h>
#include <stdatomic.h>
#include "utils.h"

pthread_mutex_t file_lock;
pthread_mutex_t socket_lock;
pthread_cond_t socket_cond;
int socket_copied = false;
char* tuples_filename = "tuples.csv";
atomic_int thread_return_value;

static int init() {
    FILE *file = fopen(tuples_filename, "w");
    if (file == NULL) {
        fclose(file);
        return -1;
    }
    fclose(file);
    return 0;
}

int exists(int key) {
    const long MAXLINE = 4096;  // big enough number that endofline will ocurr before end of buffer
    pthread_mutex_lock(&file_lock);
    FILE *tuples_file = fopen(tuples_filename, "r");
    if (tuples_file == NULL) {
        fclose(tuples_file);
        pthread_mutex_unlock(&file_lock);
        return -1;
    }
    char *line = malloc(MAXLINE * sizeof(char));    
    line = fgets(line, MAXLINE, tuples_file);
    int tuple_key;
    while (line != NULL) {
        int items = sscanf(line, "%d", &tuple_key);
        if (items != 1) {
            fclose(tuples_file);
            pthread_mutex_unlock(&file_lock);
            return 0;
        }
        if (tuple_key == key) {
            fclose(tuples_file);
            pthread_mutex_unlock(&file_lock);
            return 1;
        }
        line = fgets(line, MAXLINE, tuples_file);
    }
    free(line);
    fclose(tuples_file);
    pthread_mutex_unlock(&file_lock);
    return 0;
}

int set_value(struct tuple given_tuple) {
    // check if key exists
    if (exists(given_tuple.key) == 1) {
        return -1;
    }

    // write the new tuple at the end of the file
    pthread_mutex_lock(&file_lock);
    FILE *tuples_file = fopen(tuples_filename, "a");
    if (tuples_file == NULL) {
        pthread_mutex_unlock(&file_lock);
        fclose(tuples_file);
        return -1;
    }
    fprintf(tuples_file, "%d,%s,%d,%f\n", given_tuple.key, given_tuple.value1, given_tuple.value2, given_tuple.value3);
    fclose(tuples_file);
    pthread_mutex_unlock(&file_lock);
    return 0;
}

int get_value(int key, char *value1, int *value2, double *value3) {
    if (exists(key) == 0) {
        return -1;
    }
    const long MAXLINE = 4096;  // big enough number that endofline will occur before end of buffer
    pthread_mutex_lock(&file_lock);
    FILE *tuples_file = fopen(tuples_filename, "r");
    if (tuples_file == NULL) {
        perror("fopen");
        fclose(tuples_file);
        return -1;
    }
    char *line = malloc(MAXLINE * sizeof(char));    
    line = fgets(line, MAXLINE, tuples_file);
    while (line != NULL) {
        char str_value[MAXLINE];
        int temp_key;
        sscanf(line, "%d, %s", &temp_key, str_value);
        if (temp_key == key) {
            strcpy(value1, strtok(str_value, ","));
            *value2 = atoi(strtok(NULL, ","));
            *value3 = atoi(strtok(NULL, ","));
            free(line);
            fclose(tuples_file);
            pthread_mutex_unlock(&file_lock);
            return 0;
        }
        line = fgets(line, MAXLINE, tuples_file);
    }
    free(line);
    fclose(tuples_file);
    pthread_mutex_unlock(&file_lock);
    return -1;
}

int modify_value(int key, char *value1, int value2, double value3) {
    const long MAXLINE = 4096;  // big enough number that endofline will occur before end of buffer
    pthread_mutex_lock(&file_lock);
    FILE *tuples_file = fopen(tuples_filename, "r");
    FILE *temp_tuples_file = fopen("temp.csv", "w");
    if (tuples_file == NULL) {
        fclose(tuples_file);
        pthread_mutex_unlock(&file_lock);
        return -1;
    }
    char *line = malloc(MAXLINE * sizeof(char));    
    line = fgets(line, MAXLINE, tuples_file);
    int read_key;
    while (line != NULL) {
        sscanf(line, "%d", &read_key);
        if (read_key != key) {
            fputs(line, temp_tuples_file);
        } else {
            // write the new tuple to the temp file
            fprintf(temp_tuples_file, "%d,%s,%d,%f\n", key, value1, value2,value3);
        }
        line = fgets(line, MAXLINE, tuples_file);
    }
    free(line);
    fclose(tuples_file);
    fclose(temp_tuples_file);
    // remove original file and replace it with temp file
    remove(tuples_filename);
    rename("temp.csv", tuples_filename);
    pthread_mutex_unlock(&file_lock);
    return 0;
}

int delete_key(int key) {
    const long MAXLINE = 4096;  // big enough number that endofline will occur before end of buffer
    pthread_mutex_lock(&file_lock);
    FILE *tuples_file = fopen(tuples_filename, "r");
    FILE *temp_tuples_file = fopen("temp.csv", "w");
    if (tuples_file == NULL) {
        fclose(tuples_file);
        pthread_mutex_unlock(&file_lock);
        return -1;
    }
    char *line = malloc(MAXLINE * sizeof(char));    
    line = fgets(line, MAXLINE, tuples_file);
    int read_key;
    while (line != NULL) {
        sscanf(line, "%d", &read_key);
        if (read_key != key) {
            // write the read line to the temp file
            fputs(line, temp_tuples_file);
        }
        line = fgets(line, MAXLINE, tuples_file);
    }
    free(line);
    fclose(tuples_file);
    fclose(temp_tuples_file);
    // remove original file and replace it with temp file
    remove(tuples_filename);
    rename("temp.csv", tuples_filename);
    pthread_mutex_unlock(&file_lock);
    return 0;
}

void petition_handler(int socket) {
    if (socket_copied == true) {
        return;
    }
    pthread_mutex_lock(&socket_lock);
    // copy socket into local variable
    int client_socket = socket;
    socket_copied = true;
    pthread_cond_signal(&socket_cond);
    pthread_mutex_unlock(&socket_lock);

    // receive operation from socket
    char operation[10];
    recv(socket, operation, sizeof(operation), 0);
    printf("Received a petition: %s\n", operation);

    if (strcmp(operation, "exit") == 0) {
        atomic_store(&thread_return_value, 2);
        return;
    } else if (strcmp(operation, "set") == 0) {
        // receive tuple key from socket
        int key;
        recv(socket, &key, sizeof(int), 0);
        // if key doesn't exist, raise error
        if (exists(key) == 0) {
            atomic_store(&thread_return_value, -1);
            send(client_socket, "error", sizeof("error"), 0);
            return;
        }
        send(client_socket, "noerror", sizeof("noerror"), 0);

        // receive tuple values from socket
        char value1[256];
        recv(socket, value1, sizeof(value1), 0);
        int value2;
        recv(socket, &value2, sizeof(int), 0);
        double value3;
        recv(socket, &value3, sizeof(double), 0);
        
        struct tuple temp_tuple;
        temp_tuple.key = key;
        strcpy(temp_tuple.value1, value1);
        temp_tuple.value2 = value2;
        temp_tuple.value3 = value3;

        if (set_value(temp_tuple) < 0) {
            atomic_store(&thread_return_value, -1);
            send(client_socket, "error", sizeof("error"), 0);
            return;
        };
        atomic_store(&thread_return_value, 0);
        send(client_socket, "noerror", sizeof("noerror"), 0);
        printf("Value set correctly\n");
    } else if (strcmp(operation, "get") == 0) {
        // receive key from socket
        int key;
        recv(socket, &key, sizeof(int), 0);
        struct tuple temp_tuple;

        // if error getting value, raise error
        if (get_value(key, temp_tuple.value1, &temp_tuple.value2, &temp_tuple.value3) < 0) {
            atomic_store(&thread_return_value, -1);
            send(client_socket, "error", sizeof("error"), 0);
            return;
        }

        // send tuple item to socket
        send(client_socket, "noerror", sizeof("noerror"), 0);
        send(client_socket, temp_tuple.value1, sizeof(temp_tuple.value1), 0);
        send(client_socket, &temp_tuple.value2, sizeof(temp_tuple.value2), 0);
        send(client_socket, &temp_tuple.value3, sizeof(temp_tuple.value3), 0);

        atomic_store(&thread_return_value, 0);
        printf("Value retrieved correctly\n");
    } else if (strcmp(operation, "delete") == 0) {
        int key;
        recv(socket, &key, sizeof(int), 0);
        if (exists(key) == 0) {
            atomic_store(&thread_return_value, -1);
            send(client_socket, "error", sizeof("error"), 0);
            return;
        }
        if (delete_key(key) < 0) {
            atomic_store(&thread_return_value, -1);
            send(client_socket, "error", sizeof("error"), 0);
            return;
        }
        atomic_store(&thread_return_value, 0);
        send(client_socket, "noerror", sizeof("noerror"), 0);
    } else if (strcmp(operation, "modify") == 0) {
        struct tuple temp_tuple;
        recv(socket, &temp_tuple.key, sizeof(int), 0);
        if (exists(temp_tuple.key) == 0) {
            atomic_store(&thread_return_value, -1);
            send(client_socket, "error", sizeof("error"), 0);
            return;
        }
        recv(socket, temp_tuple.value1, sizeof(temp_tuple.value1), 0);
        recv(socket, &temp_tuple.value2, sizeof(int), 0);
        recv(socket, &temp_tuple.value3, sizeof(double), 0);
        if (modify_value(temp_tuple.key, temp_tuple.value1, temp_tuple.value2, temp_tuple.value3) < 0) {
            atomic_store(&thread_return_value, -1);
            send(client_socket, "error", sizeof("error"), 0);
            return;
        }
        atomic_store(&thread_return_value, 0);
        send(client_socket, "noerror", sizeof("noerror"), 0);
    } else if (strcmp(operation, "exist") == 0) {
        int key = recv(socket, &key, sizeof(key), 0);
        int key_exists = exists(key);
        atomic_store(&thread_return_value, 0);
        if (key_exists == 1) {
            send(client_socket, "exist", sizeof("exist"), 0);
        } else if (key_exists == 0) {
            send(client_socket, "noexist", sizeof("noexist"), 0);
        } else if (key_exists == -1) {
            perror("error checking if key exists");
            atomic_store(&thread_return_value, -1);
            send(client_socket, "error", sizeof("error"), 0);
        }
        return;
    }
}

int main () {
    init();
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("socket");
        exit(1);
    }

    struct sockaddr_in server;
    bzero((char *)&server, sizeof(struct sockaddr_in));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr("localhost");
    server.sin_port = htons(5000);
    if (bind(server_socket, (struct sockaddr *)&server, sizeof(struct sockaddr_in)) < 0) {
        perror("bind");
        exit(1);
    }

    pthread_mutex_init(&socket_lock, NULL);
    pthread_mutex_init(&file_lock, NULL);
    pthread_attr_t threads_attr;  // threads attributes
    pthread_attr_init(&threads_attr);
    pthread_attr_setdetachstate(&threads_attr, PTHREAD_CREATE_JOINABLE);  // dependent threads (to properly get thread_return_value)
    pthread_t thread;  // thread for handling petitions

    while (1) {
        printf("\nWaiting for a petition...\n");
        if (listen(server_socket, 5) < 0) {
        perror("listen");
        exit(1);
        }

        int client_socket = accept(server_socket, NULL, NULL);
        if (client_socket < 0) {
            perror("accept");
            exit(1);
        }

        pthread_mutex_lock(&socket_lock);
        if (pthread_create(&thread, &threads_attr, (void *) petition_handler, (void *) &client_socket) < 0) {
            perror("pthread_create");
            exit(1);
        }
        
        socket_copied = false;
        while (socket_copied == false)
            pthread_cond_wait(&socket_cond, &socket_lock);

        pthread_mutex_unlock(&socket_lock);

        if (pthread_join(thread, NULL) < 0) {
            perror("pthread_join");
            exit(1);
        }
        
        int return_value = atomic_load(&thread_return_value);
        if (return_value == 2) {
            printf("Exit operation received from client, terminating server\n");
            break;
        } else if (return_value < 0) {
            printf("Error in last operation\n");
        } else {
            printf("Operation completed successfully\n");
        }
        close(client_socket);
    }
    
    close(server_socket);

    // destroy mutex and attributes
    pthread_mutex_destroy(&socket_lock);
    pthread_attr_destroy(&threads_attr);
    remove(tuples_filename);

    exit(0);
}