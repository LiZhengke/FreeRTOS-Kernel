/* Page directory / page table entry common attribute bits */
#include <stddef.h>
#include <stdio.h>
#include "mmu.h"
#include "pmm.h"
#include "vmm.h"
#include "heap_alloc.h"
#include "FreeRTOS.h" // Add this to define StackType_t
#include "port.h"    // Add this for memset and memcpy

/* Extract the upper 20 bits of an address (aligned to 4KB) */
#define PAGE_ADDR(addr) ((uint32_t)(addr) & 0xFFFFF000)
/* 定义内核起始的页目录索引 (0xC0000000 >> 22 = 768) */
#define KERNEL_PDE_START 768

/* Page directory must be aligned to a 4KB boundary */
__attribute__((section(".boot"), aligned(PAGE_SIZE)))
static pde_t page_directory[1024] __attribute__((aligned(PAGE_SIZE)));

/* First page table, used to map the first 4MB where the kernel resides */
__attribute__((section(".boot"), aligned(PAGE_SIZE)))
static pte_t page_table[1024] __attribute__((aligned(PAGE_SIZE)));

#if configSUPPORT_PAGE_TABLE_TWO == 1
/* Second page table, used to map the next 4MB where the kernel resides */
__attribute__((section(".boot"), aligned(PAGE_SIZE)))
static pte_t page_table2[1024] __attribute__((aligned(PAGE_SIZE)));
#endif

void load_page_directory(uint32_t pd) {
    __asm volatile ("mov %0, %%cr3" :: "r" (pd));
}
void enable_paging(void) {
    __asm volatile (
        "mov %%cr0, %%eax\n"
        "or $0x80000000, %%eax\n"
        "mov %%eax, %%cr0\n"
        :
        :
        : "eax"
    );
}

__attribute__((section(".boot.text")))
void init_paging() {
    // 1. 将页目录初始化为“未就绪”
    for(int i = 0; i < 1024; i++) {
        page_directory[i] = 0 | PG_RW; // 设置为不在此处且可写
    }

    // 2. 填充第一个页表：映射 0.0MB 到 4.0MB (Identity Mapping)
    for(uint32_t i = 0; i < 1024; i++) {
        // 将物理地址 (i * 4KB) 填入页表
        page_table[i] = (i * 0x1000) | PG_PRESENT | PG_RW;
#if configSUPPORT_PAGE_TABLE_TWO == 1
        page_table2[i] = ((i + 1024) * 0x1000) | PG_PRESENT | PG_RW;
#endif
    }

    // 3. 将页表放入页目录的第一项
    page_directory[0] = ((uint32_t)page_table) | PG_PRESENT | PG_RW;
#if configSUPPORT_PAGE_TABLE_TWO == 1
    page_directory[1] = ((uint32_t)page_table2) | PG_PRESENT | PG_RW;
#endif


    // 3.1 将高半区映射到同一物理地址空间 (0xC0000000 -> 0x00000000)
    page_directory[KERNEL_PDE_START] = ((uint32_t)page_table) | PG_PRESENT | PG_RW;
#if configSUPPORT_PAGE_TABLE_TWO == 1
    page_directory[KERNEL_PDE_START + 1] = ((uint32_t)page_table2) | PG_PRESENT | PG_RW;
#endif


    // 4. 将页目录地址告诉 CPU (写入 CR3 寄存器)
    load_page_directory((uint32_t)page_directory);

    // 5. 开启分页 (将 CR0 的第 31 位置 1)
    enable_paging();
}

