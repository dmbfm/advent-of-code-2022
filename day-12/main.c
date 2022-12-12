#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <limits.h>

// #define TEST

#if defined(TEST)
    #define GRID_WIDTH 8
    #define GRID_HEIGHT 5
    #define INPUT "input2"
#else
    #define GRID_WIDTH 67
    #define GRID_HEIGHT 41
    #define INPUT "input"
#endif

#define BRK __builtin_debugtrap()

#define PANIC(msg) \
    {\
    printf(msg); \
    BRK;\
    }\
        
    
typedef enum {
    LEFT = 0,
    RIGHT = 1,
    UP = 2,
    DOWN = 3,
} direction_t;

typedef struct {
    int row;
    int col;
} pos_t;

bool pos_eql(pos_t lhs, pos_t rhs) {
    return lhs.row == rhs.row && lhs.col == rhs.col;
}

pos_t pos_move(pos_t p, direction_t dir) {
    pos_t result = p;

    switch (dir) {

        case LEFT:
                result.col--;
                break;
        case RIGHT:
                result.col++;
                break;
        case UP:
                result.row--;
                break;
        case DOWN:
                result.row++;
                break;
    }

    return result;
}

typedef struct {
    int grid[GRID_HEIGHT][GRID_WIDTH];
    int distance[GRID_HEIGHT][GRID_WIDTH];
    bool visited[GRID_HEIGHT][GRID_WIDTH];
    pos_t prev[GRID_HEIGHT][GRID_WIDTH];
    pos_t start_pos;
    pos_t end_pos;
} map_t;

int map_value_at_pos(map_t *m, pos_t p) {
    if (p.col < 0 || p.col >= GRID_WIDTH || p.row < 0 || p.row >= GRID_HEIGHT) {
        return INT_MAX;
    }

    return m->grid[p.row][p.col];
}

void map_set_distance(map_t *m, pos_t pos, int distance) {
    if (pos.row >= 0 && pos.row < GRID_HEIGHT && pos.col >= 0 && pos.col < GRID_WIDTH) {
        m->distance[pos.row][pos.col] = distance;
    } else {
        PANIC("[map_set_distance]: position out of bounds!");
    }
}

int map_get_distance(map_t *m, pos_t pos) {
    if (pos.row >= 0 && pos.row < GRID_HEIGHT && pos.col >= 0 && pos.col < GRID_WIDTH) {
        return m->distance[pos.row][pos.col];
    } else {
        PANIC("[map_set_distance]: position out of bounds!");
    }

    return -1;
}

void map_mark_visited(map_t *m, pos_t pos) {
    if (pos.row >= 0 && pos.row < GRID_HEIGHT && pos.col >= 0 && pos.col < GRID_WIDTH) {
        m->visited[pos.row][pos.col] = true;
    }
}

bool map_is_visited(map_t *m, pos_t pos) {
    if (pos.row >= 0 && pos.row < GRID_HEIGHT && pos.col >= 0 && pos.col < GRID_WIDTH) {
        return m->visited[pos.row][pos.col];
    }

    return false;
}

bool map_is_neighbor_visited(map_t *m, pos_t pos, direction_t dir) {
    return map_is_visited(m, pos_move(pos, dir));
}

bool map_can_move(map_t *m, pos_t p, direction_t dir) {
    pos_t final = pos_move(p, dir);

    int start_value = map_value_at_pos(m, p);
    int final_value = map_value_at_pos(m, final);

    return final_value <= (start_value + 1);
}


bool map_can_move_part_2(map_t *m, pos_t p, direction_t dir) {
    pos_t final = pos_move(p, dir);

    int start_value = map_value_at_pos(m, p);
    int final_value = map_value_at_pos(m, final);

    return final_value >= (start_value - 1) && final_value < 1000;
}


void parse_map(map_t *m) {
    FILE *f = fopen(INPUT, "r");
    char buf[GRID_WIDTH+2];
    int row = 0;
    while (fgets(buf, GRID_WIDTH+2, f)) {
        for (int col = 0; col < GRID_WIDTH; col++) {
            char ch = buf[col];
            
            if (ch == 'S') {
                m->start_pos = (pos_t) { .row = row, .col = col };
                m->grid[row][col] = 0;
            } else if (ch == 'E') {
                m->end_pos = (pos_t) { .row = row, .col = col };
                m->grid[row][col] = 'z' - 97;
            }  else {
                m->grid[row][col] = buf[col] - 97;
            }
        }
        row++;
    }
}

