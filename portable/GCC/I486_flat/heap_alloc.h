#ifndef HEAP_ALLOC_H
#define HEAP_ALLOC_H

#include <stdint.h>

void  kmalloc_init(uint32_t initial_pages);
void *kmalloc(uint32_t size);
void  kfree(void *ptr);

#endif /* HEAP_ALLOC_H */
