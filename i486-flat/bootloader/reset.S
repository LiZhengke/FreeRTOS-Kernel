.extern _start16
.code16
.section .resetvec, "ax"
.globl reset_vector

reset_vector:
    cli
    jmp _start16
    # ljmp $0x1234,$0x5678
    #.byte 0xEA          /* far jump opcode */
    #.word 0x0000        /* IP (Instruction Pointer) */
    #.word 0xF000        /* CS (Code Segment) */

