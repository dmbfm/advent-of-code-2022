#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
        char *start = &buffer[0];
        line_count = 0;
        
        if (pass == 1) {
            result = (char **) malloc((line_count + 1) * sizeof(char *));
        }

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

