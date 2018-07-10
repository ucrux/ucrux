#include "keyboard_intr.h"
#include "keyboard.h"
#include "io.h"
#include "proc.h"
#include "msg.h"
#include "kernel_service.h"
#include "interrupt.h"

//debug
//#include "kputx.h"
//enddebug


#define KBD_BUF_PORT 0x60       // 键盘buffer寄存器端口号为0x60(8420芯片)

static msg_t msg ;
static uint8_t scancode ;
static pid_t k_pid = __PID_UNDEF ;

/* 键盘中断处理程序 */
void 
intr_keyboard_handler( uint32_t intr_vec )
{
  if( intr_vec != __KEYBOARD_INTR_VEC )
    return ;

  scancode = inb(KBD_BUF_PORT) ;


  // 如果之前没有获取过keyboard进程的pid,不能通过系统调用,要直接使用内核函数
  if( k_pid == __PID_UNDEF ) //没有获取到 tty 的 pid
    if( (k_pid = k_msg_search( __KEYBOARD_ALIAS )) == __PID_UNDEF ) //tty进程还没有注册
      return ; //直接退出中断

  //debug
  //kputstr( "k pid:" ); kput_uint32_dec((uint32_t )k_pid); kputchar('\n');
  //enddebug

  /* 将普通字符或者特殊含义的组合键发送给tty进程 */
  // 初始化 keyboard_msg
  msg.sender = __KERNEL_PID ; //中断发出的消息都是由内存发出去的
  msg.msg_func = __KEYBOARD_GET_SCANCODE ;
  msg.msg_buff[0] = scancode ;

  k_msg_send( k_pid, &msg, INTR ) ;

}

