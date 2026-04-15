.code32
.section .text.entry, "ax"
.global _start
.type _start, @function

_start:
    call main
    /* If main returns, loop forever */
    mov $0, %ebx
    mov $14, %eax   # sys_exit
    int $0x30
.hang:
    jmp .hang
