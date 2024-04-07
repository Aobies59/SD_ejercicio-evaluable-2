#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include "claves.h"
#include <ctype.h>

int correct_operation(char* operation) {
    // CHECK IF OPERATION IS CORRECT
    if (strcmp(operation, "init") == 0) {
        return 1;
    } else if (strcmp(operation, "set") == 0) {
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

int handle_get() {
    int key;
    char value1[256];
    int value2;
    double value3;

    printf("Input key: ");
    scanf("%d", &key);
    int get_value_return_value = get_value(key, value1, &value2, &value3);

    if (get_value_return_value < 0) {
        return -1;
    } else if (get_value_return_value == 1) {
        return 0;
    } else {
        printf("Key: %d\nValue1: %s\nvalue2: %d\nvalue3: %.3lf\n", key, value1, value2, value3);
    }
    return 0;
}

int handle_set() {
    int key;
    char value1[256];
    int value2;
    double value3;

    // GET KEY
    printf("Input key: ");
    scanf("%d", &key);

    bool contains_comma;
    do {
        printf("Input value1: ");
        scanf("%s", value1);
        contains_comma = strchr(value1, ',') != NULL;
        if (contains_comma) {
            fprintf(stderr, "Error: Value1 contains a comma. Please enter a valid value1.\n");
        }
    } while(contains_comma);

    // GET N_VALUE2
    printf("Input value2: ");
    scanf("%d", &value2);

    // GET V_VALUE2
    printf("Input value3: ");
    scanf("%lf", &value3);

    // EXECUTE OPERATION
    if (set_value(key, value1, value2, value3) < 0) {
        return -1;
    }
    return 0;
}

int handle_get() {
    int key;
    char value1[256];
    int value2;
    double value3;

    // GET KEY
    printf("Input key: ");
    scanf("%d", &key);

    // EXECUTE OPERATION
    if (get_value(key, value1, &value2, &value3) < 0) {
        return -1;
    }
    printf("Key: %d\nValue1: %s\nvalue2: %d\nvalue3: %.3lf\n", key, value1, value2, value3);
    return 0;
}

int handle_modify() {
    int key;
    char value1[256];
    int value2;
    double value3;

    // GET KEY
    printf("Input key: ");
    scanf("%d", &key);

    // GET VALUE1
    printf("Input value1: ");
    scanf("%s", value1);

    // GET N_VALUE2
    printf("Input value2: ");
    scanf("%d", &value2);

    // GET V_VALUE2
    printf("Input value3: ");
    scanf("%lf", &value3);

    // EXECUTE OPERATION
    if (modify_value(key, value1, value2, value3) < 0) {
        return -1;
    }
    return 0;
}

int handle_init() {
    if (init() < 0) {
        return -1;
    }
    printf("Tuple system has been reset\n");
    return 0;
}

void handle_sigint(int sig) {
    exit(close_server());
}

int main (int argc, char *argv[]) {
    if (argc != 1) {
        if (strcmp(argv[1], "get") == 0) {
            if (argc != 3) {
                printf("Usage: ./client get <key>\n");
                exit(1);
            }
            if (isdigit(*argv[2]) == 0) {
                printf("Usage: ./client get <key>\n");
                exit(1);
            }
            if (create_socket() < 0) {
                exit(1);
            };
            int key = atoi(argv[2]);
            char value1[256];
            int value2;
            double value3;
            if (get_value(key, value1, &value2, &value3) < 0) {
                exit(1);
            }
            printf("Tuple %d: <%s-%d-%lf>\n", key, value1, value2, value3);
            exit(0);
        }
    }
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