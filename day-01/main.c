#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void check_top_three(int *values, int new_value) {
    if (new_value > values[0]) {
        values[2] = values[1];
        values[1] = values[0];
        values[0] = new_value;
    } else if (new_value > values[1]) {
        values[2] = values[1];
        values[1] = new_value;
    } else if (new_value > values[2]) {
        values[2] = new_value;
    }
}

int main() {
    FILE *f = fopen("input", "r");
    if (!f) {
        printf("Failed to open file!");
        exit(1);
    }

    size_t len;
    fseek(f, 0, SEEK_END);
    len = (size_t) ftell(f);
    fseek(f, 0, SEEK_SET);
    
    char *buf = (char *) malloc(len);
    if (!buf) {
        printf("Failed to allocate buffer for file!");
        exit(1);
    }

    size_t bytes_read = fread(buf, 1, (size_t)len, f);
    if (bytes_read != len) {
        printf("Read error!");
        exit(1);
    }

    int i = 0;
    int line_start = 0;
    char last_char = 0;

    int max_elf = -1;
    int max_value = -1;
    int current_sum = 0;
    int current_elf = 0;
    int total = 0;

    int top_three[3] = {0, 0, 0};
    
    while (i <= len) {

        char current = buf[i];
        if (current == '\n' || current == 0) {
            if (last_char == current && current == '\n') {
                
                check_top_three(top_three, current_sum);
                if (current_sum > max_value) {
                    max_value = current_sum;
                    max_elf = current_elf;
                }
                
                current_elf++;
                current_sum = 0;
                line_start = i+1;
                i++;
                continue;
            } else {
                int line_len = i - line_start;
                char tmp[10];
                memcpy(tmp, &buf[line_start], line_len);
                tmp[line_len] = 0;
                int num = atoi(tmp);
                current_sum += num;
                total += num;
                line_start = i+1;

                if (current == 0) {
                    check_top_three(top_three, current_sum);
                    if (current_sum > max_value) {
                        max_value = current_sum;
                        max_elf = current_elf;
                    }
                }
            }
        }
        
        last_char = current;
        i++;
    }

    printf("max_value = %d, max_elf = %d\n", max_value, max_elf);
    printf("top three total: %d\n", top_three[0] + top_three[1] + top_three[2]);
}