void print_map(map_t *m) {
    for (int row = 0; row < GRID_HEIGHT; row ++) {
        for (int col = 0; col < GRID_WIDTH; col++) {

            pos_t current = (pos_t) { row, col};

            if (pos_eql(m->start_pos, current)) {
                printf("S");
            } else if (pos_eql(m->end_pos, current)) {
                printf("E");
            } else {
                printf("%c", m->grid[row][col] + 97);
            }
        }
        printf("\n");
    }
}

void print_map_closed(map_t *m) {
     for (int row = 0; row < GRID_HEIGHT; row ++) {
        for (int col = 0; col < GRID_WIDTH; col++) {
            if (m->visited[row][col]) {
                printf("*");
            } else {
                printf("o");
            }
        }
        printf("\n");
    }
}

#define formap(a,b) \
    for (int a = 0; a < GRID_HEIGHT; a++) \
        for (int b = 0; b < GRID_WIDTH; b++) \

void calc_number_of_steps_to_target(map_t *map, bool is_part_two) {
    
    pos_t s = is_part_two ? map->end_pos : map->start_pos;
    pos_t e = map->end_pos;

    formap(row, col) {
        map->distance[row][col] = INT_MAX;
        map->visited[row][col] = false;
        map->prev[row][col] = (pos_t) { -1000, -1000 };
    }
    
    map->distance[s.row][s.row] = 0; 

    int unvisited_count = GRID_WIDTH * GRID_HEIGHT;
    int num_steps = 0;
    pos_t current = s;
    
    // BRK;
    // While we have unvisited nodes
    while (unvisited_count > 0) {
        num_steps++;

        // For every neighbor
        for (int dir = 0; dir < 4; dir++) {
            
            // If neighbor is acessible and unvisited
            bool can_move = is_part_two ?
                map_can_move_part_2(map, current, dir) :
                map_can_move(map, current, dir);
            
            if (can_move && !map_is_neighbor_visited(map, current, dir)) {
                
                // Get neighbor position
                pos_t neighbor_pos = pos_move(current, dir);

                // Calculate new tentative distance
                int new_distance = map_get_distance(map, current) + 1;

                // If tentative distance is shorter, substitute
                if (new_distance < map_get_distance(map, neighbor_pos)) {
                    map_set_distance(map, neighbor_pos, new_distance);
                    
                    // Save previous positon so we can retrace the path 
                    map->prev[neighbor_pos.row][neighbor_pos.col] = current;
                }
            }    
        }

        // Mark current node as visited
        map_mark_visited(map, current);
        unvisited_count--;
        
        // If destination is visited, we are done.
        if (is_part_two) {
            if (map->grid[current.row][current.col == 0]) {
                e = current;
                break;
            }
        } else if (map_is_visited(map, e)) {
            break;
        }

        // Find unvisited node with smallest tentative distance
        int min_dist = INT_MAX;
        pos_t next = (pos_t) {-1,-1};
        formap(_row, _col) {
            pos_t p;
            p.row = _row;
            p.col = _col;
            if (!map_is_visited(map, p)) {
                int dist = map_get_distance(map, p);
                if (dist < min_dist)  {
                    min_dist = dist;
                    next = p;
                }
            }
        }

        if (min_dist == INT_MAX) {
            return;
        }
        
        // Select unvisited node with smallest tentative distance
        current = next;
    }

    // Count steps by retracing the path
    num_steps = 0;
    current = e;

    pos_t path[512];
    int path_len = 0;
    
    while (true) {

        path[path_len++] = current;
        
        if (pos_eql(s, current)) {
            break;
        }
        
        current = map->prev[current.row][current.col];
        if (current.row < 0 || current.col < 0) {
            return;
        }
        num_steps++;
    }

    formap(row, col) {
        
        if (col == 0) {
            printf("\n");
        }

        pos_t p = (pos_t) {row, col};

        bool is_path = false;
        
        for (int i = 0; i < path_len; i++) {
            if (pos_eql(p, path[i])) {
                is_path = true;
                break;
            } 
        }

        if (is_path) {
            printf("o");
        } else {
            printf("-");
        }
    }

    printf("\nNumber of steps: %d\n", num_steps);
}

int main() {
    map_t map = {0};
    parse_map(&map);
    print_map(&map);
    printf("\n");

    printf("\n\n=== Part 1 ===\n");
    calc_number_of_steps_to_target(&map, false);
    
    printf("\n\n=== Part 2 ===\n");
    calc_number_of_steps_to_target(&map, true);
    
}
