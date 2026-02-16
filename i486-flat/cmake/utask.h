#ifndef UTASK_H
#define UTASK_H

#include <stdint.h>

#define UTASK_STACK_SIZE 512 // 每个用户态任务的栈大小（字）
#define UTASK_NAME_LEN   16

typedef enum {
    UTASK_READY,
    UTASK_RUNNING,
    UTASK_BLOCKED,
    UTASK_ZOMBIE
} utask_state_t;

/* 保存完整上下文（与 trapframe 对齐） */
typedef struct uContext {
    /* 通用寄存器 */
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t esp;        // 用户态 ESP
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;

    /* 中断自动压栈部分 */
    uint32_t eip;
    uint32_t cs;
    uint32_t eflags;
    uint32_t useresp;
    uint32_t ss;

} uContext;

/* 用户态任务控制块 */
typedef struct uTask {

    int                 id;
    char                name[UTASK_NAME_LEN];

    utask_state_t       state;
    int                 priority;
    int                 time_slice;

    uint8_t            *stack_base;   // 栈底
    uint32_t            stack_size;
    uint32_t            stack_top;    // 栈顶（初始化时使用）

    uContext            context;      // 保存寄存器

    /* 调度链表 */
    struct uTask       *next;
    struct uTask       *prev;

    /* 验证字段 */
    uint32_t            stack_canary;
    uint32_t            magic;

} uTask;

/***************** 用户态任务接口 *****************/
int uTaskCreate(uTask *task,
                const char *name,
                void (*entry)(void),
                int priority);
int32_t uSysYield(void);
void uSysExit(void);
int32_t uSysDelay(int ticks);
void uSysDump(uTask *task);
int32_t uSysPutChar(char ch); // 向用户态任务的标准输出写入一个字符
#endif
