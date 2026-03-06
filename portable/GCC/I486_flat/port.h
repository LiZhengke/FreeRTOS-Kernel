#ifndef PORT_H
#define PORT_H

#include <stdint.h>
#include <stddef.h>
void* memset(void *s, int c, size_t n);
void* memcpy(void *dest, const void *src, size_t n);
#endif // PORT_H