/*
 * FreeRTOS Kernel <DEVELOPMENT BRANCH>
 * Copyright (C) 2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * https://www.FreeRTOS.org
 * https://github.com/FreeRTOS
 *
 */

.file "portASM.S"
#include "FreeRTOSConfig.h"
#include "ISR_Support.h"
    /*.include "asm_macros.inc"*/
#include "asm_macros.h"

    .extern pxCurrentTCB
    .extern vTaskSwitchContext
    .extern vPortCentralInterruptHandler
    .extern xTaskIncrementTick
    .extern vPortAPICErrorHandler
    .extern pucPortTaskFPUContextBuffer
    .extern ulPortYieldPending
    .extern ulSysCallDispatch

    .global vPortStartFirstTask
    .global vPortCentralInterruptWrapper
    .global vPortAPICErrorHandlerWrapper
    .global vPortTimerHandler
    .global vPortYieldCall
    .global vPortAPICSpuriousHandler
    .global syscall_entry
    .global vPortSysCall
    .global gdt_flush

    .text

/*-----------------------------------------------------------*/

.align 4
.func vPortYieldCall
vPortYieldCall:
    /* Save general purpose registers. */
    pushal

    # Save segment registers (required to preserve segment state across handler)
    PUSH_SEGS()

     /*
    * Stack layout at this point (ESP → increases downward as we push):
    * After pusha and PUSH_SEGS()():
    *   +-----------------+
    *   | GS              | ← ESP + 0
    *   +-----------------+
    *   | FS              | ← ESP + 4
    *   +-----------------+
    *   | ES              | ← ESP + 8
    *   +-----------------+
    *   | DS              | ← ESP + 12
    *   +-----------------+
    *   | EDI             | ← ESP + 16
    *   +-----------------+
    *   | ESI             | ← ESP + 20
    *   +-----------------+
    *   | EBP             | ← ESP + 24
    *   +-----------------+
    *   | ESP (original)  | ← ESP + 28
    *   +-----------------+
    *   | EBX             | ← ESP + 32
    *   +-----------------+
    *   | EDX             | ← ESP + 36
    *   +-----------------+
    *   | ECX             | ← ESP + 40
    *   +-----------------+
    *   | EAX             | ← ESP + 44
    *   +-----------------+
    * Hardware-pushed interrupt frame:
    *   | EIP (return)    | ← ESP + 48
    *   +-----------------+
    *   | CS              | ← ESP + 52
    *   +-----------------+
    *   | EFLAGS          | ← ESP + 56
    *   +-----------------+
    *   | ESP (user)      | ← ESP + 60 (only if ring change)
    *   +-----------------+
    *   | SS (user)       | ← ESP + 64 (only if ring change)
    *   +-----------------+
    */

     /* Set the yield pending flag. */

    .if configSUPPORT_FPU == 1

        /* If the task has a buffer allocated to save the FPU context then save
        the FPU context now. */
        movl    pucPortTaskFPUContextBuffer, %eax
        test    %eax, %eax
        je      1f
        fnsave  ( %eax )
        fwait

        1:

        /* Save the address of the FPU context, if any. */
        push    pucPortTaskFPUContextBuffer

    .endif /* configSUPPORT_FPU */

    /* Find the TCB. */
    movl    pxCurrentTCB, %eax

.if ( configENABLE_RING3_TASKS == 1 )
    movl    8(%eax), %edx       # 8 = xPrivilegeLevel offset in TCB
    cmp     $cpuPRIVILEGE_LEVEL_3, %edx
    je      ring3_task
    /* If the task is a user mode task, then switch to the kernel stack before setting the yield pending flag. */
    movl    4(%eax), %esp       # 4 = pxKernelStackTop offset in TCB
    movl    %esp, %ebp
ring3_task:
.endif /* configENABLE_RING3_TASKS */

    /* Stack location is first item in the TCB. */
    movl    %esp, (%eax)

    call vTaskSwitchContext

    /* Find the location of pxCurrentTCB again - a callee saved register could
    be used in place of eax to prevent this second load, but that then relies
    on the compiler and other asm code. */
    movl    pxCurrentTCB, %eax
    movl    (%eax), %esp

    .if configSUPPORT_FPU == 1

        /* Restore address of task's FPU context buffer. */
        pop     pucPortTaskFPUContextBuffer

        /* If the task has a buffer allocated in which its FPU context is saved,
        then restore it now. */
        movl    pucPortTaskFPUContextBuffer, %eax
        test    %eax, %eax
        je      1f
        frstor  ( %eax )
        1:
    .endif

    # Restore segment registers (pop in reverse order of push: GS, FS, ES, DS)
    POP_SEGS()

    popal
    iret

.endfunc
/*-----------------------------------------------------------*/

