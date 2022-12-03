#include "../common.c"
#include <assert.h>
#include <stdbool.h>

bool isLower(char ch) {
    return ch >= 97 && ch <= 122;
}

bool isUpper(char ch) {
    return ch >= 65 && ch <= 90;
}

char toLower(char ch) {
    if (isLower(ch)) {
        return ch;
    }

    return ch + 32;
}

char toUpper(char ch) {
    if (isUpper(ch)) {
        return ch;
    }

    return ch - 32;
}

int priority(char ch) {
    if (isLower(ch)) {
        return ch - 96;
    } else if (isUpper(ch)) {
        return ch - 38;
    } 

    panic("[priority]: unreachable");

    return 0;
}

int main() {
    char **lines = c_read_file_lines("input");
    char *line;

    int line_idx = 0;
    int sum = 0;
    while ((line = lines[line_idx++])) {
        int len = strlen(line);
        assert(len % 2 == 0);
        
        int items_per_side = len / 2; 

        char result = 0;
        
        bool found = false;
        for (int j = 0; j < items_per_side; j++) {
            char left = line[j];
            for (int k = 0; k < items_per_side; k++) {
                char right = line[items_per_side + k];

                if (left == right) {
                    found = true;
                    break;
                }
            }

            if (found) {
                result= left;
                break;
            }
        }

        sum += priority(result);
    }

    printf("sum of priorities: %d\n", sum);

    // Part 2
    sum = 0;
    for (int idx = 0; idx < (line_idx-2); idx += 3) {
        char q[128];
        int count = 0;
        char *first_line = lines[idx];
        char *second_line = lines[idx + 1];
        char *third_line = lines[idx + 2];

        for (int i = 0; i < strlen(first_line); i++) {
            char ch = first_line[i];
            for (int j = 0; j < strlen(second_line); j++) {
                if (ch == second_line[j]) {
                    q[count] = ch;
                    count++;
                }
            }
        }

        char result = 0;
        bool found = false;
        for (int i = 0; i < count; i++) {
            char ch = q[i];
            for (int j = 0; j < strlen(third_line); j++) {
                if (ch == third_line[j]) {
                    result = ch;
                    found = true;
                    break;
                }
            }

            if (found) {
                break;
            }
        }

        sum += priority(result);
    }

    printf("group priority sum = %d\n", sum);
    
    return 0;
}
