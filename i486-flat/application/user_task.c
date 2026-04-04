/*
 * user_task.c — Ring 3 user-mode task
 *
 * This is compiled as a freestanding binary (no libc), embedded into
 * the kernel image, and executed in user space via the MMU.
 * Syscalls use "int $0x30" (portSYSCALL_INT_VECTOR).
 */

#include <stdint.h>
#include <stdarg.h>
#include "io.h"

/* Syscall numbers — must match the kernel enum in syscall.h */
#define SYS_WRITE            1
#define SYS_DELAY           2
#define SYS_PRINTF          8
#define SYS_TICK_COUNT      11
#define SYS_GET_TASK_NAME   12
#define SYSINT              0x30

#define _STR(x)  #x
#define STR(x)   _STR(x)

/* ------------------------------------------------------------------ */
/* Minimal syscall wrappers                                            */
/* ------------------------------------------------------------------ */
static int32_t sys_write(int fd, const void *str, int len)
{
    int32_t ret;

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

static int32_t sys_printf(const char *fmt, ...)
{
    int32_t ret;
    va_list args;
    va_start(args, fmt);

    __asm__ volatile (
        "int $" STR(SYSINT)
        : "=a"(ret)
        : "a"(SYS_PRINTF),
          "b"(fmt),
          "c"(&args)
        : "memory"
    );

    va_end(args);
    return ret;
}

static int32_t sys_get_task_name(char *buf, uint32_t len)
{
    int32_t ret;

    __asm__ volatile (
        "int $" STR(SYSINT)
        : "=a"(ret)
        : "a"(SYS_GET_TASK_NAME),
          "b"(buf),
          "c"(len)
        : "memory"
    );

    return ret;
}

static int32_t sys_delay(uint32_t ticks)
{
    int32_t ret;

    __asm__ volatile (
        "int $" STR(SYSINT)
        : "=a"(ret)
        : "a"(SYS_DELAY),
          "b"(ticks)
        : "memory"
    );

    return ret;
}

static int32_t sys_get_tick_count(void)
{
    int32_t ret;

    __asm__ volatile (
        "int $" STR(SYSINT)
        : "=a"(ret)
        : "a"(SYS_TICK_COUNT)
        : "memory"
    );

    return ret;
}

static inline uint16_t get_cpl(void)
{
    uint16_t cs;
    __asm__ volatile ("mov %%cs, %0" : "=r"(cs));
    return cs & 0x3;
}

/* ------------------------------------------------------------------ */
/* Entry point                                                         */
/* ------------------------------------------------------------------ */
void main(void)
{
    char name[16];

    sys_get_task_name(name, sizeof(name));
    puts("Main user task.\n");
    printf("format probe name=%p cpl=%04X\n", (void *) name, (unsigned int) get_cpl());

    for (;;) {
        printf("[%s] tick=%lu cpl=%d\n", name, (unsigned long) sys_get_tick_count(), get_cpl());
        sys_delay(100);
    }
}
