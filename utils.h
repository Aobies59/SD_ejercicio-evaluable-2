#ifndef UTILS_H
#define UTILS_H

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


#endif