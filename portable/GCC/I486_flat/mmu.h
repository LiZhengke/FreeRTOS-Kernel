#ifndef MMU_H
#define MMU_H
#include <stdint.h>
#include "vmm.h"
/* Common page attribute bit definitions */
#define PG_PRESENT  0x001  /* Present in memory */
#define PG_RW       0x002  /* Read/Write (1=RW, 0=Read-only) */
#define PG_USER     0x004  /* User access (1=User, 0=Supervisor) */
#define PG_PWT      0x008  /* Write-through */
#define PG_PCD      0x010  /* Cache disabled */
#define PG_ACCESSED 0x020  /* Accessed */
#define PG_DIRTY    0x040  /* Dirty (PTE only) */

typedef uint32_t pde_t;
typedef uint32_t pte_t;
typedef uint32_t phys_addr_t;  /* 代表一个物理地址（仅是数字，不可解引用） */
typedef uint32_t virt_addr_t;  /* 代表一个虚拟地址 */
void init_paging(void);
void map_page(uint32_t *dir, uint32_t virtual_addr, uint32_t physical_addr, uint32_t flags);
void flush_tlb(uint32_t virtual_addr);
void* kernel_malloc_page(pde_t* page_dir_virt, size_t pages);
void create_user_page_directory(uint32_t* pgd_phys, uint32_t** pgd_virt);
#define KERNEL_VIRT_START 0xC0000000
#define KERNEL_VIRT_END   0xFFFFFFFF
#define KERNEL_OFFSET     KERNEL_VIRT_START

/* 物理地址转内核虚拟地址 (Physical to Virtual) */
inline void* p2v(phys_addr_t phys) {
    /* 只有在 1MB ~ __phys_end 范围内的物理地址才能用这个简单的宏转换 */
    return (void *)(phys + KERNEL_OFFSET);
}

/* 内核虚拟地址转物理地址 (Virtual to Physical) */
inline phys_addr_t v2p(void* virt) {
    /* 只有在内核代码/数据段的虚拟地址才能用这个转换 */
    return (phys_addr_t)((uint32_t)virt - KERNEL_OFFSET);
}

uint32_t user_to_phys(void *v_addr);
void map_user_section(pde_t* pgd, void* user_stack_top , size_t user_stack_depth);

__attribute__((section(".boot.text")))
void load_page_directory(uint32_t pd);

__attribute__((section(".boot.text")))
void enable_paging(void);

void mmu_init(void);
void mmu_test(void);

#endif /* MMU_H */
