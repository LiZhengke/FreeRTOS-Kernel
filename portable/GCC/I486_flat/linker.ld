/* linker_high.ld - i486 higher-half kernel with paging */

OUTPUT_FORMAT(elf32-i386)
ENTRY(_start)

KERNEL_VMA = 0xC0000000;   /* Virtual address base */
KERNEL_LMA = 0x00100000;   /* Physical load address */

/* -------------------- */
/* Linker section definitions */
/* -------------------- */
SECTIONS
{
    /* ========================================
     * Boot code — runs at PHYSICAL addresses
     * before paging is enabled.
     * ======================================== */
    . = KERNEL_LMA;

    .boot ALIGN(4K) : AT(KERNEL_LMA)
    {
        KEEP(*(.multiboot))
        *(.boot*)
    }

    /* ========================================
     * From here on, everything has VMA in the
     * higher half (0xC0000000+).
     * ======================================== */
    . += KERNEL_VMA;

    /* -------------------- */
    /* Code + read-only data */
    /* -------------------- */
    .text ALIGN(4K) : AT(ADDR(.text) - KERNEL_VMA)
    {
        *(.text*)                /* All code */
        *(.rodata*)              /* Constant data */
        *(.eh_frame)
    }

    /* -------------------- */
    /* Initialized data     */
    /* -------------------- */
    .data ALIGN(4K) : AT(ADDR(.data) - KERNEL_VMA)
    {
        __data_start = .;
        *(.data*)
        __data_end = .;
    }

    /* -------------------- */
    /* BSS                  */
    /* -------------------- */
    .bss ALIGN(4K) : AT(ADDR(.bss) - KERNEL_VMA)
    {
        __bss_start = .;
        *(.bss*)
        *(COMMON)
        __bss_end = .;
    }
    _kernel_end = .;

    /* -------------------- */
    /* Heap for FreeRTOS    */
    /* -------------------- */
    . = ALIGN(8);
    __heap_start = .;
    .heap : AT(ADDR(.heap) - KERNEL_VMA)
    {
        *(.heap)
    }
    . += 64K;
    __heap_end = .;

    /* -------------------- */
    /* Stack (top-down)     */
    /* -------------------- */
    . = ALIGN(16);
    . += 64K;
    __stack_top = .;

    /* Physical addresses for boot code */
    __phys_start = KERNEL_LMA;
    __phys_end   = LOADADDR(.heap) + SIZEOF(.heap) + 64K;
}
