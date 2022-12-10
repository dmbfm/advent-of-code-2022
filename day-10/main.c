#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "../common.c"


typedef struct {
    int cycle;
    int x;
    int sum;
} cpu_t;

typedef enum {
    NOOP,
    ADDX,
} cpu_op_t;


void cpu_init(cpu_t *cpu) {
    cpu->cycle = 0;
    cpu->x = 1;
    cpu->sum = 0;
}

void cpu_draw_pixel(cpu_t *cpu) {
    int px = (cpu->cycle - 1) % 40;

    if (px >= (cpu->x -1) && px <= (cpu->x +1)) {
        printf("#");
    } else {
        printf(".");
    }
}

void cpu_next_cycle(cpu_t *cpu) {
    cpu->cycle++;

    cpu_draw_pixel(cpu);
    
    if ((cpu->cycle - 20) % 40 == 0) {
        cpu->sum += cpu->cycle * cpu->x;
    }

    if (cpu->cycle % 40 == 0) {
        printf("\n");
    }
}

void cpu_execute(cpu_t *cpu, cpu_op_t op, int arg) {
    switch (op) {
        case NOOP:
            cpu_next_cycle(cpu);
            break;

        case ADDX:
            cpu_next_cycle(cpu);
            cpu_next_cycle(cpu);
            cpu->x += arg;
            break;
    }
}

void cpu_print(cpu_t *cpu) {
    printf("\n{ cycles = %d, x = %d, sum = %d}\n", cpu->cycle, cpu->x, cpu->sum);
}

int main() {

    char buf[10];
    FILE *f = fopen("input", "r");

    cpu_t cpu;
    cpu_init(&cpu);

    while (fgets(buf, 10, f)) {
        if (buf[0] == 'n') {
            cpu_execute(&cpu, NOOP, 0);
        } else {
            int arg = atoi(&buf[5]);
            cpu_execute(&cpu, ADDX, arg);
        }
    }

    cpu_print(&cpu);
    
    return 0;
}
