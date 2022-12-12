#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BRK __builtin_debugtrap()

void panic(const char *msg) {
    printf("PANIC: %s\n", msg);
    BRK;
    exit(1);
}

long c_file_size(FILE *f) {
    long old = ftell(f);
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, old, SEEK_SET);

    return size;
}

char **c_read_file_lines(const char *file_name) {
    FILE *f = fopen(file_name, "r");
    if (!f) {
        return 0;
    }

    size_t size = (size_t) c_file_size(f);

    if (size == 0) { 
        return 0;
    }
    
    char *buffer = (char *)malloc(size+1);
    char **result = 0;
    
    fread(buffer, 1, size, f);
    buffer[size] = 0;
    
    int line_count = 0;
    for (int pass = 0; pass < 2; pass++) {
        if (pass == 1) {
            result = (char **) malloc((line_count + 1) * sizeof(char *) + size + 1);
            char *tmp = (char *) (result + (line_count + 1) * sizeof(char *));
            memcpy(tmp, buffer, size + 1);
            free(buffer);
            buffer = tmp;
        }

        char *start = &buffer[0];
        line_count = 0;

        for (int i = 0; i < size; i++) {

            char ch = buffer[i];


            if (ch == '\n') {
                line_count++;
                if (pass == 1) {
                    buffer[i] = 0;
                    result[line_count-1] = start;
                    start = &buffer[i+1];
                }
            }
        }
    }

    result[line_count] = 0;

    return result;
}

int c_parse_int_range(char *start, int len) {
    if (len >= 32) {
        panic("[parse_int_range]: String too large!");
    }
    char buf[32];
    for (int i = 0; i < len; i++) {
        buf[i] = start[i];
    }
    buf[len] = 0;
    
    return atoi(buf);
}

