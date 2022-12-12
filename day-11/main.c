#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <math.h>
#include "../common.c"

#define MAX_ITEMS 100

#if defined(TEST_INPUT)
    #define INPUT_FILENAME "input2"
    #define NUM_MONKEYS 4
#else
    #define INPUT_FILENAME "input"
    #define NUM_MONKEYS 8
#endif

#define BRK __builtin_debugtrap()

typedef struct {
    const char *buf;
    int cur;
} line_reader_t;

void line_reader_init(line_reader_t *r, const char *buf) {
    r->buf = buf;
    r->cur = 0;
}

bool line_reader_next(line_reader_t *r, char *out) {
    if (r->buf[r->cur] == '\n') {
        return false;
    }

    *out = r->buf[r->cur++];
    
    return true;
}

bool line_reader_peek(line_reader_t *r, char *out) {
    if (r->buf[r->cur] == '\n') {
        return false;
    }

    *out =  r->buf[r->cur];
    return true;
}

bool line_reader_seek(line_reader_t *r, char needle) {
    char ch;
    while (line_reader_next(r, &ch)) {
        if (ch == needle) {
            return true;
        }
    }

    return false;
}

bool line_reader_skip_next(line_reader_t *r) {
    char ch;
    return line_reader_next(r, &ch);
}

bool line_reader_skip_char(line_reader_t *r, char char_to_skip) {
    char ch;
    while(line_reader_peek(r, &ch)) {
        if (ch != char_to_skip) {
            return true;
        }

        if (!line_reader_skip_next(r)) {
            return false;
        }
    }

    return false;
}

bool parse_number(line_reader_t *r, int *out) {
    char ch = 0;

    if (line_reader_peek(r, &ch)) {
        if (!isdigit(ch)) {
            return false;
        }
    } else {
        return false;
    }

    int start = r->cur;
    int end = start;
    
    while (true) {
        if (!line_reader_peek(r, &ch)) {
            end = r->cur-1;
            break;
        }
        
        if (!isdigit(ch)) {
            end = r->cur-1;
            break;
        } else if (!line_reader_skip_next(r)) {
            end = r->cur-1;
            break;
        }
    }

    char buf[32];
    memcpy(buf, &r->buf[start], end - start + 1);
    buf[end - start + 1] = 0;

    *out = atoi(buf);

    return true;
}

int parse_number_list(line_reader_t *r, int *out_list, int len) {
    line_reader_skip_char(r, ' ');
    int num;
    int i = 0;
    while (parse_number(r, &num)) {
        out_list[i++] = num;
        if (i >= len) {
            panic("[parse_number_list]: out of bounds!");
        }

        line_reader_skip_char(r, ',');
        line_reader_skip_char(r, ' ');
    }

    return i;
}

typedef enum {
    OpArgOld,
    OpArgNum,
} op_arg_t;

typedef enum {
    OpAdd,
    OpMul,
} op_t;

typedef struct {
    op_t op;
    op_arg_t lhs;
    op_arg_t rhs;
    int rhs_value;
} operation_t;


typedef struct {
    int starting_items[100];
    int num_items;
    operation_t operation;
    int divisible_by;
    int target_if_true;
    int target_if_false;
} monkey_desc_t;

typedef struct {
    uint64_t items[100];
    int num_items;
    int inspections;
} monkey_state_t;

typedef struct {
    monkey_desc_t monkey_descs[NUM_MONKEYS];
    monkey_state_t monkeys[NUM_MONKEYS];
} state_t;

void op_print(operation_t *o) {
    printf("old ");
    switch (o->op) {
        case OpAdd:
            printf("+ ");
            break;

        case OpMul:
            printf("* ");
            break;
    }
    switch (o->rhs) {
        case OpArgOld:
            printf("old");
            break;
        case OpArgNum:
            printf("%d", o->rhs_value);
            break;
    }
}

operation_t parse_operation(line_reader_t *r) {
    operation_t result;
    result.lhs = OpArgOld;
    r->cur += 4;

    char ch = 0;
    
    if (line_reader_next(r, &ch)) {
        if (ch == '+') {
            result.op = OpAdd;
        } else if (ch == '*') {
            result.op = OpMul;
        } else {
            panic("?");
        }
    } else {
        panic("!");
    }

    line_reader_skip_next(r);

    ch = 0;
    if (line_reader_peek(r, &ch)) {
        if (ch == 'o') {
            result.rhs = OpArgOld;
            result.rhs_value = 0;
            return result;
        }
    }
    
    if (parse_number(r, &result.rhs_value)) {
        result.rhs = OpArgNum;
    } else {
        panic("$");
    }

    return result;
}

void monkey_descs_print(state_t *s) {
    for (int i = 0; i <NUM_MONKEYS; i++) {
        monkey_desc_t *m = &s->monkey_descs[i];
        printf("Monkey %d:\n", i);
        printf("\tStarting items(%d): ", m->num_items);
        for (int k = 0; k < m->num_items; k++) {
            printf("%d,", m->starting_items[k]);
        }
        printf("\n");
        printf("\tOperation: ");
        op_print(&m->operation);
        printf("\n");

        printf("\tTest: divisible by %d\n", m->divisible_by);
        printf("\t\tIf true: throw to monkey %d\n", m->target_if_true);
        printf("\t\tIf false: throw to monkey %d\n", m->target_if_false);

        printf("\n");
    }
}

void monkey_state_print(monkey_state_t *m) {
    printf("(%d): ", m->num_items);
    for (int i = 0; i < m->num_items; i++) {
        printf("[%llu]", m->items[i]);
    }
    printf(". inspections = %d", m->inspections);
}

