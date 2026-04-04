#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "os_helper.h"
#include "syscall.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "tss.h"
#include "fs/ramfs/ramfs.h"
// Forward declaration
extern file_t fd_table[];
extern int printf(const char *__restrict __format, ...);
extern int printf_va(const char *__restrict __format, va_list *__ap);
typedef int (*syscall_t)(uint32_t, uint32_t,
                         uint32_t, uint32_t, uint32_t);
/* syscall functions interface */
static int sys_yield(uint32_t a0,uint32_t a1,uint32_t a2,uint32_t a3,uint32_t a4);
static int sys_write(uint32_t a0,uint32_t a1,uint32_t a2,uint32_t a3,uint32_t a4);
static int sys_delay(uint32_t a0,uint32_t a1,uint32_t a2,uint32_t a3,uint32_t a4);
static int sys_exit(uint32_t a0,uint32_t a1,uint32_t a2,uint32_t a3,uint32_t a4);
static int sys_time_get(uint32_t a0,uint32_t a1,uint32_t a2,uint32_t a3,uint32_t a4);
static int sys_sem_pend(uint32_t a0,uint32_t a1,uint32_t a2,uint32_t a3,uint32_t a4);
static int sys_sem_post(uint32_t a0,uint32_t a1,uint32_t a2,uint32_t a3,uint32_t a4);
static int sys_putc(uint32_t a0,uint32_t a1,uint32_t a2,uint32_t a3,uint32_t a4);
static int sys_printf(uint32_t a0,uint32_t a1,uint32_t a2,uint32_t a3,uint32_t a4);
static int sys_panic(uint32_t a0,uint32_t a1,uint32_t a2,uint32_t a3,uint32_t a4);
static int sys_task_create(uint32_t a0,uint32_t a1,uint32_t a2,uint32_t a3,uint32_t a4);
static int sys_tick_count(uint32_t a0,uint32_t a1,uint32_t a2,uint32_t a3,uint32_t a4);
static int sys_get_task_name(uint32_t a0,uint32_t a1,uint32_t a2,uint32_t a3,uint32_t a4);
static int sys_open(uint32_t a0,uint32_t a1,uint32_t a2,uint32_t a3,uint32_t a4);
static int sys_read(uint32_t a0,uint32_t a1,uint32_t a2,uint32_t a3,uint32_t a4);

syscall_t syscall_table[SYS_MAX] = {
    sys_yield,
    sys_write,
    sys_delay,
    sys_exit,
    sys_time_get,
    sys_sem_pend,
    sys_sem_post,
    sys_putc,
    sys_printf,
    sys_panic,
    sys_task_create,
    sys_tick_count,
    sys_get_task_name,
    sys_open,
    sys_read,
};


