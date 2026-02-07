.global _start
.extern __stack_top

.align 4
.section .multiboot
    .long 0x1BADB002
    .long 0x00
    .long -(0x1BADB002 + 0x00)

.align 4
.section .text
_start:
    cli
    mov $__stack_top, %esp
    mov $__stack_top, %ebp

    call main

halt:
    hlt
    jmp halt

.section .note.GNU-stack,"",@progbits
/*-----------------------------------------------------------*/
