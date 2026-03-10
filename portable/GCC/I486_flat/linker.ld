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

    _kernel_phys_end = . - KERNEL_VMA;
    _user_code_phys_start = _kernel_phys_end; /* 记录用户代码段的物理起始地址 */
    /* ==========================================================
     * 3. 用户态模版段 (User Section Templates)
     * 这些代码会被链接器放在内核镜像的末尾（物理上），但编译器认为它们在 0x08048000
     * ========================================================== */
    
    /* 定义用户代码段的运行起始地址 */
    . = 0x08048000; 

    .user_text ALIGN(4K) : AT(_kernel_phys_end)
    {
        _user_text_vma_start = .;
        *(.user_text)  /* 存放所有的用户函数和 exit 包装器 */
        _user_text_vma_end = .;
    }

    /* 更新物理指针，紧跟在 user_text 物理空间之后 */
    _user_data_phys_template = _kernel_phys_end + SIZEOF(.user_text);

    . = ALIGN(4K); /* 数据段也保持页对齐，方便单独映射 */
    .user_data ALIGN(4K) : AT(_user_data_phys_template)
    {
        _user_data_vma_start = .;
        *(.user_data)
        *(.user_rodata)
        _user_data_vma_end = .;
    }

    /* 记录所有用户模版占用的总物理结束位置 */
    _user_templates_phys_end = _user_data_phys_template + SIZEOF(.user_data);

    /* ==========================================================
     * 4. 回归内核空间：定义堆和栈
     * ========================================================== */
    . = KERNEL_VMA + _user_templates_phys_end;
    . = ALIGN(4K);
    /* -------------------- */
    /* Heap for FreeRTOS    */
    /* -------------------- */
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
    __phys_end   = _user_templates_phys_end;
    _kernel_end = .;
}
