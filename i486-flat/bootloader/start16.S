/* =========================================================
 * start16.S
 *  - real mode entry at F000:0000
 *  - print 'R'
 *  - load GDT
 *  - enter protected mode
 *  - print 'P'
 * ========================================================= */
#define BOOT_SEG	0xffff0000	/* linear segment of boot code */


.section .text16, "ax"
.code16
.globl _start16

_start16:
    /* ---------- real mode init ---------- */
    cli
    mov %cs, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %ss

enable_a20_fast:
    in      $0x92, %al          # Read current value from port 0x92
    or      $0x02, %al          # Set bit 1 to 1 (Bit 1 is the A20 control bit)
    out     %al, $0x92          # Write back to port
    /* ---------- VGA test (real mode): 'R' ---------- */
    # mov $0xB800, %ax
    # mov %ax, %es
    # mov $0x1F52, %ax        /* 'R' */
    # mov %ax, %es:0

    /* ---------- load GDT (still real mode) ---------- */
    /* load GDT safely */
    /*lea gdt_desc_rm, %si
    lgdt (%si)*/
    data32 cs lgdt gdt_desc_rm

    /* ---------- enter protected mode ---------- */
    mov %cr0, %eax
    or  $0x1, %eax          /* set PE bit */
    mov %eax, %cr0

    /* far jump: flush CS and enter PM */
    /* ljmp $0x08, $pm_entry*/
    /* far jump: opcode form, avoid relocation */
    /* far jump 16:32 */
    /*.byte 0x66
    .byte 0xEA
    .long pm_entry
    .word 0x08*/
/* Flush the prefetch queue */
	jmp	ff
ff:

	/* Finally restore BIST and jump to the 32-bit initialization code */
	/*movl	%ecx, %eax*/
data32 cs	ljmp	*code32start
/* far jump 16:32 */
    /*.byte 0x66
    .byte 0xEA
    .long _start
    .word 0x08*/
	/* 48-bit far pointer */
code32start:
	.long	_start		/* offset */
	.word	0x08		/* segment */


/* =========================================================
 * Global Descriptor Table
 * ========================================================= */
/*.section .rodata.gdt, "a"*/
.align 8
gdt_desc_rm:
    .word gdt_end - gdt_begin - 1     /* limit */
    /*.long BOOT_SEG + gdt_begin                  /* base (32-bit) */
    .long gdt_begin

.align	16
.global gdt_begin
gdt_begin:
    .quad 0x0000000000000000    /* null descriptor */

    /* code segment: base=0, limit=4GB, RX */
    .quad 0x00CF9B000000FFFF

    /* data segment: base=0, limit=4GB, RW */
    .quad 0x00CF93000000FFFF

gdt_end:
