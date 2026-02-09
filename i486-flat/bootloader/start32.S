/* =========================================================
 * Protected mode entry
 * ========================================================= */
.code32
.section .text.start, "ax"
.globl _start
.type _start, @function
_start:
    movb $0x22, %al
    outb %al, $0x80
    # Setup data segment registers
    movw $0x10, %ax      # 0x10 is the data segment in GDT
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %ss
    movw %ax, %fs
    movw %ax, %gs

    # Finally we can go to C language
    movl $0x90000, %esp
    call kmain

    # jump to kernel
    mov $0x10000c, %eax
    jmp *%eax

pm_hang:
    hlt
    jmp pm_hang
