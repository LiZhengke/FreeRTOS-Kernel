#ifndef __ASM_MACROS_H__
#define __ASM_MACROS_H__

/* -----------------------------
 * 段寄存器
 * ----------------------------- */
#define KERNEL_CS 0x08
#define KERNEL_DS 0x10

/* -----------------------------
 * 保存通用寄存器
 * 适用于 486 (无 SSE)
 * ----------------------------- */
#define PUSH_GPRS()        \
    pushl %eax;            \
    pushl %ebx;            \
    pushl %ecx;            \
    pushl %edx;            \
    pushl %esi;            \
    pushl %edi;            \
    pushl %ebp

#define POP_GPRS()         \
    popl %ebp;             \
    popl %edi;             \
    popl %esi;             \
    popl %edx;             \
    popl %ecx;             \
    popl %ebx;             \
    popl %eax

/* -----------------------------
 * 保存段寄存器
 * 486 上必须显式保存
 * ----------------------------- */
#define PUSH_SEGS()        \
    pushl %ds;             \
    pushl %es;             \
    pushl %fs;             \
    pushl %gs

#define POP_SEGS()         \
    popl %gs;              \
    popl %fs;              \
    popl %es;              \
    popl %ds

/* -----------------------------
 * 完整上下文
 * ----------------------------- */
#define SAVE_CONTEXT()     \
    PUSH_SEGS();           \
    PUSH_GPRS()

#define RESTORE_CONTEXT()  \
    POP_GPRS();            \
    POP_SEGS()

#endif
