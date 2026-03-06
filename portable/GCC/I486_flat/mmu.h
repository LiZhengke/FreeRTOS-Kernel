#ifndef MMU_H
#define MMU_H
#include <stdint.h>
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

void init_paging(void);
void map_page(uint32_t *dir, uint32_t virtual_addr, uint32_t physical_addr, uint32_t flags);
void flush_tlb(uint32_t virtual_addr);
void* kernel_malloc_page(pde_t* page_directory, size_t pages);
uint32_t create_user_page_directory(void);
#endif /* MMU_H */