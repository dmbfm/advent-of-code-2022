#include <stddef.h>
#include <ctype.h>
#include "../common.c"

#define BRK __builtin_debugtrap();

int parse_int_range(char *start, int len) {
    if (len >= 32) {
        panic("[parse_int_range]: String too large!");
    }
    char buf[32];
    for (int i = 0; i < len; i++) {
        buf[i] = start[i];
    }
    buf[len] = 0;
    
    return atoi(buf);
}

#define MAX_DIRS 1024

typedef struct dir_t {
     const char *name;
     struct dir_t *first_child;
     struct dir_t *next;
     struct dir_t *parent;
     size_t size;
} dir_t;

void d_init(dir_t *d, const char *name) {
    *d = (dir_t) {0};
    d->name = name;
}

void d_add_child(dir_t *d, dir_t *child) {
    if (d->first_child == 0) {
        d->first_child = child;
        child->parent = d;
        return;
    } 

    dir_t *current = d->first_child;
    while (current->next) {
        current = current->next;
    }

    current->next = child;
    child->parent = d;
}

dir_t *d_cd(dir_t *d, const char *name) {
    if (!d->first_child) return 0;

    dir_t *current = d->first_child;
    while (current) {
        if (strcmp(d->name, name) == 0) {
            return current;
        }
        current = current->next;
    }

    return 0;
}
 
typedef struct {
     dir_t dirs[MAX_DIRS];
     dir_t *root;
     int num_dirs;
} file_system_t;

dir_t  *fs_alloc_dir(file_system_t *fs) {
    if (fs->num_dirs >= MAX_DIRS) {
        panic("Max dirs reached!");
    }

    fs->num_dirs++;
    return &fs->dirs[fs->num_dirs-1];
}

dir_t *fs_cd_or_add(file_system_t *fs, dir_t *dir, const char *name) {
    dir_t *result = d_cd(dir, name);

    if (result) return result;

    result = fs_alloc_dir(fs);
    d_init(result, name);
    d_add_child(dir, result);

    return result;
}

void d_print(struct dir_t *d, int level) {
    for (int i = 0; i < level; i++) {
        printf(".");
    }

    printf("%s/ [%lu]\n", d->name, d->size);

    if (!d->first_child) return;

    dir_t *current = d->first_child;
    while (current) {
        d_print(current, level + 1);
        current = current->next;
    }
}

void fs_print(file_system_t *fs) {
    if (fs->root == 0) return;
    d_print(fs->root, 0);
}

int main() {
    file_system_t fs  = {0};
    dir_t *root = fs_alloc_dir(&fs);
    d_init(root, "/");
    fs.root = root;

    char **lines = c_read_file_lines("input");
    char *line;
    dir_t *current_dir = root;

    while ((line = *lines++)) {
        
        if (line[0] == '$') {
            // command
            if (line[2] == 'l') {
                // ls
                continue;
            }
            
            if (line[5] == '/') {
                current_dir = fs.root;
                continue;
            }

            if (line[5] == '.') {
                current_dir = current_dir->parent;
                continue;
            }

            current_dir = fs_cd_or_add(&fs, current_dir, &line[5]);
            
        } else if (isdigit(line[0])) {
            // file listed
            
            int i = 0;
            while (isdigit(line[i])) {
                i++;
            }
            int file_size = parse_int_range(line, i);

            dir_t *current = current_dir;
            while (current) {
                current->size += file_size;
                current = current->parent;
            }
        } else if (line[0] == 'd') { 
            // dir listed
        }

    }

    size_t sum = 0;
    for (int i = 0; i < fs.num_dirs; i++) {
        dir_t *current = &fs.dirs[i];
        
        if (current->size <= 100000) {
            sum += current->size;
        }
        
    }

    printf("%lu\n", sum);

    // Part 2
    printf("Total used size: %lu\n", root->size);
    printf("Available size: %lu\n", 70000000 - root->size);

    size_t smallest = root->size;
    size_t amount_needed = 30000000 - (70000000 - root->size);
    
    printf("Amount we need to free: %lu\n", amount_needed);
    
    for (int i = 0; i < fs.num_dirs; i++) {
        dir_t *current = &fs.dirs[i];
        
        if (current->size >= amount_needed && current->size < smallest) {
            smallest = current->size;
        }
    }
    
    printf("Smallest dir we can delete has size: %lu\n", smallest);
}

