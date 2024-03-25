#include <mqueue.h>
#include "utils.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>

mqd_t client_queue;
mqd_t server_queue;
struct petition current_petition;
char queue_name[256];

static int send_tuple(struct tuple tuple_to_send, int socket) {
    return 0;
}

static int receive_tuple(struct tuple tuple_to_send, int socket) {
    return 0;
}

int init () {
    struct mq_attr attr;
    attr.mq_maxmsg = 1;
    attr.mq_msgsize = sizeof(struct answer);

    sprintf(queue_name, "/tuples_client_queue_%d", getpid());
    strcpy(current_petition.queue_name, queue_name);
    mq_unlink(queue_name);  // destroy queue if it already exists
    client_queue = mq_open(queue_name, O_RDONLY | O_CREAT, 0666, &attr);
    if (client_queue == -1) {
        perror("mq_open");
        return -1;
    }

    attr.mq_msgsize = sizeof(struct petition);
    attr.mq_maxmsg = 10;

    server_queue = mq_open("/tuples_server_queue", O_WRONLY, 0666, &attr);
    if (server_queue == -1) {
        printf("Error opening server queue, is server initialized?\n");
        return -1;
    }
    return 0;
}

int set_value(int key, char *value1, int value2, double value3) {
    // create petition
    strcpy(current_petition.operation, "set");
    struct tuple temp_tuple;
    temp_tuple.key = key;
    strcpy(temp_tuple.value1, value1);
    temp_tuple.value2 = value2;
    temp_tuple.value3 = value3;
    current_petition.operated_tuple = temp_tuple;

    // send petition to server
    if (mq_send(server_queue, (char *)&current_petition, sizeof(struct petition), 0) == -1) {
        perror("mq_send");
        return -1;
    }

    // receive answer from server
    struct answer current_answer;
    if (mq_receive(client_queue, (char *)&current_answer, sizeof(current_answer), 0) == -1) {
        perror("mq_receive");
        return -1;
    }
    if (strncmp(current_answer.error_code, "error", sizeof("error")) == 0) {
        return -2;
    }
    return 0;
}

int get_value(int key, char *value1, int *N_value2, double *V_value2) {
    // create petition
    strcpy(current_petition.operation, "get");
    struct tuple temp_tuple;
    temp_tuple.key = key;
    current_petition.operated_tuple = temp_tuple;

    // send petition to server
    if (mq_send(server_queue, (char *)&current_petition, sizeof(struct petition), 0) == -1) {
        perror("mq_send");
        return -1;
    }

    struct answer current_answer;

    // receive answer from server
    if (mq_receive(client_queue, (char *)&current_answer, sizeof(current_answer), 0) == -1) {
        perror("mq_receive");
        return -1;
    }
    if (strncmp(current_answer.error_code, "error", sizeof("error")) == 0) {
        return -2;
    }
    strcpy(value1, current_answer.return_tuple.value1);
    *N_value2 = current_answer.return_tuple.N_value2;
    for (int i = 0; i < current_answer.return_tuple.N_value2; i++) {
        V_value2[i] = current_answer.return_tuple.V_value2[i];
    }

    return 0;
}

int modify_value(int key, char *value1, int N_value2, double *V_value2) {
    // create petition
    strcpy(current_petition.operation, "modify");
    struct tuple temp_tuple;
    temp_tuple.key = key;
    strcpy(temp_tuple.value1, value1);
    temp_tuple.N_value2 = N_value2;
    for (int i = 0; i < N_value2; i++) {
        temp_tuple.V_value2[i] = V_value2[i];
    }
    current_petition.operated_tuple = temp_tuple;

    // send petition to server
    if (mq_send(server_queue, (char *)&current_petition, sizeof(struct petition), 0) == -1) {
        perror("mq_send");
        return -1;
    }

    // receive answer from server
    struct answer current_answer;
    if (mq_receive(client_queue, (char *)&current_answer, sizeof(current_answer), 0) == -1) {
        perror("mq_receive");
        return -1;
    }
    if (strncmp(current_answer.error_code, "error", sizeof("error")) == 0) {
        return -2;
    }

    return 0;
}

int delete_key(int key) {
    // create petition
    strcpy(current_petition.operation, "delete");
    struct tuple temp_tuple;
    temp_tuple.key = key;
    current_petition.operated_tuple = temp_tuple;

    // send petition to server
    if (mq_send(server_queue, (char *)&current_petition, sizeof(struct petition), 0) == -1) {
        perror("mq_send");
        return -1;
    }

    // receive answer from server
    struct answer current_answer;
    if (mq_receive(client_queue, (char *)&current_answer, sizeof(current_answer), 0) == -1) {
        perror("mq_receive");
        return -1;
    }
    if (strncmp(current_answer.error_code, "error", sizeof("error")) == 0) {
        return -2;
    }

    return 0;
}

int exist(int key) {
    // create petition
    strcpy(current_petition.operation, "exist");
    struct tuple temp_tuple;
    temp_tuple.key = key;
    current_petition.operated_tuple = temp_tuple;

    // send petition to server
    if (mq_send(server_queue, (char *)&current_petition, sizeof(current_petition), 0) == -1) {
        perror("mq_send");
        return -1;
    }

    // receive answer from server
    struct answer current_answer;
    if (mq_receive(client_queue, (char *)&current_answer, sizeof(current_answer), 0) == -1) {
        perror("mq_receive");
        return -1;
    }
    if (strncmp(current_answer.error_code, "noexist", sizeof("noexist")) == 0) {
        return 0;
    } else if (strncmp(current_answer.error_code, "error", 5) == 0) {
        return -2;
    }
    // getting here means key exists
    return 1;
}

int close_server() {
    printf("\n");
    // create petition
    strcpy(current_petition.operation, "exit");

    // send petition to server
    if (mq_send(server_queue, (char *)&current_petition, sizeof(struct petition), 0) == -1) {
        perror("mq_send");
        return -1;
    }

    // close queues and delete client queue
    mq_close(server_queue);
    mq_close(client_queue);
    mq_unlink(queue_name);

    printf("Bye!\n");
    return 0;
}
