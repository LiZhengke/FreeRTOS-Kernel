#include <stdlib.h>
#include <string.h>
#include "FreeRTOS.h"
#include "utask.h"

#define USER_CS 0x1B
#define USER_DS 0x23

static int global_task_id = 1;

int uTaskCreate(uTask *task,
                const char *name,
                void (*entry)(void),
                int priority)
{
    (void)name; /* Name is currently unused. */
    memset(task, 0, sizeof(uTask));

    task->id = global_task_id++;
    /* strncpy(task->name, name, UTASK_NAME_LEN - 1); */

    task->priority = priority;
    task->state    = UTASK_READY;

    /* Allocate user stack */
    task->stack_size = UTASK_STACK_SIZE;
    task->stack_base = NULL;

    if (!task->stack_base)
        return -1;

    /* Set stack canary */
    *(uint32_t*)task->stack_base = 0xDEADBEEF;

    uint32_t stack_top =
        (uint32_t)(task->stack_base + task->stack_size);

    /* ---- Construct iret stack frame ---- */

    stack_top -= sizeof(uint32_t);
    *(uint32_t*)stack_top = USER_DS;         // SS

    stack_top -= sizeof(uint32_t);
    *(uint32_t*)stack_top = stack_top;       // ESP

    stack_top -= sizeof(uint32_t);
    *(uint32_t*)stack_top = 0x202;           // EFLAGS (IF=1)

    stack_top -= sizeof(uint32_t);
    *(uint32_t*)stack_top = USER_CS;         // CS

    stack_top -= sizeof(uint32_t);
    *(uint32_t*)stack_top = (uint32_t)entry; // EIP

    /* Initialize context */

    task->context.eip      = (uint32_t)entry;
    task->context.cs       = USER_CS;
    task->context.eflags   = 0x202;
    task->context.useresp  = stack_top;
    task->context.ss       = USER_DS;

    task->context.eax = 0;
    task->context.ebx = 0;
    task->context.ecx = 0;
    task->context.edx = 0;
    task->context.esi = 0;
    task->context.edi = 0;
    task->context.ebp = 0;

    task->magic = 0xCAFEBABE;

    return 0;
}
