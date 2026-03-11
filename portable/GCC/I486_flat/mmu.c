/* Page directory / page table entry common attribute bits */
#include <stddef.h>
#include "mmu.h"
#include "pmm.h"
#include "vmm.h"
#include "heap_alloc.h"

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
    }

    // 3. 将页表放入页目录的第一项
    page_directory[0] = ((uint32_t)page_table) | PG_PRESENT | PG_RW;

    // 3.1 将高半区映射到同一物理地址空间 (0xC0000000 -> 0x00000000)
    page_directory[KERNEL_PDE_START] = ((uint32_t)page_table) | PG_PRESENT | PG_RW;

    // 4. 将页目录地址告诉 CPU (写入 CR3 寄存器)
    load_page_directory((uint32_t)page_directory);

    // 5. 开启分页 (将 CR0 的第 31 位置 1)
    enable_paging();
}

void map_page(uint32_t *dir, uint32_t virtual_addr, uint32_t physical_addr, uint32_t flags) {
    uint32_t pd_index = virtual_addr >> 22;            /* Upper 10 bits */
    uint32_t pt_index = (virtual_addr >> 12) & 0x3FF;  /* Middle 10 bits */

    /* Check whether the corresponding page table exists */
    if (!(dir[pd_index] & PG_PRESENT)) {
        /* Page table does not exist; allocate a physical page for it.
         * Note: this must be a physical page address! */
        uint32_t new_pt = (uint32_t)pmm_alloc_page();

        /* Zero-fill the new page table to prevent stale mappings */
        uint32_t *pt_ptr = (uint32_t *)new_pt;
        for(int i = 0; i < 1024; i++) pt_ptr[i] = 0;

        /* Set the PDE with full permissions; fine-grained access is controlled by the PTE */
        dir[pd_index] = new_pt | PG_PRESENT | PG_RW | PG_USER;
    }

    uint32_t *page_table = (uint32_t *)(dir[pd_index] & 0xFFFFF000);
    page_table[pt_index] = PAGE_ADDR(physical_addr) | flags;

    /* Flush TLB so the CPU does not use a stale mapping */
    flush_tlb(virtual_addr);
}

/* Flush the TLB entry for a specific virtual address */
inline void flush_tlb(uint32_t virtual_addr) {
   __asm__ volatile("invlpg (%0)" : : "r" (virtual_addr) : "memory");
}

uint32_t create_user_page_directory(void) {
    /* Allocate a physical page for the new page directory */
    uint32_t new_pd = (uint32_t)pmm_alloc_page();
    if (!new_pd) return 0;

    uint32_t *pd = (uint32_t *)new_pd;

    /* Clear user-space entries (0 - 767) */
    for (int i = 0; i < KERNEL_PDE_START; i++) {
        pd[i] = 0;
    }

    /* Copy kernel-space entries (768 - 1023) from the boot page directory */
    for (int i = KERNEL_PDE_START; i < 1024; i++) {
        pd[i] = page_directory[i];
    }

    return new_pd;
}

void* kernel_malloc_page(pde_t* page_directory, size_t pages) {
    /* 1. Find a free region in virtual memory */
    void* virt_addr = vmm_alloc(pages);
    if (!virt_addr) return NULL;

    for (size_t i = 0; i < pages; i++) {
        /* 2. Allocate a physical RAM page */
        uint32_t phys_addr = (uint32_t)pmm_alloc_page();

        /* 3. Establish the virtual-to-physical mapping */
        uint32_t current_v = (uint32_t)virt_addr + (i * PAGE_SIZE);
        map_page(page_directory, current_v, phys_addr, PG_PRESENT | PG_RW);
    }

    return virt_addr;
}

void mmu_init(void) {
    kmalloc_init(page_directory, 16); /* 初始化内核堆，预分配 16 页 (64KB) */
    pmm_init(MEMORY_MAX_SIZE); /* 初始化物理内存管理器，假设总内存为 128MB */
}