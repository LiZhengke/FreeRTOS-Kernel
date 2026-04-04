#include <stddef.h>
#include <string.h>
#include "FreeRTOS.h"
#include "ramfs.h"

node_t *root;
file_t fd_table[32];

static void *alloc(size_t size)
{
    return pvPortMalloc(size);
}

static void ramfs_strcpy(char *dst, const char *src)
{
    while (*src != '\0') {
        *dst++ = *src++;
    }

    *dst = '\0';
}

static int ramfs_strcmp(const char *a, const char *b)
{
    while ((*a != '\0') && (*a == *b)) {
        a++;
        b++;
    }

    return (int) ((unsigned char) *a - (unsigned char) *b);
}

static const uint8_t ucHelloData[] = "hello\n";

/**
  ******************************************************************************
 * @file    ramfs.c
 * @author  Max
 * @version V1.0.0
 * @date    06-June-2024
 * @brief   RAMFS implementation for FreeRTOS on i486 flat memory model.
 * File Name          : ramfs.c
 * Author             : Max
 * Version            : V1.0.0
 * Date               : 06-June-2024
 * Description        : RAMFS implementation for FreeRTOS on i486 flat memory model.
 ******************************************************************************
 * This software is licensed under the MIT License. You may obtain a copy of the
 * License at: https://opensource.org/licenses/MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this
 * software and associated documentation files (the "Software"), to deal in the Software
 * without restriction, including without limitation the rights to use, copy, modify,
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 *
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included in all copies
 * or substantial portions of the Software.
 */

node_t* create_dir(node_t *parent, const char *name) {
    node_t *n = alloc(sizeof(node_t)); // 用你自己的内存分配
    memset(n, 0, sizeof(node_t));

    ramfs_strcpy(n->name, name);
    n->type = NODE_DIR;
    n->parent = parent;

    if (parent) {
        parent->children[parent->child_count++] = n;
    }

    return n;
}

node_t* create_file(node_t *parent,
                    const char *name,
                    uint8_t *data,
                    uint32_t size) {

    node_t *n = alloc(sizeof(node_t));
    memset(n, 0, sizeof(node_t));

    ramfs_strcpy(n->name, name);
    n->type = NODE_FILE;
    n->data = data;
    n->size = size;
    n->parent = parent;

    parent->children[parent->child_count++] = n;

    return n;
}

node_t* lookup(const char *path) {
    if (!path || path[0] != '/') return NULL;

    node_t *cur = root;
    path++; // skip '/'

    char name[32];

    while (*path) {
        // 取一个路径组件
        int i = 0;
        while (*path && *path != '/') {
            name[i++] = *path++;
        }
        name[i] = '\0';

        // 在当前目录查找
        int found = 0;
        for (int j = 0; j < cur->child_count; j++) {
            if (ramfs_strcmp(cur->children[j]->name, name) == 0) {
                cur = cur->children[j];
                found = 1;
                break;
            }
        }

        if (!found) return NULL;

        if (*path == '/') path++;
    }

    return cur;
}

void ramfs_init() {
    root = create_dir(NULL, "/");

    node_t *bin = create_dir(root, "bin");

    uint32_t size = (uint32_t) (sizeof(ucHelloData) - 1U);

    create_file(bin, "hello",
                (uint8_t *)ucHelloData,
                size);
}