void map_page(uint32_t *dir_vir, uint32_t virtual_addr, uint32_t physical_addr, uint32_t flags) {
    uint32_t pd_index = virtual_addr >> 22;            /* Upper 10 bits */
    uint32_t pt_index = (virtual_addr >> 12) & 0x3FF;  /* Middle 10 bits */

    /* Check whether the corresponding page table exists */
    if (!dir_vir[pd_index] || !(dir_vir[pd_index] & PG_PRESENT)) {
        /* Page table does not exist; allocate a physical page for it.
         * Note: this must be a physical page address! */
        uint32_t new_pt = (uint32_t)pmm_alloc_page();
        if (!new_pt) return; /* Out of memory - in a real kernel, you'd want to handle this more gracefully */

        /* Zero-fill the new page table to prevent stale mappings */
        uint32_t *pt_ptr = (uint32_t *)p2v(new_pt); /* Convert to virtual address for initialization */
        for(int i = 0; i < 1024; i++) pt_ptr[i] = 0;

        /* Set the PDE with full permissions; fine-grained access is controlled by the PTE */
        dir_vir[pd_index] = new_pt | PG_PRESENT | PG_RW | PG_USER;
    }
    /*
        * At this point, the page table exists. Get its physical address from the PDE,
        * convert to virtual address, and set the PTE for the desired mapping.
        */
    uint32_t pt_phys_addr = dir_vir[pd_index] & 0xFFFFF000;
    uint32_t *page_table = (uint32_t *)p2v(pt_phys_addr);

    /* Set the PTE with the physical address and flags */
    page_table[pt_index] = (physical_addr & 0xFFFFF000) | flags | PG_PRESENT;

    /* Flush TLB so the CPU does not use a stale mapping */
    flush_tlb(virtual_addr);
}

/* Flush the TLB entry for a specific virtual address */
inline void flush_tlb(uint32_t virtual_addr) {
   __asm__ volatile("invlpg (%0)" : : "r" (virtual_addr) : "memory");
}

void create_user_page_directory(uint32_t* pgd_phys, uint32_t** pgd_virt) {
    /* Allocate a physical page for the new page directory */
    uint32_t new_pd_phys = (uint32_t)pmm_alloc_page();
    if (!new_pd_phys) return;

    uint32_t *pd = (uint32_t *)p2v(new_pd_phys); /* Convert to virtual address for initialization */

    /* Clear user-space entries (0 - 767) */
    for (int i = 0; i < KERNEL_PDE_START; i++) {
        pd[i] = 0;
    }

    /* Copy kernel-space entries (768 - 1023) from the boot page directory */
    for (int i = KERNEL_PDE_START; i < 1024; i++) {
        pd[i] = page_directory[i];
    }

    if (pgd_phys) *pgd_phys = new_pd_phys;
    if (pgd_virt) *pgd_virt = pd;

}

void* kernel_malloc_page(pde_t* page_dir_virt, size_t pages) {
    /* 1. Find a free region in virtual memory */
    void* virt_addr = vmm_alloc(pages);
    if (!virt_addr) return NULL;

    for (size_t i = 0; i < pages; i++) {
        /* 2. Allocate a physical RAM page */
        uint32_t phys_addr = (uint32_t)pmm_alloc_page();

        /* 3. Establish the virtual-to-physical mapping */
        uint32_t current_v = (uint32_t)virt_addr + (i * PAGE_SIZE);
        map_page(page_dir_virt, current_v, phys_addr, PG_PRESENT | PG_RW);
    }

    return virt_addr;
}

/* 引用链接脚本中的符号 */
extern char _user_text_vma_start[]; /* 0x08048000 */
extern char _kernel_phys_end[];     /* 物理起始点 (比如 0x150000) */
extern char _user_text_vma_end[];   /* 当前线程控制块，包含用户栈地址 */
// 专门计算用户模板段物理地址的逻辑
uint32_t user_to_phys(void *v_addr) {
/*    uint32_t virt = (uint32_t)v_addr;
    uint32_t v_start = (uint32_t)_user_text_vma_start;
    uint32_t p_start = (uint32_t)_kernel_phys_end;

    // 物理地址 = 物理基址 + (虚拟地址 - 虚拟基址)
    return p_start + (virt - v_start);
    */
   return (uint32_t)v_addr;
}

