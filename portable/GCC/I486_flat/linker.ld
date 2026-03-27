/* linker_high.ld - i486 higher-half kernel with embedded user blobs */

OUTPUT_FORMAT(elf32-i386)
ENTRY(_start)

KERNEL_VMA = 0xC0000000;   /* 虚拟基地址 */
KERNEL_LMA = 0x00100000;   /* 物理加载地址 */

SECTIONS
{
    /* ========================================
     * 1. 引导段：运行在物理地址
     * ======================================== */
    . = KERNEL_LMA;

    .boot ALIGN(4K) : AT(KERNEL_LMA)
    {
        KEEP(*(.multiboot))
        *(.boot*)
    }

    /* ========================================
     * 2. 切换到高半部虚拟地址
     * ======================================== */
    . += KERNEL_VMA;

    /* -------------------- */
    /* 内核代码和只读数据     */
    /* -------------------- */
    .text ALIGN(4K) : AT(ADDR(.text) - KERNEL_VMA)
    {
        *(.text*)
        *(.rodata*)
        *(.eh_frame)
    }

    /* ----------------------------------------------------------
     * 3. 用户程序二进制块 (User Blobs)
     * 这里存放通过 .incbin 导入的 user_task.bin
     * 它们在内核眼中只是数据，不再尝试映射到 0x08048000
     * ---------------------------------------------------------- */
    .user_blobs ALIGN(4K) : AT(ADDR(.user_blobs) - KERNEL_VMA)
    {
        _user_blobs_start = .;
        *(.user_bin)   /* 对应汇编中的 .section .user_bin */
        . = ALIGN(4K);
        _user_blobs_end = .;
    }

    /* -------------------- */
    /* 内核数据段           */
    /* -------------------- */
    .data ALIGN(4K) : AT(ADDR(.data) - KERNEL_VMA)
    {
        __data_start = .;
        *(.data*)
        __data_end = .;
    }

    /* -------------------- */
    /* 内核 BSS 段          */
    /* -------------------- */
    .bss ALIGN(4K) : AT(ADDR(.bss) - KERNEL_VMA)
    {
        __bss_start = .;
        *(.bss*)
        *(COMMON)
        __bss_end = .;
    }

    /* -------------------- */
    /* 内核堆和栈           */
    /* -------------------- */
    . = ALIGN(4K);
    __heap_start = .;
    .heap : AT(ADDR(.heap) - KERNEL_VMA)
    {
        *(.heap)
        . += 64K;  /* 预留 64KB 堆空间 */
    }
    __heap_end = .;

    . = ALIGN(16);
    __stack_bottom = .;
    . += 64K;      /* 预留 64KB 内核栈空间 */
    __stack_top = .;

    /* 符号导出：用于物理内存管理 */
    _kernel_end = .;
    __phys_start = KERNEL_LMA;
    __phys_end = . - KERNEL_VMA;
}
