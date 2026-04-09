#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include "io.h"

#define SYS_WRITE 1
#define SYSINT 0x30

#define _STR(x) #x
#define STR(x) _STR(x)

static int sys_write(int fd, const void *str, int len)
{
    int ret;

    __asm__ volatile (
        "int $" STR(SYSINT)
        : "=a"(ret)
        : "a"(SYS_WRITE),
          "b"(fd),
          "c"(str),
          "d"(len)
        : "memory"
    );

    return ret;
}

int write(int fd, const void *str, int len)
{
    return sys_write(fd, str, len);
}

void puts(const char *str)
{
    while(*str)
        write(1, str++, 1);
    write(1, "\n", 1);
}

static void print_str(const char *s) {
    if (s == NULL) {
        s = "(null)";
    }

    while (*s) write(1, s++, 1);
}

static void print_int(int x) {
    char buf[16];
    int i = 0;

    if (x == 0) {
        write(1, "0", 1);
        return;
    }

    if (x < 0) {
        write(1, "-", 1);
        x = -x;
    }

    while (x > 0) {
        buf[i++] = '0' + (x % 10);
        x /= 10;
    }

    while (i--) write(1, &buf[i], 1);
}

static void print_long(long x) {
    char buf[32];
    int i = 0;

    if (x == 0) {
        write(1, "0", 1);
        return;
    }

    if (x < 0) {
        write(1, "-", 1);
        x = -x;
    }

    while (x > 0) {
        buf[i++] = (char) ('0' + (x % 10));
        x /= 10;
    }

    while (i--) write(1, &buf[i], 1);
}

static void print_ulong(unsigned long x)
{
    char buf[32];
    int i = 0;

    if (x == 0UL) {
        write(1, "0", 1);
        return;
    }

    while (x > 0UL) {
        buf[i++] = (char) ('0' + (x % 10UL));
        x /= 10UL;
    }

    while (i--) write(1, &buf[i], 1);
}

static void print_hex(unsigned int x, unsigned int width, int zeroPad, int uppercase, int prefix)
{
    char buf[16];
    int i = 0;
    char padChar = zeroPad ? '0' : ' ';

    if (x == 0) {
        buf[i++] = '0';
    }
    else {
        while (x > 0) {
            int d = x & 0xF;
            buf[i++] = (char) ( d < 10 ? '0' + d : ( uppercase ? 'A' : 'a' ) + d - 10 );
            x >>= 4;
        }
    }

    while ((unsigned int) i < width) {
        buf[i++] = padChar;
    }

    if (prefix != 0) {
        write(1, "0x", 2);
    }

    while (i--) write(1, &buf[i], 1);
}

int printf(const char *restrict format, ...)
{
    va_list ap;
    int written = 0;

    va_start(ap, format);

    while (*format) {
        if (*format == '%') {
            unsigned int width = 0;
            int zeroPad = 0;
            int lengthLong = 0;

            format++;

            if (*format == '0') {
                zeroPad = 1;
                format++;
            }

            while ((*format >= '0') && (*format <= '9')) {
                width = ( width * 10U ) + (unsigned int) (*format - '0');
                format++;
            }

            if (*format == 'l') {
                lengthLong = 1;
                format++;
            }

            switch (*format) {
                case 's':
                    print_str(va_arg(ap, char*));
                    break;
                case 'd':
                    if (lengthLong != 0) {
                        print_long(va_arg(ap, long));
                    }
                    else {
                        print_int(va_arg(ap, int));
                    }
                    break;
                case 'u':
                    if (lengthLong != 0) {
                        print_ulong(va_arg(ap, unsigned long));
                    }
                    else {
                        print_ulong((unsigned long) va_arg(ap, unsigned int));
                    }
                    break;
                case 'x':
                    print_hex(va_arg(ap, unsigned int), width, zeroPad, 0, 1);
                    break;
                case 'X':
                    print_hex(va_arg(ap, unsigned int), width, zeroPad, 1, 0);
                    break;
                case 'p':
                    print_hex((unsigned int) (uintptr_t) va_arg(ap, void *), (unsigned int) ( sizeof(uintptr_t) * 2U ), 1, 0, 1);
                    break;
                case '%':
                    write(1, "%", 1);
                    break;
                default:
                    write(1, "%", 1);
                    write(1, format, 1);
            }
        } else {
            write(1, format, 1);
        }

        written++;
        format++;
    }

    va_end(ap);

    return written;
}
