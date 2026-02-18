.extern kmain
.section .text.start
.code32
.globl _start
.type _start, @function

_start:
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

.section .text
.code32
# Macro 1: Handle exceptions where hardware doesn't generate error code (e.g., divide by zero)
.macro ISR_NOERR vector
.global exc\vector
exc\vector:
    pushl $0           # Push dummy error code
    pushl $\vector       # Push exception vector number
    jmp common_stub
.endm

# Macro 2: Handle exceptions where hardware automatically pushes error code (e.g., GPF)
.macro ISR_ERR vector
.global exc\vector
exc\vector:
    # Error code is already on the stack top at this point
    pushl $\vector       # Push exception vector number
    jmp common_stub
.endm

# --- Use macros to generate common exception entry points ---
/* 0–7 */
ISR_NOERR 0    /* #DE  Divide Error */
ISR_NOERR 1    /* #DB  Debug */
ISR_NOERR 2    /* NMI */
ISR_NOERR 3    /* #BP  Breakpoint */
ISR_NOERR 4    /* #OF  Overflow */
ISR_NOERR 5    /* #BR  BOUND Range Exceeded */
ISR_NOERR 6    /* #UD  Invalid Opcode */
ISR_NOERR 7    /* #NM  Device Not Available */

/* 8–15 */
ISR_ERR   8    /* #DF  Double Fault */
ISR_NOERR 9    /* Coprocessor Segment Overrun (reserved) */
ISR_ERR   10   /* #TS  Invalid TSS */
ISR_ERR   11   /* #NP  Segment Not Present */
ISR_ERR   12   /* #SS  Stack-Segment Fault */
ISR_ERR   13   /* #GP  General Protection Fault */
ISR_ERR   14   /* #PF  Page Fault */
ISR_NOERR 15   /* Reserved */

/* 16–31 */
ISR_NOERR 16   /* #MF  x87 FPU Floating-Point Error */
ISR_ERR   17   /* #AC  Alignment Check */
ISR_NOERR 18   /* #MC  Machine Check */
ISR_NOERR 19   /* #XM  SIMD Floating-Point Exception */
ISR_NOERR 20   /* Virtualization Exception */
ISR_NOERR 21   /* Control Protection Exception (CET, newer CPU) */
ISR_NOERR 22
ISR_NOERR 23
ISR_NOERR 24
ISR_NOERR 25
ISR_NOERR 26
ISR_NOERR 27
ISR_NOERR 28
ISR_NOERR 29
ISR_NOERR 30   /* #SX  Security Exception (newer CPU) */
ISR_NOERR 31


# --- Unified handling logic ---
common_stub:
    pushal             # Save EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI
    mov %ds, %ax
    pushl %eax
    mov %es, %ax
    pushl %eax
    mov %fs, %ax
    pushl %eax
    mov %gs, %ax
    pushl %eax
    mov %ss, %ax
    pushl %eax
 /*
     * Stack layout at this point (ESP → higher address):
     *        ss
     *  0x00  gs
     *  0x04  fs
     *  0x08  es
     *  0x0C  ds
     *  0x10  eax
     *  0x14  ecx
     *  0x18  edx
     *  0x1C  ebx
     *  0x20  esp_dummy
     *  0x24  ebp
     *  0x28  esi
     *  0x2C  edi
     *  0x30  vector
     *  0x34  error_code
     *  0x38  eip
     *  0x3C  cs
     *  0x40  eflags
     */

    /* Pass regs_t * to C */
    # Stack structure at this point:
    # [esp + 36] Error code (ErrorCode)
    # [esp + 32] Vector number (Vector)
    # [esp + 0-31] Content saved by pushal

    # Call C function: void exception_handler(uint32_t vector, uint32_t err_code)
    # According to C calling convention (cdecl), parameters are pushed right to left
    # pushl 36(%esp)     # Push parameter 2: err_code
    # pushl 40(%esp)     # Push parameter 1: vector (note the offset changed because we just pushed a value)

    pushl %esp        # Parameter: regs_t *
    # extern exception_handler
    call exception_handler
    addl $4, %esp
    # Handler should usually not return. If it returns, cleanup and restore
    #addl $8, %esp

    /* Restore segment registers (reverse order) */
    popl %eax
    mov %ax, %ss
    popl %eax
    mov %ax, %gs
    popl %eax
    mov %ax, %fs
    popl %eax
    mov %ax, %es
    popl %eax
    mov %ax, %ds

    popal
    addl $8, %esp      # Clean up vector and err_code
    iret
