#include "kputx.h"
#include "elf_load.h"
#include "bootload.h"

int
main( void )
{
  kputstr( "            boot loader started             \n" ) ;
  kputstr( "********************************************\n" ) ;
  kputstr( "read kernel file from disk ..." ) ;
  readkernel(  ) ;
  kputstr( "          [OK]\n" ) ;
  kputstr( "relocate kernel .............." ) ;
  elf_load( __KERNEL_ELF_FILE_ADDR ) ;
  kputstr( "          [OK]\n\n" ) ;

  // 注意,这里将栈指针恢复成内核的栈指针
  asm volatile ( "movl %k1, %%esp; \
                  jmp *%k0" \
                  : : \
    "a"(__KERNEL_ENTRY_ADDR),"b"(__KERNEL_STACK_BASE) : \
    "memory") ;
  
  return 0 ;
}