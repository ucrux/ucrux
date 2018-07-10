#ifndef __BUDDY_SYS_H
#define __BUDDY_SYS_H

#include "kernel.h"
#include "global_type.h"

#define __BUDDY_SYS_MAX_ORDER  10

/*
 * 伙伴系统的最小分配单位是一页, 
 * 最大分配单位是 __PAGE_SIZE << __BUDDY_SYS_MAX_ORDER 大小
 */

// 初始化伙伴系统,total_mem以字节为单位,实际上就是物理内存最后一个地址+1
bool_t init_buddy_system( uint32_t begin_phy_addr, uint32_t total_mem ) ;
// 申请页面.成功,返回物理地址的整数值;失败返回0
uint32_t alloc_pages( uint32_t pg_count ) ;
// 释放页面.
void free_pages( uint32_t pg_phy_addr, uint32_t pg_count ) ;

#endif