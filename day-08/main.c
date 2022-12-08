#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#include "../common.c"

#define MAPSIZE 99
#define TOP 0
#define RIGHT 1
#define BOTTOM 2
#define LEFT 3

typedef uint8_t map_t[MAPSIZE][MAPSIZE];
typedef uint32_t map_32_t[MAPSIZE][MAPSIZE];

int char_to_int(char ch) {
    return ch - 48;
}

void m_init(map_t *map) {
    memset(map, 0, MAPSIZE*MAPSIZE);
}

void m_print(map_t *map) {
    for (int i = 0; i < MAPSIZE; i++) {
        for (int j = 0; j < MAPSIZE; j++) {
            printf("%d", (*map)[i][j]);
        }
        printf("\n");
    }
}

void m_output_ppm(map_t *map, const char *filename, int max_value) {
    FILE *f = fopen(filename, "w");

    if (!f) {
        panic("[output_ppm]: failed to open file!");
    }

    fprintf(f,"P3\n99 99\n255\n");

    for (int col = 0; col < MAPSIZE; col++) {
        for (int line = 0; line < MAPSIZE; line++) {
            float val = (float)((*map)[line][col])/max_value;
            int value = (int) (val * 255);
            fprintf(f, "%d %d %d ", value, 0, 0);
        }
        fprintf(f,"\n");
    }
}

void m32_output_ppm(map_32_t *map, const char *filename, int max_value) {
    FILE *f = fopen(filename, "w");

    if (!f) {
        panic("[output_ppm]: failed to open file!");
    }

    fprintf(f,"P3\n99 99\n255\n");

    for (int col = 0; col < MAPSIZE; col++) {
        for (int line = 0; line < MAPSIZE; line++) {
            float val = (float)(log((*map)[line][col]))/log(max_value);
            // float val = (float)((*map)[line][col])/max_value;
            int value = (int) (val *255);
            fprintf(f, "%d %d %d ", value, 0, 0);
        }
        fprintf(f,"\n");
    }
}

int main() {
    map_t map, map_density;
    map_32_t map_scenic;
    
    m_init(&map);
    m_init(&map_density);
    
    // m_print(&map);

    FILE *f = fopen("input", "r");
    char buf[101];
    int line = 0;
    while (fgets(buf, 101, f)) {
        for (int i = 0; i < MAPSIZE; i++) {
            map[line][i] = char_to_int(buf[i]);
        }
        line++;
    }
    
    int count = 0;
    int num = 0;

    for (int line = 0; line < MAPSIZE; line++)  {
        for (int col = 0; col < MAPSIZE; col++) {
            num++;

            int current = map[line][col];
            bool visible[4] = { true, true, true, true };

            for (int top = line - 1; top >= 0; top--) {
                int c = map[top][col];

                if (c >= current) {
                    visible[TOP] = false;
                    break;
                }
            }

            for (int bottom = line + 1; bottom < MAPSIZE; bottom++) {
                int c = map[bottom][col];

                if (c >= current) {
                    visible[BOTTOM] = false;
                    break;
                }
            }

            for (int left = col - 1; left >= 0; left--) {
                int c = map[line][left];

                if (c >= current) {
                    visible[LEFT] = false;
                    break;
                }
            }

            for (int right = col + 1; right < MAPSIZE; right++) {
                int c = map[line][right];

                if (c >= current) {
                    visible[RIGHT] = false;
                    break;
                }
            }

            if (visible[0] || visible[1] || visible[2] || visible[3]) {
                count++;
            }

            int visible_sides = 0;
            for (int i = 0; i < 4; i++) {
                visible_sides += visible[i] ? 1 : 0;
            }

            map_density[line][col] = visible_sides;
        }
    }

    printf("# of trees visible oustide the grid = %d of %d\n", count, num);

    m_output_ppm(&map_density, "dens.ppm", 4);

    // Part 2
    int max_score = 0;
    for (int line = 0; line < MAPSIZE; line++) {
        for (int col = 0; col < MAPSIZE; col++) {

            int current = map[line][col];
            int counter[4] = {0, 0, 0, 0};

            for (int top = line - 1; top >= 0; top--) {
                counter[TOP]++;
                if (map[top][col] >= current) break;
            }
            
            for (int bottom = line + 1; bottom < MAPSIZE; bottom++) {
                counter[BOTTOM]++;
                if (map[bottom][col] >= current) break;
            }
            
            for (int left = col - 1; left >= 0; left--) {
                counter[LEFT]++;
                if (map[line][left] >= current) break;
            }
            
            for (int right = col + 1; right < MAPSIZE; right++) {
                counter[RIGHT]++;
                if (map[line][right] >= current) break;
            }

            int score = counter[0] * counter[1] * counter[2] * counter[3];

            map_scenic[line][col] = score;
            
            if (score > max_score) max_score = score;
        }
    }
 
    printf("The highest scenic score is: %d\n", max_score);

    m32_output_ppm(&map_scenic, "scenic.ppm", max_score);
   
    return 0;
}
