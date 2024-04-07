#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <unistd.h>
#include <ctype.h>
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
        printf("Input value1 (string): ");
        scanf("%s", value1);
        contains_comma = strchr(value1, ',') != NULL;
        if (contains_comma) {
            printf("Error: Value1 contains a comma. Please enter a valid value1.\n");
        }
    } while(contains_comma);

    do 
    {
    printf("Input N_value2 (size of value2 vector): ");
    scanf("%d", &N_Value2);
    if (N_Value2 < 1 || N_Value2 > 32) {
        printf("Error: N_value2 must be between 1 and 32. Please enter a valid value.\n");
    }
    } while(N_Value2 < 1 || N_Value2 > 32);

    printf("Input vector V_Value2 of size %d:\n", N_Value2);
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

void handle_arguments(int argc, char *argv[]) {
    if (argc != 1) {
        if (strcmp(argv[1], "init") == 0) {
            if (create_socket() < 0) {
                exit(-1);
            };
            exit(handle_init());  // we can use handle_init since it doesn't ask for user input
        } else if (strcmp(argv[1], "get") == 0) {
            if (argc != 3) {
                printf("Usage: ./client get <key>\n");
                exit(-1);
            }
            if (isdigit(*argv[2]) == 0) {
                printf("Usage: ./client get <key>\n");
                exit(-1);
            }
            if (create_socket() < 0) {
                exit(-1);
            };
            int key = atoi(argv[2]);
            char value1[256];
            int N_Value2;
            double V_Value2[32];
            int get_value_return_value = get_value(key, value1, &N_Value2, V_Value2);
            if (get_value_return_value < 0) {
                exit(-1);
            } else if (get_value_return_value == 1) {
                exit(2);
            }
            printf("Tuple %d: value1 = '%s', N_Value2 = %d, V_Value2 = ", key, value1, N_Value2);
            printf("%.2f", V_Value2[0]);
            for (int i = 1; i < N_Value2; i++) {
                printf(", %.2f", V_Value2[i]);
            }
            printf("\n");
        } else if (strcmp(argv[1], "exit") == 0) {
            if (create_socket() < 0) {
                exit(-1);
            };
            exit(close_server());
        } else if (strcmp(argv[1], "delete") == 0) {
            if (argc != 3) {
                printf("Usage: ./client delete <key>\n");
                exit(-1);
            }
            if (isdigit(*argv[2]) == 0) {
                printf("Usage: ./client delete <key>\n");
                exit(-1);
            }
            if (create_socket() < 0) {
                exit(-1);
            };
            int key = atoi(argv[2]);
            exit(delete_key(key));
        } else if (strcmp(argv[1], "exist") == 0) {
            if (argc != 3) {
                printf("Usage: ./client exist <key>\n");
                exit(-1);
            }
            if (isdigit(*argv[2]) == 0) {
                printf("Usage: ./client exist <key>\n");
                exit(-1);
            }
            if (create_socket() < 0) {
                exit(-1);
            };
            int key = atoi(argv[2]);
            int exist_return_value = exist(key);
            if (exist_return_value == 0) {
                printf("Key %d doesn't exist\n", key);
            } else if (exist_return_value == 1) {
                printf("Key %d exists\n", key);
            } else {
                exit(-1);
            }
        } else if (strcmp(argv[1], "set") == 0) {
            if (argc < 6) {
                printf("Usage: ./client set <key> <value_1> <N_Value2> <V_Value>\n");
            }
            if (isdigit(*argv[2]) == 0) {
                printf("Usage: ./client set <key> <value_1> <N_Value2> <V_Value>\n");
                exit(1);
            }
            int key = atoi(argv[2]);
            char value_1[256];
            strcpy(value_1, argv[3]);
            if (isdigit(*argv[4]) == 0) {
                printf("Usage: ./client set <key> <value_1> <N_Value2> <V_Value>\n");
                exit(1);
            }
            int N_Value2 = atoi(argv[4]);
            if (argc != N_Value2 + 5) {
                printf("Number of elements in V_Value2 differs from the one specified in N_Value2\n");
                exit(1);
            }
            double V_Value2[32];
            for (int i = 0; i < N_Value2; i++) {
                if (sscanf(argv[5+i], "%lf", &V_Value2[i]) != 1) {
                    printf("Vector elements must be numeric\n");
                    exit(1);
                }
                V_Value2[i] = atof(argv[5+i]);
            }
            if (create_socket() < 0) {
                exit(1);
            };
            sleep(0.1);
            exit(set_value(key, value_1, N_Value2, V_Value2));
        } else if (strcmp(argv[1], "modify") == 0) {
            if (argc < 6) {
                printf("Usage: ./client modify <key> <value_1> <N_Value2> <V_Value>\n");
            }
            if (isdigit(*argv[2]) == 0) {
                printf("Usage: ./client modify <key> <value_1> <N_Value2> <V_Value>\n");
                exit(1);
            }
            int key = atoi(argv[2]);
            char value_1[256];
            strcpy(value_1, argv[3]);
            if (isdigit(*argv[4]) == 0) {
                printf("Usage: ./client modify <key> <value_1> <N_Value2> <V_Value>\n");
                exit(1);
            }
            int N_Value2 = atoi(argv[4]);
            if (argc != N_Value2 + 5) {
                printf("Number of elements in V_Value2 differs from the one specified in N_Value2\n");
                exit(1);
            }
            double V_Value2[32];
            for (int i = 0; i < N_Value2; i++) {
                if (sscanf(argv[5+i], "%lf", &V_Value2[i]) != 1) {
                    printf("Vector elements must be numeric\n");
                    exit(1);
                }
                V_Value2[i] = atof(argv[5+i]);
            }
            if (create_socket() < 0) {
                exit(1);
            };
            sleep(0.1);
            exit(modify_value(key, value_1, N_Value2, V_Value2));
        } else {
            printf("Argument not related to any operation, entering user input mode.\n");
            return;
        }
        exit(0);
    } else {
        return;
    }
}


int main (int argc, char *argv[]) {
    handle_arguments(argc, argv);
    printf("Welcome to the tuple management system, user input mode.\n");
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