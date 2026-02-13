#include <stdint.h>
#include "os_helper.h"
#include "syscall.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

// Forward declaration
void putchar(char c);

syscall_t syscall_table[SYS_MAX] = {
    sys_yield,
    sys_delay,
    sys_exit,
    sys_time_get,
    sys_sem_pend,
    sys_sem_post,
    sys_putc,
    sys_panic,
    sys_task_create
};

int syscall_dispatch(void)
{
    uint32_t num, a0, a1, a2, a3, a4;

    asm volatile(
        "movl 36(%%ebp), %0\n"  // EAX saved by pushal
        "movl 24(%%ebp), %1\n"  // EBX
        "movl 32(%%ebp), %2\n"  // ECX
        "movl 28(%%ebp), %3\n"  // EDX
        "movl 12(%%ebp), %4\n"  // ESI
        "movl 8(%%ebp), %5\n"   // EDI
        : "=a"(num), "=b"(a0), "=c"(a1),
          "=d"(a2), "=S"(a3), "=D"(a4)
    );

    if (num >= SYS_MAX)
        return -OS_ERR_INVALID;

    return syscall_table[num](a0,a1,a2,a3,a4);
}

int os_err_to_errno(OS_ERR err)
{
    switch (err) {
    case OS_ERR_INVALID:
        return -EINVAL;
    case OS_ERR_PERM:
        return -EPERM;
    default:
        return -EINVAL;
    }
}

int sys_yield(uint32_t a0,uint32_t a1,uint32_t a2,uint32_t a3,uint32_t a4)
{
    (void)a0; (void)a1; (void)a2; (void)a3; (void)a4;
    taskYIELD();
    return 0;
}

int sys_delay(uint32_t a0,uint32_t a1,uint32_t a2,uint32_t a3,uint32_t a4)
{
    (void)a1; (void)a2; (void)a3; (void)a4;
    uint16_t ticks = a0;

    vTaskDelay(ticks);
    return 0;
}

int sys_exit(uint32_t a0,uint32_t a1,uint32_t a2,uint32_t a3,uint32_t a4)
{
    (void)a0; (void)a1; (void)a2; (void)a3; (void)a4;
    vTaskDelete(NULL);
    return 0;
}

int sys_time_get(uint32_t a0,uint32_t a1,uint32_t a2,uint32_t a3,uint32_t a4)
{
    (void)a1; (void)a2; (void)a3; (void)a4;
    uint32_t *ticks = (uint32_t *)a0;
    *ticks = xTaskGetTickCount();
    return 0;
}

int sys_sem_pend(uint32_t a0,uint32_t a1,uint32_t a2,uint32_t a3,uint32_t a4)
{
    (void)a0; (void)a1; (void)a2; (void)a3; (void)a4;
    // TODO: Implement semaphore pend using FreeRTOS APIs
    return -EINVAL;
}

int sys_sem_post(uint32_t a0,uint32_t a1,uint32_t a2,uint32_t a3,uint32_t a4)
{
    (void)a0; (void)a1; (void)a2; (void)a3; (void)a4;
    // TODO: Implement semaphore post using FreeRTOS APIs
    return -EINVAL;
}

int sys_panic(uint32_t a0,uint32_t a1,uint32_t a2,uint32_t a3,uint32_t a4)
{
    (void)a0; (void)a1; (void)a2; (void)a3; (void)a4;
    // TODO: Implement panic handler
    while(1);
    return 0;
}

int sys_putc(uint32_t a0,uint32_t a1,uint32_t a2,uint32_t a3,uint32_t a4)
{
    (void)a1; (void)a2; (void)a3; (void)a4;
    char ch = a0;
    putchar((char)ch);
    return 0;
}

int sys_task_create(uint32_t a0,uint32_t a1,uint32_t a2,uint32_t a3,uint32_t a4)
{
    (void)a0; (void)a1; (void)a2; (void)a3; (void)a4;
    // TODO: Implement task creation using FreeRTOS xTaskCreate
    static StaticTask_t taskTCB;
    static StackType_t taskKernelStack[ configMINIMAL_STACK_SIZE ];

    TaskFunction_t taskFunction = (TaskFunction_t)a0;
    const char *taskName = (const char *)a1;
    uint16_t stackDepth = (uint16_t)a2;
    UBaseType_t priority = (UBaseType_t)a3;
    // a4 is reserved for future use
    TaskHandle_t handle = xTaskCreateStatic( taskFunction,
                                taskName,
                                stackDepth,
                                NULL,
                                priority,
                                &( taskKernelStack[ 0 ] ),
                                &( taskTCB ) );
    return handle ? 0 : -EINVAL;
}
