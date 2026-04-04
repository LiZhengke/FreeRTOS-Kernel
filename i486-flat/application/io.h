#ifndef IO_H
#define IO_H

void puts(const char *str);
int write(int fd, const void *str, int len);
int printf(const char *restrict format, ...);
#endif // IO_H
