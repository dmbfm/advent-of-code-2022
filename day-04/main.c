#include "../common.c"
#include <stdbool.h>
#include <ctype.h>
#include <assert.h>

char *parse_range(char *line, int *start, int *end) {
    char buf[64];
    int i = 0;
    int *range[2] = { start, end };

    for (int j = 0; j < 2; j++) {
        while (isdigit(*line)) {
            buf[i++] = *line; 
            line++;
        }
        
        buf[i] = 0;
        *range[j] = atoi(buf);
        
        if (*line == '-') {
           line++;
        }
        i = 0;
    }

    return line+1;
}

int main() {

    char buf[64];

    FILE *f = fopen("input", "r");

    int count = 0;
    int overlap_count = 0;
    while (fgets(buf, 64, f)) {

        int s1 = -1; int s2 = -1; int e1 = -1; int e2 = -1;

        parse_range(parse_range(buf, &s1, &e1), &s2, &e2);

        if (s2 > s1 ? e2 <= e1 : s1 == s2 ? true : e1 <= e2) {
            count ++;
        }

        if ((s1 <= s2 && s2 <= e1) || (s2 < s1) && (s1 <= e2)) {
            overlap_count++;
        }
    } 

    printf("Number of pairs with full overlap: %d\n", count);
    printf("Number of pairs with any overlap: %d\n", overlap_count);
    
    return 0;
}
