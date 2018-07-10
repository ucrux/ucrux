#ifndef __K_SYNC_H
#define __K_SYNC_H
#include "dlist.h"
#include "global_type.h"

/*
 * 内核同步头文件
 * 此文件包含各种内核同步机制
 */


/* 
 * 因为是单CPU环境,所以不需要lock指令前缀 
 * spinlock的原理是用1和内存中的一个值进行交换
 * 然后由其他部分检查交换出来的值是否为0
 * src初始值为1,dest是一个uint32_t的指针类型
 * 这个宏的函数原型为 spinlock( *src, *dest )
 */
#define spinlock( src, dest ) \
          asm volatile ( \
            "xchg (%k1), %0" : \
            "+r"(src) : \
            "r"(dest) : \
            "memory" )

/*
 * 下面要实现信号量
 * 信号量一共两个操作
 *   sem_down
 *   sem_up
 */
// 信号量数据结构
typedef struct s_sem
{
  uint32_t sem ;
  dlist_head_t wait_que ;
} sem_t ;


// 初始化信号量
bool_t init_sem( sem_t *sem, uint32_t sem_val ) ;

/*
 * sem_down 和 sem_up 只能在内核空间中使用
 * 内核不可重入,所以不需要同步机制
 */
void sem_down( sem_t *sem ) ;
void sem_up( sem_t *sem ) ;
// 尝试sem_down,成功返回true,失败返回false
bool_t try_sem_down( sem_t *sem ) ;
#endif