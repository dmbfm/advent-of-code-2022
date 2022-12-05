#include "../common.c"
#include <assert.h>
#include <ctype.h>

#define COLS 9
#define ROWS 56

#define VALID_COL(x) ((x) >= 0 && (x) < COLS)
#define VALID_ROW(x) ((x) >= 0 && (x) < ROWS)

int count_crates(char stacks[COLS][ROWS]) {
    int count = 0;
    for (int row = 0; row < ROWS; row++) {
        for (int col = 0; col < COLS; col++) { 
            if (stacks[col][row] != ' ') {
                count++;
            }
        }
    }
    return count;
}

void print_stack(char stacks[COLS][ROWS]) {
    for (int row = 0; row < ROWS; row++) {
        for (int col = 0; col < COLS; col++) { 
            printf("[%c]", stacks[col][row]);
        }
        printf("\n");
    }
};

int col_get_top_row(char stacks[COLS][ROWS], int col) {
    for (int row = 0; row < ROWS; row++) {
        if (stacks[col][row] != ' ') {
            return row;
        }
    }

    return ROWS;
}

void move_crate(char stacks[COLS][ROWS], int col_from, int col_to) {
    if (!VALID_COL(col_from) || !VALID_COL(col_to)) {
        panic("Invalid column");
    }

    int row_from = col_get_top_row(stacks, col_from);
    int row_to = col_get_top_row(stacks, col_to);

    if (!VALID_ROW(row_to-1) || !VALID_ROW(row_from)) {
        panic("Invalid row!");
    }

    stacks[col_to][row_to-1] = stacks[col_from][row_from];
    stacks[col_from][row_from] = ' ';
}

void move_crate_9001(char stacks[COLS][ROWS], int num_crates, int col_from, int col_to) {
    if (!VALID_COL(col_from) || !VALID_COL(col_to)) {
        printf("%d, %d\n", col_from, col_to);
        panic("Invalid column");
    }

    int row_from = col_get_top_row(stacks, col_from);
    int row_to = col_get_top_row(stacks, col_to);

    for (int i = 0; i < num_crates; i++) {

        stacks[col_to][row_to -i -1] = stacks[col_from][row_from + num_crates - 1 - i];
        stacks[col_from][row_from + num_crates - 1 - i] = ' ';
    }
}

int parse_num(char *at, int *out_len) {
    char num_str[3];
    int len = (isdigit(at[1])) ? 2 : 1;
    num_str[0] = at[0];
    num_str[1] = len == 2 ? at[1] : 0;
    num_str[2] = 0;
    *out_len = len;
    return atoi(num_str);
}

void do_part(int part) {
    char buf[64];
    FILE *f = fopen("input", "r");
    int line = 0;
    char stacks[COLS][ROWS];
    memset(stacks, ' ', COLS * ROWS);
    
    while (fgets(buf, 64, f)) {
        if (line >= 0 && line < 8) {
            for (int i = 0; i < COLS; i++) {
                char ch = buf[1 + i * 4];
                stacks[i][48 + line] = ch;
            }
        } else if (line >= 10) {
            int cur = 5;
            int len;
            int num_crates = parse_num(&buf[cur], &len);
            cur += len;
            cur += 6;
            int from = parse_num(&buf[cur], &len);
            cur += 5;
            int to = parse_num(&buf[cur], &len);

            if (part == 1) {
                for (int i = 0; i < num_crates; i++) {
                    move_crate(stacks, from-1, to-1);
                }
            } else if (part == 2){
                move_crate_9001(stacks, num_crates, from-1, to-1);
            }
        }
        
        line++;
    }

    for (int i = 0; i < COLS; i++) {
        printf("%c", stacks[i][col_get_top_row(stacks, i)]);
    }
    printf("\n");

    fclose(f);
}

int main() {
    do_part(1);
    do_part(2);
    return 0;
}
