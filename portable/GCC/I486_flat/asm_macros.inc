
/* The actual exception handlers will be generated using these macros below. Each will push the appropriate arguments and jump to the common handler stub. */
.macro STOR_SEGS_ON_STACK
    xor %eax,%eax      # Clear EAX (segment regs are 16-bit, zero-extend to 32-bit) \
    mov %ds, %ax       # Load DS into lower 16 bits of EAX \
    pushl %eax         # Push DS \
    mov %es, %ax       # Load ES into lower 16 bits of EAX \
    pushl %eax         # Push ES \
    mov %fs, %ax       # Load FS into lower 16 bits of EAX \
    pushl %eax         # Push FS \
    mov %gs, %ax       # Load GS into lower 16 bits of EAX \
    pushl %eax         # Push GS
.endm

/* The common exception handler stub that all ISRs jump to after pushing their specific arguments. */
.macro RESTORE_SEGS_FROM_STACK
    popl %eax          # Pop GS
    mov %ax, %gs       # Restore GS
    popl %eax          # Pop FS
    mov %ax, %fs       # Restore FS
    popl %eax          # Pop ES
    mov %ax, %es       # Restore ES
    popl %eax          # Pop DS
    mov %ax, %ds       # Restore DS
.endm
