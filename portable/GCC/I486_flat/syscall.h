#ifndef SYSCALL_H
#define SYSCALL_H

#include <stdint.h>
#include "os_helper.h"
enum {
    SYS_YIELD = 0,
    SYS_DELAY,
    SYS_EXIT,
    SYS_TIME_GET,
    SYS_SEM_PEND,
    SYS_SEM_POST,
    SYS_PUTC,
    SYS_PANIC,
    SYS_TASK_CREATE,
    SYS_MAX
};

int sys_yield(uint32_t a0,uint32_t a1,uint32_t a2,uint32_t a3,uint32_t a4);
int sys_delay(uint32_t a0,uint32_t a1,uint32_t a2,uint32_t a3,uint32_t a4);
int sys_exit(uint32_t a0,uint32_t a1,uint32_t a2,uint32_t a3,uint32_t a4);
int sys_time_get(uint32_t a0,uint32_t a1,uint32_t a2,uint32_t a3,uint32_t a4);
int sys_sem_pend(uint32_t a0,uint32_t a1,uint32_t a2,uint32_t a3,uint32_t a4);
int sys_sem_post(uint32_t a0,uint32_t a1,uint32_t a2,uint32_t a3,uint32_t a4);
int sys_putc(uint32_t a0,uint32_t a1,uint32_t a2,uint32_t a3,uint32_t a4);
int sys_panic(uint32_t a0,uint32_t a1,uint32_t a2,uint32_t a3,uint32_t a4);
int sys_task_create(uint32_t a0,uint32_t a1,uint32_t a2,uint32_t a3,uint32_t a4);

typedef int (*syscall_t)(uint32_t, uint32_t,
                         uint32_t, uint32_t, uint32_t);
extern syscall_t syscall_table[SYS_MAX];

int syscall_dispatch(void);
int os_err_to_errno(OS_ERR err);

#endif /* SYSCALL_H */
