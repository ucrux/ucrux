#ifndef __KERNEL_MM_H
#define __KERNEL_MM_H 

#include "mm.h"
#include "proc.h"

/*
 *       free malloc  用户空间分配函数
 *           |
 *           v
 *        int 0x80  
 *           |
 *           v
 *      kfree kmalloc
 */


// 内核内存分配函数
void *kmalloc( uint32_t size, pcb_t *pcb ) ;
// 内核内存释放函数
// 重复释放同一个内存地址或者释放一个没有分配的地址会有风险
void kfree( void *ptr, pcb_t *pcb ) ;

// 系统调用
void *sys_malloc( uint32_t size ) ;
void sys_free( void *ptr ) ;


#endif