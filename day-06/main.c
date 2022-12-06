#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <math.h>
#include <string.h>

#include "input.h"

int32_t table[26];

void init_lookup_table() {
    for (int i = 0; i < 26; i++) {
        table[i] = 1 << i;
    }
}

#define LOOKUP(x) table[(x) - 97]
#define GET(x) LOOKUP(INPUT[(x)])

int main() {
    init_lookup_table();

    int i = 0;
    while (INPUT[i+3]) {
        int32_t a = GET(i);
        int32_t b = GET(i+1);
        int32_t c = GET(i+2);
        int32_t d = GET(i+3);
        int32_t x = (a & b) || (a & c) || (a & d) || (b & c) || (b & d) || (c & d);
        if (x == 0) {
            printf("%d: %c%c%c%c\n", i + 4, INPUT[i], INPUT[i+1], INPUT[i+2], INPUT[i+3]);
            break;
        }
        i++;
    }

    // Part 2

    i = 0;
start:
    if (!INPUT[i+13]) {
        printf("Failed to find message!");
        return 0;
    };

    for (int j = 0; j < 13; j++) {
        for (int k = j + 1; k < 14; k++) {
            if (INPUT[i + j] == INPUT[i + k]) {
                i += (j + 1);
                goto start;
            }
        }
    }

    printf("%d: ", i+14);
    for (int j = 0; j < 14; j++) 
        printf("%c", INPUT[i+j]);
}
