#include "timer_intr_handle.h"
#include "proc.h"
#include "dlist.h"
#include "msg.h"
#include "get_struct_pointor.h"
#include "kernel_service.h"
//debug
//#include "kputx.h"
//enddebug

extern dlist_head_t proc_sleep_list ;
static msg_t msg ;

void 
intr_timer_handle( uint32_t intr_vec )
{
  sleep_list_node_t *slnode ;
  pcb_t *pcb, *sleep_pcb ;
  get_current_pcb( pcb ) ;
  dlist_node_t *dlnode, *tmpdlnode ;

  // 为了编译时不报警
  if( intr_vec == 0 )
    return ;

  //处理休眠的进程
  for_each_dlnode( &proc_sleep_list, dlnode )
  {
    slnode = get_struct_pointor( sleep_list_node_t, sleep_node, dlnode ) ;
    sleep_pcb = get_struct_pointor( pcb_t, sleep_node, slnode ) ;
    if( sleep_pcb->sleep_node.sleep_time > 0 )
      sleep_pcb->sleep_node.sleep_time-- ;
    if( sleep_pcb->sleep_node.sleep_time == 0 )
    {
      //发消息唤醒进程
      init_msg( &msg, __KERNEL_PID, __KERNEL_MSG_WAKEUP, \
              NULL, 0 ) ;
      k_msg_send( sleep_pcb->pid, &msg, INTR );
      tmpdlnode = dlnode->prev ;
      del_dlnode( dlnode ) ;
      dlnode = tmpdlnode ;
    }
  }

  if( pcb->rest_ticks > 0 )
    pcb->rest_ticks-- ;
  // 这里需要防止pcb->all_used_ticks回卷到0,因为他是 uint32_t 类型的
  // 而我使用了 all_used_ticks == 0 来判断进是否是第一次调度到cpu
  pcb->all_used_ticks++ ;
  if( pcb->all_used_ticks == 0 )
    pcb->all_used_ticks = 1 ;
  //debug
  //kputstr( "process name is: " ); kputstr( pcb->proc_name ); kputchar( '\n' ) ;
  //kputstr( "rest ticks is: " ); kput_uint32_dec( pcb->rest_ticks ); kputchar( '\n' ) ;
  //kputstr( "all used ticks is: " ); kput_uint32_dec( pcb->all_used_ticks ); kputchar( '\n' ) ;
  //enddebug
  if( pcb->rest_ticks == 0 )
  {
    schedule( ) ;
  }
}


