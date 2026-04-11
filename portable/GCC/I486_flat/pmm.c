#include <stddef.h>
#include "pmm.h"
#include "os_helper.h"

uint8_t pmm_bitmap[BITMAP_SIZE];
/**
 * Find the first free page (currently implements single-page allocation).
 * @return Starting address of the allocated physical page.
 */
void* pmm_alloc_page() {
    for (uint32_t i = 0; i < TOTAL_PAGES; i++) {
        if (!bitmap_test(i)) {
            bitmap_set(i);
            return (void*)(i * PAGE_SIZE);
        }
    }
    return NULL; /* Out of memory */
}

/**
 * Free a physical page.
 */
void pmm_free_page(void* phys_addr) {
    uint32_t page_idx = (uint32_t)phys_addr / PAGE_SIZE;
    bitmap_unset(page_idx);
}

void pmm_init(uint32_t mem_size) {
    /* 1. Mark all pages as occupied by default */
    memset(pmm_bitmap, 0xFF, BITMAP_SIZE);

    /* 2. Mark pages in the usable region as free.
     *    Assumes memory from 4MB up to mem_size is available. */
    uint32_t start_page = 0x400000 / PAGE_SIZE;
    uint32_t end_page = mem_size / PAGE_SIZE;

    for (uint32_t i = start_page; i < end_page; i++) {
        bitmap_unset(i);
    }

    /* 3. Protect pages occupied by the kernel itself.
     *    Uses the _kernel_end linker symbol to determine the boundary. */
    extern char _kernel_end[];
    uint32_t kernel_pages_end = ((uint32_t)_kernel_end - 0xC0000000) / PAGE_SIZE;
    for (uint32_t i = start_page; i <= kernel_pages_end; i++) {
        bitmap_set(i);
    }
}
