OUTPUT_FORMAT("elf32-i386", "elf32-i386", "elf32-i386")
OUTPUT_ARCH(i386)
ENTRY(_start)

SECTIONS
{
    /* start16 placed at F000:0000 */
    . = 0xFFF00000;

    .text : {
        *(.text.start)
        *(.text .text.*)
    }

    .rodata : { *(.rodata*) }
    .data   : { *(.data*) }
    .bss    : { *(.bss) }

    . = 0xfffff800;
    .text16 : { *(.text16) }
    /* reset vector placed at 0xFFFF0 */
    . = 0xfffffff0;
    .resetvec : { *(.resetvec) }
}
