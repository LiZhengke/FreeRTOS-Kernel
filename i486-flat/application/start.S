.code32
.section .text.entry, "ax"
.global _start
.type _start, @function

_start:
    call main
    /* If main returns, loop forever */
.hang:
    jmp .hang
