.global _start
.extern __stack_top

_start:
    cli
    mov $__stack_top, %esp
    mov $__stack_top, %ebp

    call main

halt:
    hlt
    jmp halt
