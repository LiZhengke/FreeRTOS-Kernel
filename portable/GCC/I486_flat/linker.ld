/* linker.ld - i486 flat memory */

OUTPUT_FORMAT(elf32-i386)
ENTRY(_start)

MEMORY
{
    RAM (rwx) : ORIGIN = 0x00100000, LENGTH = 1M
}

SECTIONS
{
    . = ORIGIN(RAM);

    /* -------------------- */
    /* Code + RO data       */
    /* -------------------- */
    .text : ALIGN(4)
    {
        KEEP(*(.multiboot))      /* If you use multiboot in the future */
        *(.text*)
        *(.rodata*)
    } > RAM

    /* -------------------- */
    /* Initialized data     */
    /* -------------------- */
    .data : ALIGN(4)
    {
        __data_start = .;
        *(.data*)
        __data_end = .;
    } > RAM

    /* -------------------- */
    /* BSS (zeroed)         */
    /* -------------------- */
    .bss : ALIGN(4)
    {
        __bss_start = .;
        *(.bss*)
        *(COMMON)
        __bss_end = .;
    } > RAM

    /* -------------------- */
    /* Heap for FreeRTOS    */
    /* -------------------- */
    . = ALIGN(8);
    __heap_start = .;
    . += 64K;                    /* heap size */
    __heap_end = .;

    /* -------------------- */
    /* Stack (top-down)     */
    /* -------------------- */
    . = ORIGIN(RAM) + LENGTH(RAM);
    __stack_top = .;
}
