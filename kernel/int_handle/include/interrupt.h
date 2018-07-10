#ifndef __INT_HANDLE_H
#define __INT_HANDLE_H
#include "global_type.h"


#define __TIMER_INTR_VEC    0x20U
#define __KEYBOARD_INTR_VEC 0x21U
#define __SYSCALL_INTR_VEC  0x30U

typedef enum e_syscall_type
{
  SYS_MALLOC ,
  SYS_FREE ,
  SYS_SEND_MSG ,
  SYS_RECV_MSG ,
//  SYS_GETPID ,
  SYS_IDLE ,
  SYSCALL_CNT
} syscall_type_t ;



typedef void (*p_intr_handle_func_t)( uint32_t intr_vec ) ;

typedef enum e_intr_status  
{
  INTR_OFF ,
  INTR_ON 
} intr_status_t ;

// 获取当前中断状态
intr_status_t intr_get_status( void ) ;
// 关闭中断,并返回之前的中断状态
intr_status_t intr_turn_off( void ) ;
// 开启中断,并返回之前的中断状态
intr_status_t intr_turn_on( void ) ;
// 设置中断位指定值,并返回之前的值
intr_status_t intr_set_status( intr_status_t status ) ;

// 正对中断向量安装中断处理程序
void inst_intr_handle( p_intr_handle_func_t func, uint32_t intr_vec ) ;
// 卸载原有的中断处理程序,并将其恢复成默认的中断处理程序
void uninst_intr_handle( uint32_t intr_vec ) ;

void exception_init( void ) ;
void interrupt_init( void ) ;

// 初始换系统调用,这个由内核调用
void init_syscall( void ) ;



#endif