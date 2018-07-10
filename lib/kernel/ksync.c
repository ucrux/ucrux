#include "ksync.h"
#include "proc.h"
#include "get_struct_pointor.h"
#include "debug.h"

// 初始化信号量
bool_t
init_sem( sem_t *sem, uint32_t sem_val )
{
  if( sem == NULL )
    return FALSE ;
  sem->sem = sem_val ;
  if( init_dlist( &(sem->wait_que) ) == FALSE )
    return FALSE ;

  return TRUE ;
}

/*
 * sem_down 和 sem_up 只能在内核空间中使用
 * 内核不可重入,所以不需要同步机制
 */
void
sem_down( sem_t *sem )
{
  pcb_t *pcb ;
  get_current_pcb( pcb ) ;

  if( sem->sem == 0 ) //没有可用信号量了
  {
    // 从进程运行列表中删除pcb
    // del_dlnode( &(pcb->pcb_list) ) ; 这一句是不需要的,因为正在运行的程序不可能在运行队列中
    pcb->status = WAITING ;
    add_dlnode( sem->wait_que.tail.prev, &(pcb->pcb_list) ) ;
    schedule( ) ;
  }
  sem->sem-- ;
}

void
sem_up( sem_t *sem )
{
  sem->sem++ ;
  pcb_t *pcb ;
  dlist_node_t *dlnode ;

  // 这里需要唤醒所有等待的进程
  while( sem->wait_que.ncnt > 0 )   // 有进程在等待队列中
  {
    // 将进程插入运行队列中
    dlnode = del_dlnode( sem->wait_que.head.next ) ;
    assert( dlnode != &(sem->wait_que.tail) && dlnode != NULL ) ;
    pcb = get_struct_pointor( pcb_t, pcb_list, dlnode ) ;
    pcb->status = RUNNING ;
    inst_proc_que_running( pcb ) ;
  }
}

// 尝试sem_down,成功返回true,失败返回false
bool_t
try_sem_down( sem_t *sem )
{
  if( sem->sem == 0 )
    return FALSE ;
  else
  {
    sem->sem-- ;
    return TRUE ;
  }
}

