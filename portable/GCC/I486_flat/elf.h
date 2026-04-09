#ifndef ELF_H
#define ELF_H

#include <stdint.h>

#define PT_LOAD 1

typedef struct {
    unsigned char e_ident[16]; // 魔数 + 基本信息
    uint16_t e_type;           // 文件类型
    uint16_t e_machine;        // 架构
    uint32_t e_version;
    uint32_t e_entry;          // 🔥 程序入口地址
    uint32_t e_phoff;          // 🔥 Program Header 偏移
    uint32_t e_shoff;          // Section Header 偏移
    uint32_t e_flags;
    uint16_t e_ehsize;
    uint16_t e_phentsize;      // 每个 Program Header 大小
    uint16_t e_phnum;          // 🔥 Program Header 数量
    uint16_t e_shentsize;
    uint16_t e_shnum;
    uint16_t e_shstrndx;
} Elf32_Ehdr;

typedef struct {
    uint32_t p_type;    // = PT_LOAD
    uint32_t p_offset;  // 文件中的位置
    uint32_t p_vaddr;   // 🔥 加载到的虚拟地址
    uint32_t p_paddr;
    uint32_t p_filesz;  // 文件中实际大小
    uint32_t p_memsz;   // 内存中大小（>= filesz）
    uint32_t p_flags;   // 权限
    uint32_t p_align;
} Elf32_Phdr;

#endif /* ELF_H */
