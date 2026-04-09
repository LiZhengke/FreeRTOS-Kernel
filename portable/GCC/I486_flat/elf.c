#include "elf.h"
#include "mmu.h"

void load_elf(process_t *proc, void *elf_data) {
    Elf32_Ehdr *eh = (Elf32_Ehdr*)elf_data;
    Elf32_Phdr *ph = (Elf32_Phdr*)((char*)elf_data + eh->e_phoff);

    for (int i = 0; i < eh->e_phnum; i++) {

        if (ph[i].p_type != PT_LOAD)
            continue;

        uint32_t vaddr = ph[i].p_vaddr;
        uint32_t memsz = ph[i].p_memsz;
        uint32_t filesz = ph[i].p_filesz;
        uint32_t offset = ph[i].p_offset;

        // 👇 为该段分配并映射页
        map_user_pages(proc, vaddr, memsz);

        // 👇 拷贝文件数据
        memcpy((void*)vaddr,
               (char*)elf_data + offset,
               filesz);

        // 👇 清 BSS
        if (memsz > filesz) {
            memset((void*)(vaddr + filesz),
                   0,
                   memsz - filesz);
        }
    }

    proc->entry = eh->e_entry;
}