.align 4
.func vPortStartFirstTask
vPortStartFirstTask:

    /* Find the TCB. */
    movl    pxCurrentTCB, %eax

    /* Stack location is first item in the TCB. */
    movl    (%eax), %esp

    /* Restore FPU context flag. */
    .if configSUPPORT_FPU == 1

        pop     pucPortTaskFPUContextBuffer

    .endif /* configSUPPORT_FPU */

    POP_SEGS()

    popal
    iret
.endfunc
/*-----------------------------------------------------------*/

.align 4
.func vPortAPICErrorHandlerWrapper
vPortAPICErrorHandlerWrapper:
    pusha
    call    vPortAPICErrorHandler
    popal
    /* EOI. */
    movl    $0x00, (0xFEE000B0)
    iret
.endfunc
/*-----------------------------------------------------------*/

.align 4
.func vPortTimerHandler
vPortTimerHandler:
    /* Save general purpose registers. */
    pushal
    PUSH_SEGS()

    /*
    * Stack layout at this point (ESP → increases downward as we push):
    * After pusha and PUSH_SEGS()():
    *   +-----------------+
    *   | GS              | ← ESP + 0
    *   +-----------------+
    *   | FS              | ← ESP + 4
    *   +-----------------+
    *   | ES              | ← ESP + 8
    *   +-----------------+
    *   | DS              | ← ESP + 12
    *   +-----------------+
    *   | EDI             | ← ESP + 16
    *   +-----------------+
    *   | ESI             | ← ESP + 20
    *   +-----------------+
    *   | EBP             | ← ESP + 24
    *   +-----------------+
    *   | ESP (original)  | ← ESP + 28
    *   +-----------------+
    *   | EBX             | ← ESP + 32
    *   +-----------------+
    *   | EDX             | ← ESP + 36
    *   +-----------------+
    *   | ECX             | ← ESP + 40
    *   +-----------------+
    *   | EAX             | ← ESP + 44
    *   +-----------------+
    * Hardware-pushed interrupt frame:
    *   | EIP (return)    | ← ESP + 48
    *   +-----------------+
    *   | CS              | ← ESP + 52
    *   +-----------------+
    *   | EFLAGS          | ← ESP + 56
    *   +-----------------+
    *   | ESP (user)      | ← ESP + 60 (only if ring change)
    *   +-----------------+
    *   | SS (user)       | ← ESP + 64 (only if ring change)
    *   +-----------------+
     */
      /* Interrupts are not nested, so save the rest of the task context. */
    .if configSUPPORT_FPU == 1

        /* If the task has a buffer allocated to save the FPU context then save the
        FPU context now. */
        movl    pucPortTaskFPUContextBuffer, %eax
        test    %eax, %eax
        je      1f
        fnsave  ( %eax ) /* Save FLOP context into ucTempFPUBuffer array. */
        fwait

        1:
        /* Save the address of the FPU context, if any. */
        push    pucPortTaskFPUContextBuffer

    .endif /* configSUPPORT_FPU */

    /* Find the TCB. */
    movl    pxCurrentTCB, %eax

    /* Stack location is first item in the TCB. */
    movl    %esp, (%eax)

    movl    52(%esp), %ecx       # 52 = pushal(32) + eip(4) + cs offset
    and     $3, %ecx
    test    %ecx, %ecx
    jne     already_on_kernel_stack

    movl    pxCurrentTCB, %eax
    movl    4(%eax), %esp        # pxKernelStackTop
    movl    %esp, %ebp

already_on_kernel_stack:

    /* Increment nesting count. */
    addl    $1, ulInterruptNesting

    call    xTaskIncrementTick

    sti

    /* Is a switch to another task required? */
    test    %eax, %eax
    je      _skip_context_switch
    cli
    call    vTaskSwitchContext

_skip_context_switch:
    cli

    /* Decrement the variable used to determine if a switch to a system
    stack is necessary. */
    subl    $1, ulInterruptNesting

    /* Stack location is first item in the TCB. */
    movl    pxCurrentTCB, %eax
    movl    (%eax), %esp

    .if configSUPPORT_FPU == 1

        /* Restore address of task's FPU context buffer. */
        pop     pucPortTaskFPUContextBuffer

        /* If the task has a buffer allocated in which its FPU context is saved,
        then restore it now. */
        movl    pucPortTaskFPUContextBuffer, %eax
        test    %eax, %eax
        je      1f
        frstor  ( %eax )
        1:
    .endif

    POP_SEGS()
    popa

    .if configUSE_APIC == 1
        /* EOI. */
        movl    $0x00, (0xFEE000B0)
    .elseif configUSE_I8259 == 1
        /* PIC EOI. */
        movb $0x20, %al
        outb %al, $0x20
        outb %al, $0xA0
    .endif
    iret


.endfunc
/*-----------------------------------------------------------*/