void monkey_states_print(state_t *s) {
    for (int i = 0; i < NUM_MONKEYS; i++) {
        monkey_state_t *m = &s->monkeys[i];
        
        printf("Monkey %d: \n", i);
        printf("\tItems: ");
        for (int j = 0; j < m->num_items; j++) {
            printf("[%llu]", m->items[j]);
        }
        printf("\n\tInspections: %d\n", m->inspections);
    }
}

void set_initial_state(state_t *state) {
        for (int i = 0; i < NUM_MONKEYS; i++) {
        state->monkeys[i].inspections = 0;
        state->monkeys[i].num_items = 0;
        state->monkeys[i].num_items = state->monkey_descs[i].num_items;
        for (int j = 0; j < state->monkey_descs[i].num_items; j++) {
            state->monkeys[i].items[j] = (uint64_t) state->monkey_descs[i].starting_items[j];
        }
    }
}

uint64_t calc_monkey_business(state_t *state) {
    uint64_t m1 = 0; //state->monkeys[0].inspections;
    uint64_t m2 = 0; //m1;

    for (int i = 0; i < NUM_MONKEYS; i++) {
        monkey_state_t *m = &state->monkeys[i];

        if (m->inspections > m1) {
            m2 = m1;
            m1 = m->inspections;
        } else if (m->inspections > m2) {
            m2 = m->inspections;
        }
    }

    return m1 * m2;
}

int main() {
    FILE *f = fopen(INPUT_FILENAME, "r");
    char buf[64];
    int linum = 0;

    // monkey_desc_t monkey_descs[8];
    state_t state = { 0 };
    int current_monkey = 0;

    //BRK;

    // Parse
    while (fgets(buf, 64, f)) {

        line_reader_t r;
        line_reader_init(&r, buf);

        linum++;
        int relative = linum % 7;

        monkey_desc_t *m = &state.monkey_descs[current_monkey];

        switch (relative) {
            case 0:
                current_monkey++;
                break;

            case 2:
                {
                    line_reader_seek(&r, ':');
                    m->num_items = parse_number_list(&r, m->starting_items, 100);
                } break;

            case 3:
                {
                    line_reader_seek(&r, '=');
                    line_reader_skip_char(&r, ' ');
                    m->operation = parse_operation(&r);
                } break;

            case 4:
                {
                    line_reader_seek(&r, 'y');
                    line_reader_skip_char(&r, ' ');
                    if (!parse_number(&r, &m->divisible_by)) {
                        panic("Failed to parse Test!");
                    }
                } break;

            case 5:
                {
                    r.cur = 29;
                    if (!parse_number(&r, &m->target_if_true)) {
                        panic("Failed to parse true condition!");
                    }
                } break;

            case 6:
                {
                    r.cur = 30;
                    if (!parse_number(&r, &m->target_if_false)) {
                        panic("Failed to parse false condition!");
                    }
                } break;
        }
    }

    set_initial_state(&state);

    for (int i = 0; i < 20; i++) {
        
        // For each monkey
        for (int k = 0; k < NUM_MONKEYS; k++) {

            monkey_state_t *m = &state.monkeys[k];
            monkey_desc_t *desc = &state.monkey_descs[k];

            // For each item
            for (int j = 0; j < m->num_items; j++) {
                m->inspections++;
                uint64_t item = m->items[j];
                uint64_t rhs = 0;
                uint64_t new = 0;

                switch (desc->operation.rhs) {
                    case OpArgOld:
                        rhs = item;
                        break;

                    case OpArgNum:
                        rhs = (uint64_t) desc->operation.rhs_value;
                        break;
                }

                switch (desc->operation.op) {
                    case OpAdd:
                        new = item + rhs;
                        break;

                    case OpMul:
                        new = item * rhs;
                        break;
                }

                // divide by 3
                uint64_t final = new / 3;

                int target = -1000;
                if (final % desc->divisible_by == 0) {
                    target = desc->target_if_true;
                } else {
                    target = desc->target_if_false;
                }

                monkey_state_t *tm = &state.monkeys[target];
                tm->items[tm->num_items++] = final; //item;
            }

            m->num_items = 0;
        }
    }

    uint64_t result = calc_monkey_business(&state);
    
    printf("Level of monkey business: %llu\n\n", result);

    // Part 2
    set_initial_state(&state);
 
    uint64_t lcm = 1;
    for (int i = 0; i < NUM_MONKEYS; i++) {
        lcm *= state.monkey_descs[i].divisible_by;
    }

    // Play 10000 rounds
    for (int i = 0; i < 10000; i++) {
               // For each monkey
        for (int k = 0; k < NUM_MONKEYS; k++) {
            monkey_state_t *m = &state.monkeys[k];
            monkey_desc_t *desc = &state.monkey_descs[k];

            // For each item
            for (int j = 0; j < m->num_items; j++) {
                
                // increment inspections
                m->inspections++;
                operation_t *op = &desc->operation;

                uint64_t val = m->items[j];

                switch(op->op) {
                    case OpAdd:
                        {
                            val = (val + op->rhs_value) % lcm;
                        }
                        break;

                    case OpMul:
                        {
                            if (op->rhs == OpArgOld) {
                                val = (val * val) % lcm;
                            } else {
                                val = (val * op->rhs_value) % lcm;
                            }
                        } break;
                }

                int target = (val % desc->divisible_by == 0) ? desc->target_if_true : desc->target_if_false;
                monkey_state_t *target_monkey = &state.monkeys[target];
                target_monkey->items[target_monkey->num_items++] = val;
            }
            
            m->num_items = 0;
        }
    }

    result = calc_monkey_business(&state);
    printf("Level of monkey business: %llu\n\n", result);
}
