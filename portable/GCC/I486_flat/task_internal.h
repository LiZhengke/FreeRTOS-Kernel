#ifndef PORT_INTERNAL_H
#define PORT_INTERNAL_H
#include <stdint.h>
typedef enum {
    TASK_PROCESS,
    TASK_THREAD
} TaskType_t;

typedef struct {
    TaskType_t tsk_type;             /* 任务类型：进程或线程 */
    uint8_t xUserPrivilegeLevel;     /* 用户态特权级 (0-3, typically 3 for user tasks) */

     /* 1. 资源需求 */
     /* 2. 执行入口 */
     /* 3. 运行资源 */
    void* (*user_entry)(void*); /* 用户态函数指针 (start_routine) */
    void* user_arg;             /* 传递给用户函数的参数 (arg) */

    uint32_t user_stack_top;    /* 用户态栈顶指针 (ESP) */
    uint32_t brk;              /* 进程数据段末尾 (用于 sbrk 内存扩展) */

     /* 4. 管理信息 (可选) */
     /* 进程/线程管理相关的其他信息可以放在这里，例如： */
     /* - 进程/线程状态（就绪、运行、阻塞等） */
     /* - 进程/线程优先级 */
     /* - 进程/线程 ID */
     /* - 父子关系（对于进程） */
     /* - 信号处理函数指针（对于进程） */
    uint32_t thread_id;         /* 内部线程 ID */
} TaskArgs_t;

#endif /* PORT_INTERNAL_H */
