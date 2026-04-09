#ifndef RAMFS_H
#define RAMFS_H
#include <stdint.h>

typedef enum {
    NODE_FILE,
    NODE_DIR
} node_type_t;

typedef struct node {
    char name[32];
    node_type_t type;

    uint8_t *data;     // 文件内容
    uint32_t size;

    struct node *parent;
    struct node *children[16];
    int child_count;
} node_t;

typedef struct {
    node_t *node;
    uint32_t offset;
} file_t;

node_t *lookup(const char *path);
void ramfs_init();
#endif // RAMFS_H