int uSysCallDispatch(void)
{
    uint32_t num, a0, a1, a2, a3, a4;

    asm volatile(
        "movl 52(%%ebp), %0\n"  // EAX saved by pushal
        "movl 40(%%ebp), %1\n"  // EBX
        "movl 48(%%ebp), %2\n"  // ECX
        "movl 44(%%ebp), %3\n"  // EDX
        "movl 28(%%ebp), %4\n"  // ESI
        "movl 24(%%ebp), %5\n"   // EDI
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

static int sys_yield(uint32_t a0,uint32_t a1,uint32_t a2,uint32_t a3,uint32_t a4)
{
    (void)a0; (void)a1; (void)a2; (void)a3; (void)a4;
    taskYIELD();
    return 0;
}

static int sys_write(uint32_t a0,uint32_t a1,uint32_t a2,uint32_t a3,uint32_t a4)
{
    (void)a3; (void)a4;
    int fd = a0;
    if (fd != 1 && fd != 2) // Only support stdout and stderr for now
        return -EINVAL;
    const char *str = (const char *)a1;
    if (str == NULL)
        return -EINVAL;
    int len = a2;
    if (len == 0)        return 0;
    if (len < 0)         return -EINVAL;
    for(int i = 0; i < len; i++) {
        putchar(str[i]);
        if (str[i] == '\n') // Convert newline to carriage return + newline
            putchar('\r');
    }
    return len;
}
static int sys_delay(uint32_t a0,uint32_t a1,uint32_t a2,uint32_t a3,uint32_t a4)
{
    (void)a1; (void)a2; (void)a3; (void)a4;
    uint16_t ticks = a0;

    vTaskDelay(ticks);
    return 0;
}

static int sys_exit(uint32_t a0,uint32_t a1,uint32_t a2,uint32_t a3,uint32_t a4)
{
    (void)a0; (void)a1; (void)a2; (void)a3; (void)a4;
    vTaskDelete(NULL);
    return 0;
}

static int sys_time_get(uint32_t a0,uint32_t a1,uint32_t a2,uint32_t a3,uint32_t a4)
{
    (void)a1; (void)a2; (void)a3; (void)a4;
    uint32_t *ticks = (uint32_t *)a0;
    *ticks = xTaskGetTickCount();
    return 0;
}

static int sys_sem_pend(uint32_t a0,uint32_t a1,uint32_t a2,uint32_t a3,uint32_t a4)
{
    (void)a0; (void)a1; (void)a2; (void)a3; (void)a4;
    // TODO: Implement semaphore pend using FreeRTOS APIs
    return -EINVAL;
}

static int sys_sem_post(uint32_t a0,uint32_t a1,uint32_t a2,uint32_t a3,uint32_t a4)
{
    (void)a0; (void)a1; (void)a2; (void)a3; (void)a4;
    // TODO: Implement semaphore post using FreeRTOS APIs
    return -EINVAL;
}

static int sys_panic(uint32_t a0,uint32_t a1,uint32_t a2,uint32_t a3,uint32_t a4)
{
    (void)a0; (void)a1; (void)a2; (void)a3; (void)a4;
    // TODO: Implement panic handler
    while(1);
    return 0;
}

static int sys_putc(uint32_t a0,uint32_t a1,uint32_t a2,uint32_t a3,uint32_t a4)
{
    (void)a1; (void)a2; (void)a3; (void)a4;
    char ch = a0;
    putchar((char)ch);
    return 0;
}

static int sys_printf(uint32_t a0,uint32_t a1,uint32_t a2,uint32_t a3,uint32_t a4)
{
    (void)a2; (void)a3; (void)a4;
    const char *fmt = (const char *)a0;
    va_list *args = (va_list *)a1;
    if (fmt == NULL || args == NULL)
        return -EINVAL;
    return printf_va(fmt, args);
}

static int sys_task_create(uint32_t a0,uint32_t a1,uint32_t a2,uint32_t a3,uint32_t a4)
{
    TaskFunction_t taskFunction = (TaskFunction_t)a0;
    const char *taskName = (const char *)a1;
    configSTACK_DEPTH_TYPE stackDepth = (configSTACK_DEPTH_TYPE)a2;
    UBaseType_t priority = (UBaseType_t)a3;
    void *pvParameters = (void *)a4;

    if (taskFunction == NULL){
        printf("sys_task_create: taskFunction is NULL\n");
        return -EINVAL;
    }

    if (priority >= configMAX_PRIORITIES) {
        printf("sys_task_create: invalid priority %u\n", priority);
        return -EINVAL;
    }

    if (stackDepth == 0)
        stackDepth = configMINIMAL_STACK_SIZE;

    /* Dynamically allocate TCB and stacks so multiple tasks can be created. */
    StaticTask_t *pxTCB = (StaticTask_t *)pvPortMalloc(sizeof(StaticTask_t));
    StackType_t *pxKernelStack = (StackType_t *)pvPortMalloc(stackDepth * sizeof(StackType_t));
    StackType_t *pxUserStack = (StackType_t *)pvPortMalloc(stackDepth * sizeof(StackType_t));

    if (pxTCB == NULL || pxKernelStack == NULL || pxUserStack == NULL) {
        printf("Failed to allocate memory for task creation\n");
        printf("pxTCB=%p pxKernelStack=%p pxUserStack=%p\n", (void *)pxTCB, (void *)pxKernelStack, (void *)pxUserStack);
        if (pxTCB) vPortFree(pxTCB);
        if (pxKernelStack) vPortFree(pxKernelStack);
        if (pxUserStack) vPortFree(pxUserStack);
        return -EINVAL;
    }

    TaskHandle_t handle = xTaskCreateStatic( taskFunction,
                                taskName,
                                stackDepth,
                                pvParameters,
                                priority,
                                pxKernelStack,
                                cpuPRIVILEGE_LEVEL_3,
                                pxTCB );

    if (handle == NULL) {
        printf("Failed to create task\n");
        vPortFree(pxTCB);
        vPortFree(pxKernelStack);
        vPortFree(pxUserStack);
        return -EINVAL;
    }

    printf("Task '%s' created successfully with priority %u\n", taskName, priority);
    return 0;
}

static int sys_tick_count(uint32_t a0,uint32_t a1,uint32_t a2,uint32_t a3,uint32_t a4)
{
    (void)a0; (void)a1; (void)a2; (void)a3; (void)a4;
    return (int)xTaskGetTickCount();
}

static int sys_get_task_name(uint32_t a0,uint32_t a1,uint32_t a2,uint32_t a3,uint32_t a4)
{
    (void)a2; (void)a3; (void)a4;
    char *buf = (char *)a0;
    uint32_t len = a1;

    if (buf == NULL || len == 0)
        return -EINVAL;

    const char *name = pcTaskGetName(NULL);
    uint32_t i;
    for (i = 0; i < len - 1 && name[i] != '\0'; i++)
        buf[i] = name[i];
    buf[i] = '\0';
    return 0;
}

static int sys_open(uint32_t a0,uint32_t a1,uint32_t a2,uint32_t a3,uint32_t a4)
{
    (void)a1; (void)a2; (void)a3; (void)a4;
    const char *path = (const char *)a0;

    node_t *n = lookup(path);

    if (!n || n->type != NODE_FILE)
        return -1;

    for (int i = 0; i < 32; i++) {
        if (fd_table[i].node == NULL) {
            fd_table[i].node = n;
            fd_table[i].offset = 0;
            return i;
        }
    }

    return -1;
}

static int sys_read(uint32_t a0,uint32_t a1,uint32_t a2,uint32_t a3,uint32_t a4)
{
    int fd = (int)a0;
    void *buf = (void *)a1;
    int len = (int)a2;
     (void)a3; (void)a4;

    if (fd < 0 || fd >= 32)
        return -1;

    file_t *f = &fd_table[fd];

    if (!f->node)
        return -1;

    int remain = f->node->size - f->offset;

    if (remain <= 0)
        return 0;  // EOF

    if (len > remain)
        len = remain;

    memcpy(buf,
           f->node->data + f->offset,
           len);

    f->offset += len;

    return len;
}
/*--------------------------------------------------------------------- */
/* User-space syscall wrappers. These functions can be called by user tasks to
 * invoke system calls.
 *--------------------------------------------------------------------- */
int32_t uSysPutChar(char c)
{
    int32_t ret;
    asm volatile (
        "int $" STR(SYSINT)
        : "=a"(ret)
        : "a"(SYS_PUTC),   // eax: syscall number
          "b"(c)           // ebx: argument
        : "memory"
    );
    return ret;
}

int32_t uSysDelay(uint16_t ticks)
{
    int32_t ret;
    asm volatile (
        "int $" STR(SYSINT)
        : "=a"(ret)
        : "a"(SYS_DELAY),   // eax: syscall number
          "b"(ticks)        // ebx: argument
        : "memory"
    );
    return ret;
}

int32_t uSysTaskCreate(void (*taskFunction)(void *), const char *taskName,
                       uint16_t stackDepth, uint32_t priority,
                       void *pvParameters)
{
    int32_t ret;
    asm volatile (
        "int $" STR(SYSINT)
        : "=a"(ret)
        : "a"(SYS_TASK_CREATE),
          "b"(taskFunction),
          "c"(taskName),
          "d"((uint32_t)stackDepth),
          "S"(priority),
          "D"(pvParameters)
        : "memory"
    );
    return ret;
}

int32_t uSysGetTickCount(void)
{
    int32_t ret;
    asm volatile (
        "int $" STR(SYSINT)
        : "=a"(ret)
        : "a"(SYS_TICK_COUNT)
        : "memory"
    );
    return ret;
}

int32_t uSysPrintf(const char *fmt, ...)
{
    int32_t ret;
    va_list args;
    va_start(args, fmt);

    asm volatile (
        "int $" STR(SYSINT)
        : "=a"(ret)
        : "a"(SYS_PRINTF),    // eax: syscall number
          "b"(fmt),           // ebx: format string pointer
          "c"(&args)          // ecx: pointer to va_list
        : "memory"
    );

    va_end(args);
    return ret;
}

int32_t uSysGetTaskName(char *buf, uint32_t len)
{
    int32_t ret;
    asm volatile (
        "int $" STR(SYSINT)
        : "=a"(ret)
        : "a"(SYS_GET_TASK_NAME),
          "b"(buf),
          "c"(len)
        : "memory"
    );
    return ret;
}
