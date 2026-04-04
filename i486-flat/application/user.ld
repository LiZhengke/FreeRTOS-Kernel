ENTRY(_start)

SECTIONS
{
    . = 0x08048000;

    .text : {
        *(.text.entry)
        *(.text .text.*)
    }

    .rodata : { *(.rodata .rodata.*) }
    .data   : { *(.data .data.*) }
    .bss    : { *(.bss .bss.* COMMON) }

    /DISCARD/ : { *(.comment) *(.note*) *(.eh_frame*) }
}
