#include <pthread.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdatomic.h>
#include "utils.h"

pthread_mutex_t file_lock;
pthread_mutex_t socket_lock;
pthread_cond_t socket_cond;
int socket_copied = false;
char* tuples_filename = "tuples.csv";
atomic_int thread_return_value;

static int receive_key(int socket, int *key) {
    int received_bytes = 0;
    do {
        received_bytes = recv(socket, key, sizeof(int), 0);
        if (received_bytes != sizeof(int)) {
            printf("Received %d bytes\n", received_bytes);\
            int error = -1;
            if (send(socket, &error, 1, 0) < 0) {
                perror("send");
                return -1;
            }
        }
    } while (received_bytes != sizeof(int));

    int success = 0;
    if (send(socket, &success, 1, 0) < 0) {
        perror("send");
        return -1;
    }

    *key = (int)ntohl(*key);
    return 0;
}

int init() {
    pthread_mutex_lock(&file_lock);
    FILE *file = fopen(tuples_filename, "w");
    if (file == NULL) {
        fclose(file);
        pthread_mutex_unlock(&file_lock);
        return -1;
    }
    fclose(file);
    pthread_mutex_unlock(&file_lock);
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
            fprintf(stderr, "Error: error in reading file\n");
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
    // check if N_value2 is between 1 and 32
    if (given_tuple.N_Value2 < 1 || given_tuple.N_Value2 > 32) {
        return -1;
    }

    // check if key exists
    if (exists(given_tuple.key) == 1) {
        fprintf(stderr, "Error: key already exists\n");
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
    fprintf(tuples_file, "%d,%d,%s", given_tuple.key, given_tuple.N_Value2, given_tuple.value1);
    for (int i = 0; i < given_tuple.N_Value2; i++) {
        fprintf(tuples_file, ",%lf", given_tuple.V_Value2[i]);
    }
    fprintf(tuples_file, "\n");
    fclose(tuples_file);
    pthread_mutex_unlock(&file_lock);
    return 0;
}

int get_value(int key, char *value1, int *N_Value2, double *V_Value2) {
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
        sscanf(line, "%d,%d, %s", &temp_key, N_Value2, str_value);
        if (temp_key == key) {
            strcpy(value1, strtok(str_value, ","));
            for (int i = 0; i < *N_Value2; i++) {
                V_Value2[i] = atof(strtok(NULL, ","));
            }
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

int modify_value(int key, char *value1, int N_Value2, double *V_Value2) {
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
            fprintf(temp_tuples_file, "%d,%d,%s", key, N_Value2, value1);
            for (int i = 0; i < N_Value2; i++) {
                fprintf(temp_tuples_file, ",%lf", V_Value2[i]);
            }
            fprintf(temp_tuples_file, "\n");
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

void petition_handler(void *socket) {
    printf("Received a petition\n");
    if (socket_copied == true) {
        return;
    }
    pthread_mutex_lock(&socket_lock);
    // copy socket into local variable
    int *client_socket_pointer = (int *)socket;
    int client_socket = *client_socket_pointer;
    socket_copied = true;
    pthread_cond_signal(&socket_cond);
    pthread_mutex_unlock(&socket_lock);

    // receive operation from socket
    char operation[10];
    recv(client_socket, operation, sizeof(operation), 0);
    printf("Handling operation %s...\n", operation);

    if (strcmp(operation, "exit") == 0) {
        printf("Exit operation received from client, closing server\n");
        atomic_store(&thread_return_value, 2);
        return;
    } else if (strcmp(operation, "set") == 0) {
        // receive tuple key from socket
        int key;
        printf("Receiving key\n");
        if (receive_key(client_socket, &key) < 0) {
            send(client_socket, "error", sizeof("error"), 0);
            atomic_store(&thread_return_value, -1);
            return;
        };
        printf("Received key %d\n", key);

        // receive tuple values from socket
        struct tuple temp_tuple;
        temp_tuple.key = key;
        printf("Receiving tuple items\n");
        socket_recv(client_socket, temp_tuple.value1, &temp_tuple.N_Value2, temp_tuple.V_Value2);
        printf("Received tuple items\n");

        if (exists(key) == 1) {
            atomic_store(&thread_return_value, -1);
            send(client_socket, "exist", sizeof("exist"), 0);
        } else if (set_value(temp_tuple) < 0) {
            atomic_store(&thread_return_value, -2);
            send(client_socket, "error", sizeof("error"), 0);
        } else {
            atomic_store(&thread_return_value, 0);
            send(client_socket, "noerror", sizeof("noerror"), 0);
            printf("Value set correctly\n");
        }
        return;
    } else if (strcmp(operation, "get") == 0) {
        // receive key from socket
        int key;
        if (receive_key(client_socket, &key) < 0) {
            send(client_socket, "error", sizeof("error"), 0);
            atomic_store(&thread_return_value, -1);
            return;
        };
        char value1[256];
        int N_Value2;
        double V_Value2[32];

        if (exists(key) == 0) {
            atomic_store(&thread_return_value, -1);
            send(client_socket, "noexist", sizeof("noexist"), 0);
            return;
        }

        // if error getting value, raise error
        if (get_value(key, value1, &N_Value2, V_Value2) < 0) {
            atomic_store(&thread_return_value, 2);
            send(client_socket, "error", sizeof("error"), 0);
            return;
        }
        send(client_socket, "noerror", sizeof("noerror"), 0);

        // send tuple item to socket
        socket_send(client_socket, value1, &N_Value2, V_Value2);

        atomic_store(&thread_return_value, 0);
        printf("Value retrieved correctly\n");
    } else if (strcmp(operation, "delete") == 0) {
        int key;
        if (receive_key(client_socket, &key) < 0) {
            send(client_socket, "error", sizeof("error"), 0);
            atomic_store(&thread_return_value, -1);
            return;
        };
        if (exists(key) == 0) {
            atomic_store(&thread_return_value, -1);
            send(client_socket, "noexist", sizeof("noexist"), 0);
            return;
        }
        if (delete_key(key) < 0) {
            printf("delete_key < 0\n");
            atomic_store(&thread_return_value, 2);
            send(client_socket, "error", sizeof("error"), 0);
            return;
        }
        send(client_socket, "noerror", sizeof("noerror"), 0);
        atomic_store(&thread_return_value, 0);
        return;
    } else if (strcmp(operation, "modify") == 0) {
        // receive key and check if it exists
        int key;
        if (receive_key(client_socket, &key) < 0) {
            send(client_socket, "error", sizeof("error"), 0);
            atomic_store(&thread_return_value, -1);
            return;
        };
        if (exists(key) == 0) {
            atomic_store(&thread_return_value, -1);
            send(client_socket, "noexist", sizeof("noexist"), 0);
            atomic_store(&thread_return_value, 0);
            return;
        }
        send(client_socket, "noerror", sizeof("noerror"), 0);
        
        // receive new tuple items from client
        char value1[256];
        int N_Value2;
        double V_Value2[32];
        socket_recv(client_socket, value1, &N_Value2, V_Value2);

        // attempt to modify tuple
        if (modify_value(key, value1, N_Value2, V_Value2) < 0) {
            printf("modify_value < 0\n");
            atomic_store(&thread_return_value, 2);
            send(client_socket, "error", sizeof("error"), 0);
        } else{
            atomic_store(&thread_return_value, 0);
            send(client_socket, "noerror", sizeof("noerror"), 0);
        }

        return;
        
    } else if (strcmp(operation, "exist") == 0) {
        int key;
        if (receive_key(client_socket, &key) < 0) {
            send(client_socket, "error", sizeof("error"), 0);
            atomic_store(&thread_return_value, -1);
            return;
        };
        printf("Received key %d\n", key);   
        int key_exists = exists(key);
        atomic_store(&thread_return_value, 0);
        if (key_exists == 1) {
            send(client_socket, "exist", sizeof("exist"), 0);
        } else if (key_exists == 0) {
            send(client_socket, "noexist", sizeof("noexist"), 0);
        } else if (key_exists == -1) {
            perror("error checking if key exists");
            atomic_store(&thread_return_value, 2);
            send(client_socket, "error", sizeof("error"), 0);
        }
        return;
    } else if (strcmp(operation, "init") == 0){
        if (init() < 0){
            fprintf(stderr, "Error: error initiating tuples functionality\n");
            atomic_store(&thread_return_value, 2);  // close the server
        }
    } else {
        fprintf(stderr, "Error: received wrong operation\n");
        atomic_store(&thread_return_value, -1);
    }
}

int main () {
    // create the tuples file if it does not exist
    FILE* tuples_file = fopen(tuples_filename, "r");
    if (tuples_file) {
        fclose(tuples_file);
    } else {
        tuples_file = fopen(tuples_filename, "w");
        fclose(tuples_file);
    }
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("socket");
        exit(1);
    }

    int val = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&val, sizeof(int)));

    char *ip = getenv("IP_TUPLAS");
    if (ip == NULL) {
        fprintf(stderr, "Variable de entorno IP_TUPLAS no definida\n");
        exit(1);
    }
    int port = atoi(getenv("PORT_TUPLAS"));
    if (port == 0) {
        fprintf(stderr, "Variable de entorno PORT_TUPLAS no definida\n");
        exit(1);
    }
    struct sockaddr_in server;
    bzero((char *)&server, sizeof(struct sockaddr_in));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(ip);
    server.sin_port = htons(port);
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
            exit(-1);
        }

        int client_socket = accept(server_socket, NULL, NULL);
        if (client_socket < 0) {
            perror("accept");
            exit(-1);
        }

        pthread_mutex_lock(&socket_lock);
        if (pthread_create(&thread, &threads_attr, (void *) petition_handler, (void *) &client_socket) < 0) {
            perror("pthread_create");
            exit(-1);
        }
        
        socket_copied = false;
        while (socket_copied == false)
            pthread_cond_wait(&socket_cond, &socket_lock);

        pthread_mutex_unlock(&socket_lock);

        if (pthread_join(thread, NULL) < 0) {
            perror("pthread_join");
            exit(-1);
        }
        
        int return_value = atomic_load(&thread_return_value);
        if (return_value == 2) {
            break;
        } else if (return_value < 0) {
            fprintf(stderr, "Error in last operation\n");
        } else {
            printf("Operation completed successfully\n");
        }
        close(client_socket);
    }
    
    close(server_socket);

    // destroy mutex and attributes
    pthread_mutex_destroy(&socket_lock);
    pthread_attr_destroy(&threads_attr);

    exit(0);
}