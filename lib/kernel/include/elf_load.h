#ifndef __ELF_LOAD_H
#define __ELF_LOAD_H

// 将存储在内存地址 elf_file_addr 的elf文件,加载到正确的位置
extern void elf_load( uint32_t elf_file_addr ) ;

#endif