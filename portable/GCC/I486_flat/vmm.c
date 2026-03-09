#include "vmm.h"
#include "mmu.h"
#include "pmm.h"
/**
 * Kernel virtual memory management module.
 * Allocates contiguous virtual address space and maps it to physical memory.
 * Depends on the physical memory manager (pmm) and paging subsystem (mmu).
 *
 * Design:
 * - The kernel virtual address space starts at 3GB and grows upward.
 * - Each allocation finds a free region in virtual space, obtains physical RAM,
 *   then establishes the mapping.
 * - Currently uses simple linear allocation with no reclamation (can be extended later).
 *
 * Notes:
 * - Ensure allocated virtual address space does not exceed the preset range.
 * - Set the correct permission and attribute bits when mapping (e.g. RW, user/kernel). */
/* Start of kernel virtual space (typically the 3GB boundary) */
#define KERNEL_VIRT_START 0xC0000000
#define KERNEL_VIRT_END   0xFFFFFFFF

/* Tracks where the next virtual address allocation begins */
static uint32_t next_virt_addr = KERNEL_VIRT_START;

/**
 * Allocate contiguous virtual address space.
 * @param pages  Number of pages to allocate (4KB each).
 * @return       Starting virtual address.
 */
void* vmm_alloc(size_t pages) {
    uint32_t size = pages * PAGE_SIZE;

    if (next_virt_addr + size > KERNEL_VIRT_END) {
        /* Virtual address space exhausted */
        return NULL;
    }

    uint32_t addr = next_virt_addr;
    next_virt_addr += size;

    return (void*)addr;
}
