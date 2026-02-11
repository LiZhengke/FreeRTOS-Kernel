#include <stdarg.h>
extern void putchar(char c);

int puts(const char *__s)
{
    while (*__s)
        putchar(*__s++);
    putchar('\n');
    return 0;
}
static void print_uint(unsigned int value, unsigned int base)
{
    char buf[32];
    int i = 0;

    if (value == 0) {
        putchar('0');
        return;
    }

    while (value) {
        unsigned int digit = value % base;
        buf[i++] = (digit < 10) ? ('0' + digit) : ('a' + digit - 10);
        value /= base;
    }

    while (i--) {
        putchar(buf[i]);
    }
}

static void print_int(int value)
{
    if (value < 0) {
        putchar('-');
        print_uint((unsigned int)(-value), 10);
    } else {
        print_uint((unsigned int)value, 10);
    }
}

static void print_ptr(const void *ptr)
{
    unsigned int v = (unsigned int)ptr;
    putchar('0');
    putchar('x');

    for (int i = 7; i >= 0; i--) {
        unsigned int nibble = (v >> (i * 4)) & 0xF;
        putchar(nibble < 10 ? '0' + nibble : 'a' + nibble - 10);
    }
}

static void print_dec_unsigned(unsigned long val)
{
    char buf[16];
    int i = 0;

    if (val == 0) {
        putchar('0');
        return;
    }

    while (val > 0) {
        buf[i++] = '0' + (val % 10);
        val /= 10;
    }

    while (i > 0) {
        putchar(buf[--i]);
    }
}

int printf (const char *__restrict __format, ...)
{
    va_list ap;
    va_start(ap, __format);

    for (; *__format; __format++) {
        if (*__format != '%') {
            putchar(*__format);
            continue;
        }

        __format++;  // skip %

        // Check for 'l' length modifier
        int is_long = 0;
        if (*__format == 'l') {
            is_long = 1;
            __format++;
        }

        switch (*__format) {
        case 'c':
            putchar((char)va_arg(ap, int));
            break;

        case 's': {
            const char *s = va_arg(ap, const char *);
            if (!s) s = "(null)";
            while (*s) putchar(*s++);
            break;
        }

        case 'd':
            print_int(va_arg(ap, int));
            break;

        case 'u':
            if (is_long) {
                print_dec_unsigned(va_arg(ap, unsigned long));
            } else {
                print_uint(va_arg(ap, unsigned int), 10);
            }
            break;

        case 'x':
            print_uint(va_arg(ap, unsigned int), 16);
            break;

        case 'p':
            print_ptr(va_arg(ap, void *));
            break;

        case '%':
            putchar('%');
            break;

        default:
            putchar('%');
            if (is_long) putchar('l');
            putchar(*__format);
            break;
        }
    }

    va_end(ap);
    return 0;
}
