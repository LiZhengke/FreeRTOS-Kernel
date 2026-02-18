#ifndef UTASK_H
#define UTASK_H

#include <stdint.h>

#define UTASK_STACK_SIZE 512 // Stack size for each user-mode task (words)
#define UTASK_NAME_LEN   16

typedef enum {
    UTASK_READY,
    UTASK_RUNNING,
    UTASK_BLOCKED,
    UTASK_ZOMBIE
} utask_state_t;

/* Save complete context (aligned with trapframe) */
typedef struct uContext {
    /* General purpose registers */
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t esp;        // User-mode ESP
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;

    /* Interrupt auto-pushed stack part */
    uint32_t eip;
    uint32_t cs;
    uint32_t eflags;
    uint32_t useresp;
    uint32_t ss;

} uContext;

/* User-mode task control block */
typedef struct uTask {

    int                 id;
    char                name[UTASK_NAME_LEN];

    utask_state_t       state;
    int                 priority;
    int                 time_slice;

    uint8_t            *stack_base;   // Stack base
    uint32_t            stack_size;
    uint32_t            stack_top;    // Stack top (used during initialization)

    uContext            context;      // Saved registers

    /* Scheduling linked list */
    struct uTask       *next;
    struct uTask       *prev;

    /* Verification fields */
    uint32_t            stack_canary;
    uint32_t            magic;

} uTask;

/***************** User-mode task interface *****************/
int uTaskCreate(uTask *task,
                const char *name,
                void (*entry)(void),
                int priority);
int32_t uSysYield(void);
void uSysExit(void);
int32_t uSysDelay(int ticks);
void uSysDump(uTask *task);
int32_t uSysPutChar(char ch); // Write a character to user-mode task's standard output
int32_t uSysPrintf(const char *fmt, ...); // Format output to user-mode task's standard output
#endif