void map_user_section(pde_t* pgd, void* user_stack_top, size_t user_stack_depth) {
    // 1. 映射共享的用户代码“池” (使用修正后的物理偏移)
    uint32_t text_p = (uint32_t)_kernel_phys_end;
    uint32_t text_v = (uint32_t)_user_text_vma_start;
    uint32_t text_size = (uint32_t)_user_text_vma_end - text_v;

    for(uint32_t i = 0; i < text_size; i += 4096) {
        map_page(pgd, text_v + i, text_p + i, PG_PRESENT | PG_USER); // 只读执行
    }

     /* 计算栈的大小（字节） */
    uint32_t stack_size = user_stack_depth * sizeof( StackType_t );
    /* 计算用户栈的物理地址 */
    uint32_t user_stack_phys = (uint32_t)pmm_alloc_page(stack_size / 4096);

    /* 计算用户栈的虚拟地址 */
    uint32_t user_stack_virt = (uint32_t)user_stack_top;
    /* 计算用户栈占用的页数 */
    /* 假设 STACK_SIZE 是 4096 的倍数 */
    uint32_t num_pages = (stack_size + 4095) / 4096;
    size_t i;

    /* 映射足够的页面（根据 STACK_SIZE 计算页数） */
    for( i = 0; i < num_pages; i++ )
    {
        /* * 逻辑：
        * 物理页：从 user_stack_phys 开始往上加 (i * 4096)
        * 虚拟页：从 user_stack_virt 开始往下减 ((i + 1) * 4096)
        * 注意：栈顶地址通常是该页的末尾，所以映射时要减去一整页
        */
        uint32_t phys_page = user_stack_phys + (i * 4096);
        uint32_t virt_page = (user_stack_virt - stack_size) + (i * 4096);
        map_page( pgd,
                virt_page,
                phys_page,
                PG_PRESENT | PG_RW | PG_USER );
    }
}

extern char _user_blobs_start[];
extern char _user_blobs_end[];

// 以后你可以根据需要定义多个二进制块
// 或者在汇编里导出 _user_task_bin_start 符号

void spawn_user_task(pde_t* pgd) {
    // 获取用户程序在内核里的“缓存”位置
    void* template_addr = (void*)&_user_blobs_start;
    uint32_t template_size = (uint32_t)&_user_blobs_end - (uint32_t)&_user_blobs_start;

    // 1. 准备物理页
    uint32_t prog_phys = (uint32_t)pmm_alloc_page();

    // 2. 拷贝！将“嵌入在内核里的二进制”搬运到“新的物理页”
    // 注意：template_addr 是内核虚拟地址 (0xC0...)
    //      p2v(prog_phys) 也是内核访问该物理页的虚拟地址
    memcpy((void*)p2v(prog_phys), template_addr, template_size);

    // 3. 映射到任务空间
    // 虚拟地址 0x08048000 -> 物理页 prog_phys
    map_page(pgd, 0x08048000, prog_phys, PG_PRESENT | PG_RW | PG_USER);
}

void mmu_init(void) {
    pmm_init(MEMORY_MAX_SIZE); /* 初始化物理内存管理器，假设总内存为 128MB */
    kmalloc_init(p2v((phys_addr_t)page_directory), 16); /* 初始化内核堆，预分配 16 页 (64KB) */
    mmu_test(); /* 进行简单的映射测试，确保 MMU 工作正常 */
}

void mmu_test() {
     // 尝试映射一个远处的地址
    uint32_t test_virt = 0xDEADC000; // 虚拟地址
    uint32_t test_phys = 0x2000000; // 物理 32MB 处
    uint32_t* page_dir_virt = (uint32_t *)p2v((phys_addr_t)page_directory); // 获取第一个页目录的虚拟地址
    map_page(page_dir_virt, test_virt, test_phys, PG_PRESENT | PG_RW | PG_USER);
    /*load_page_directory((uint32_t)page_directory);*/ /* 刷新 TLB */

    // 尝试写入
    volatile uint32_t *ptr = (uint32_t*)test_virt;
    *ptr = 0x12345678;

    printf("Virtual 0xDEADC000 value: 0x%x\n", *ptr);
}