.if configUSE_COMMON_INTERRUPT_ENTRY_POINT == 1

    .align 4
    .func vPortCentralInterruptWrapper
    vPortCentralInterruptWrapper:

        portFREERTOS_INTERRUPT_ENTRY

        movl $0xFEE00170, %eax          /* Highest In Service Register (ISR) long word. */
        movl $8, %ecx                   /* Loop counter. */

    next_isr_long_word:
        test %ecx, %ecx                 /* Loop counter reached 0? */
        je wrapper_epilogue             /* Looked at all ISR registers without finding a bit set. */
        sub $1, %ecx                    /* Sub 1 from loop counter. */
        movl (%eax), %ebx               /* Load next ISR long word. */
        sub $0x10, %eax                 /* Point to next ISR long word in case no bits are set in the current long word. */
        test %ebx, %ebx                 /* Are there any bits set? */
        je next_isr_long_word           /* Look at next ISR long word if no bits were set. */
        sti
        bsr %ebx, %ebx                  /* A bit was set, which one? */
        movl $32, %eax                  /* Destination operand for following multiplication. */
        mul %ecx                        /* Calculate base vector for current register, 32 vectors per register. */
        add %ebx, %eax                  /* Add bit offset into register to get final vector number. */
        push %eax                       /* Vector number is function parameter. */
        call vPortCentralInterruptHandler
        pop %eax                        /* Remove parameter. */

    wrapper_epilogue:
        portFREERTOS_INTERRUPT_EXIT

    .endfunc

.endif /* configUSE_COMMON_INTERRUPT_ENTRY_POINT */
/*-----------------------------------------------------------*/

.align 4
.func vPortAPISpuriousHandler
vPortAPICSpuriousHandler:
    iret

.endfunc

.if configENABLE_RING3_TASKS == 1

    .align 4
    .func vPortSysCall
    vPortSysCall:
        cli
        pushal
        PUSH_SEGS()

         /*
         * Stack layout at this point (ESP → increases downward as we push):
         *   +-----------------+
         *   | GS              | ← ESP + 0 (top of stack after pushal)
         *   +-----------------+
         *   | FS              | ← ESP + 4
         *   +-----------------+
         *   | ES              | ← ESP + 8
         *   +-----------------+
         *   | DS              | ← ESP + 12
         *   +-----------------+
         *   | EDI             | ← ESP + 16
         *   +-----------------+
         *   | ESI             | ← ESP + 20
         *   +-----------------+
         *   | EBP             | ← ESP + 24
         *   +-----------------+
         *   | ESP (original)  | ← ESP + 28
         *   +-----------------+
         *   | EBX             | ← ESP + 32
         *   +-----------------+
         *   | EDX             | ← ESP + 36
         *   +-----------------+
         *   | ECX             | ← ESP + 40
         *   +-----------------+
         *   | EAX             | ← ESP + 44
         *   +-----------------+ (ESP points here)
          */

        mov     $0x10, %ax              /* KERNEL_DS 0x10 */
        mov     %ax, %ds
        mov     %ax, %es
        mov     %ax, %fs
        mov     %ax, %gs

        /* pushl %esp */                /* Parameter: regs_t * */
        call    uSysCallDispatch
        /* addl $4, %esp */

        POP_SEGS()

         /*
         * Stack layout at this point (ESP → increases downward as we push):
         *   +-----------------+
         *   | GS              | ← ESP + 32 (top of stack after pushal)
         *   +-----------------+
         *   | FS              | ← ESP + 28
         *   +-----------------+
         *   | ES              | ← ESP + 24
         *   +-----------------+
         *   | DS              | ← ESP + 20
         *   +-----------------+
         *   | EDI             | ← ESP + 16
         *   +-----------------+
         *   | ESI             | ← ESP + 12
         *   +-----------------+
         *   | EBP             | ← ESP + 8
         *   +-----------------+
         *   | ESP (original)  | ← ESP + 4
         *   +-----------------+
         *   | EBX             | ← ESP (current value after pushal)
         *   +-----------------+
         *   | EDX             |
         *   +-----------------+
         *   | ECX             |
         *   +-----------------+
         *   | EAX             |
         *   +-----------------+ (ESP points here)
          */
        popal

        sti
        iret
    .endfunc

.endif /* configENABLE_RING3_TASKS */

/* GDT flush function - loads new GDT and reloads segment registers */
.align 4
.func gdt_flush
gdt_flush:
    movl 4(%esp), %eax      /* Get the pointer to the GDT descriptor */
    lgdt (%eax)             /* Load the new GDT */

    /* Reload all segment registers */
    movw $0x10, %ax         /* 0x10 is the offset in the GDT to our data segment */
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    movw %ax, %ss

    /* Far jump to reload CS with our new code segment (0x08) */
    ljmp $0x08, $flush_cs
flush_cs:
    ret
.endfunc

.end

.section .note.GNU-stack,"",@progbits
