#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include "claves.h"

int correct_operation(char* operation) {
    if (strcmp(operation, "set") == 0) {
        return 1;
    } else if (strcmp(operation, "get") == 0) {
        return 1;
    }else if (strcmp(operation, "delete") == 0) {
        return 1;
    } else if (strcmp(operation, "modify") == 0) {
        return 1;
    } else if (strcmp(operation, "exist") == 0) {
        return 1;
    } else if (strcmp(operation, "exit") == 0) {
        return 1;
    } else if (strcmp(operation, "init") == 0) {
        return 1;
    }

    return 0;
}

void exit_with_error(char* operation) {
    char error_string[100];
    sprintf(error_string, "Error with operation: %s", operation);
    close_server();
    exit(-1);
}

int handle_init() {
    if (init() < 0) {
        fprintf(stderr, "Error: error restarting tuple functionality\n");
        return -1;
    }
    printf("Tuple functionality restarted\n");
    return 0;
}

int handle_get() {
    int key;
    char value1[256];
    int N_value2;
    double V_value2[32];

    printf("Input key: ");
    scanf("%d", &key);

    int get_value_return_value = get_value(key, value1, &N_value2, V_value2);

    if (get_value_return_value < 0) {
        return -1;
    } else if (get_value_return_value == 1) {
        return 0;
    }
    printf("Key: %d\nValue1: %s\nN_value2: %d\nV_value2: ", key, value1, N_value2);
    printf("%.2f", V_value2[0]);
    for (int i = 1; i < N_value2; i++) {
        printf(", %.2f", V_value2[i]);
    }
    printf("\n");
    return 0;
}

int handle_set() {
    int key;
    char value1[256];
    int N_Value2;
    double V_Value2[32];

    printf("Input key: ");
    scanf("%d", &key);


    bool contains_comma;
    do {
        printf("Input value1: ");
        scanf("%s", value1);
        contains_comma = strchr(value1, ',') != NULL;
        if (contains_comma) {
            printf("Error: Value1 contains a comma. Please enter a valid value1.\n");
        }
    } while(contains_comma);

    do 
    {
    printf("Input N_value2: ");
    scanf("%d", &N_Value2);
    if (N_Value2 < 1 || N_Value2 > 32) {
        printf("Error: N_value2 must be between 1 and 32. Please enter a valid value.\n");
    }
    } while(N_Value2 < 1 || N_Value2 > 32);

    printf("Input V_Value2 of size %d:\n", N_Value2);
    for (int i = 0; i < N_Value2; i++) {
        printf("V_Value2[%d]: ", i);
        scanf("%lf", &V_Value2[i]);
    }
    if (set_value(key, value1, N_Value2, V_Value2) < 0) {
        return -1;
    }
    return 0;
}

int handle_delete() {
    int key;
    printf("Input key: ");
    scanf("%d", &key);
    if (delete_key(key) < 0) {
        return -1;
    }
    return 0;
}

int handle_exist() {
    int key;
    printf("Input key: ");
    scanf("%d", &key);
    int key_exists = exist(key);
    if (key_exists < 0) {
        return -1;
    } else if (key_exists == 0) {
        printf("Key does not exist.\n");
        return 0;
    }
    printf("Key exists.\n");
    return 0;
}

int handle_modify() {
    int key;
    char value1[256];
    int N_value2;
    double V_value2[32];

    printf("Input key: ");
    scanf("%d", &key);

    printf("Input value1: ");
    scanf("%s", value1);

    printf("Input N_value2: ");
    scanf("%d", &N_value2);

    printf("Input V_value2:\n");
    for (int i = 0; i < N_value2; i++) {
        printf("V_value2[%d]: ", i);
        scanf("%lf", &V_value2[i]);
    }
    if (modify_value(key, value1, N_value2, V_value2) < 0) {
        return -1;
    }
    return 0;
}

void handle_sigint(int sig) {
    exit(close_server());
}

int main () {
    signal(SIGINT, handle_sigint);

    printf("Welcome to the tuple management system.\n");
    while (1) {
        if (create_socket() < 0) {
            exit(-1);
        }
        char operation[10];
        do {
            printf("\nPossible operations are: set, get, delete, modify, exist and exit.\n");
            printf("Input operation: ");
            scanf("%s", operation);
        } while (!correct_operation(operation));
        if (strcmp(operation, "exit") == 0) {
            break;
        } else if (strcmp(operation, "get") == 0) {
            if (handle_get() < 0) {
                exit_with_error(operation);
            };
        } else if (strcmp(operation, "set") == 0) {
            if (handle_set() < 0) {
                exit_with_error(operation);
            };
        } else if (strcmp(operation, "delete") == 0) {
            if (handle_delete() < 0) {
                exit_with_error(operation);
            };
        } else if (strcmp(operation, "exist") == 0) {
            if (handle_exist() < 0) {
                exit_with_error(operation);
            };
        } else if (strcmp(operation, "modify") == 0) {
            if (handle_modify() < 0) {
                exit_with_error(operation);
            };
        } else if (strcmp(operation, "init") == 0) {
            if (handle_init() < 0) {
                exit_with_error(operation);
            }
        }
    }
    exit(close_server());
}