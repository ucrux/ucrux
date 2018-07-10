#ifndef __BOOT_LOAD_H
#define __BOOT_LOAD_H

#define __KERNEL_ELF_FILE_ADDR      0x00050000
#define __KERNEL_ENTRY_ADDR         0x00011000
#define __KERNEL_STACK_BASE         0x00007000

void readkernel( void ) ;

#endif