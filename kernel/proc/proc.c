#include "proc.h"
#include "string.h"
#include "kernel.h"
#include "hash.h"
#include "mm.h"
#include "kernel_mm.h"
#include "get_struct_pointor.h"
#include "debug.h"

//debug
//#include "kputx.h"
//enddebug

/*
 * 进程生命周期管理要做的事情
 *  1. 从文件系统中读取程序文件, 由文件系统来做
 *  2. 分配pcb,初始化pcb
 *  3. 分配初始化内存
 *  4. 加载elf到指定虚拟地址,  文件系统完善之后再做
 *  5. 调度运行程序
 */


// 进程pcb优先级列表
/*
 * 进程一共分8个优先级 1 - 8 ;
 * 每个优先级对应一个链表
 * 每个优先级链表中的进程都调度并用完时间片之后(或者主动放弃CPU),
 * 下一个优先级的进程才能被调度
 */

/*
 * 进程调度方法
 * 从最高优先级开始调度,
 * 如果此优先级没有正在运行的进程
 * 就在上一级优先级中查找
 * 当查找完所有进程队列的正在运行队列,就将进程的就绪队列置为正在运行的队列
 * 如果就绪队列和正在运行的队列都没有其他进程可以调度,就调用idle进程
 * 注意:
 *  本内核是不可重入的
 *  每个优先级队列上有两个队列
 *    1. 一个为正在运行的对列: 进程状态是 RUNNING( 此队列由 running 指针指定)
 *    2. 另一个队列为就绪队列: 进程状态是 READY
 *    3. 当所有正在运行队列的进程用完自己的时间片就加入到就绪队列中
 */




typedef struct s_pcb_head 
{
  dlist_head_t prio_que[QUE_TYPE_CNT] ;    // 使用完ticks的会从其中一个队列中删除,插入到另一个队列中
  dlist_head_t *running ;                  // 进程从这个队列中查找,初始还指向prio_qui[QUE_READY1]
} pcb_head_t ;


static pcb_head_t proc_que[PROC_TYPE_CNT] ;
static hash_table_t proc_hash ;
static uint8_t pid_map[__PAGE_SIZE] ;     //刚号一页内存

// 初始化哈希表使用的函数(内存管理)
static void *proc_mm_getkey( dlist_node_t *lnode ) ;
// 初始化哈希表使用的函数(内存管理)
static int proc_mm_cmp( void *key1, void *key2 ) ;
// 初始化哈希表使用的函数(进程管理)
static void *proc_pid_getkey( dlist_node_t *lnode ) ;
// 初始化哈希表使用的函数(进程管理)
static int proc_pid_cmp( void *key1, void *key2 ) ;


// 分配进程id
// 这里需要一个数据结构
// 为了简便就不使用位图了
// 直接使用数组记录进程ip是否被占用
// 最大进程号为4095
// 分配不成功返回0
uint32_t 
new_pid( void )
{
  uint32_t loop ;
  for( loop = 1 ; loop < __PAGE_SIZE ; loop++ )
    if( pid_map[loop] == 0 )
    {
      pid_map[loop] = 1 ;
      return loop ;
    }
  return 0 ;
}

// 初始化的 status 为 READY
bool_t
init_pcb( pcb_t *pcb, pid_t pid, uint8_t priority, \
          char* proc_name, uint32_t *self_stack, \
          uint32_t *pdt, mm_blob_t *mmblob )
{
  int loop;
  //uint32_t loop ;
  if( pcb == NULL )
    return FALSE ;

  pcb->pid = pid ;
  pcb->status = RUNNING ;
  pcb->priority = priority ;
  if( priority == 0 ) //仅idle进程是这个优先级
    pcb->rest_ticks = 1 ;
  else                //其他进程优先级的默认ticks
    pcb->rest_ticks = priority << 4 ;
  // 将进程名 拷贝到 pcb->proc_name
  memcpy( (void *)pcb->proc_name, proc_name, __MAX_PROC_NAME-1 ) ;
  pcb->proc_name[__MAX_PROC_NAME-1] = '\0' ;
  pcb->kernel_stack_top = (uint32_t *)((char *)pcb + __PAGE_SIZE) ;
  pcb->self_stack = self_stack ;
  pcb->pdt = pdt ;

  // 初始化msg_que
  for( loop = 0 ; loop < MSG_TYPE_CNT ; loop++ )
  {
    if( init_dlist( &(pcb->msg_que.msg_list[loop]) ) == FALSE )
      return FALSE ;
    if( init_sem( &(pcb->msg_que.sem[loop]), 0 ) == FALSE )
      return FALSE ;
  }
  /*
  if( init_sem( &(pcb->wait_kernel_msg), 1 ) == FALSE )
    return FALSE ;
  */

  if( init_dlnode( &(pcb->pcb_list) ) == FALSE )
    return FALSE ;
  if( init_dlnode( &(pcb->hash_list) ) == FALSE )
    return FALSE ;
  if( init_vaddr_manage( &(pcb->vaddr_manage) ) == FALSE )
    return FALSE ;
  //初始化睡眠链表
  if( init_dlnode( &(pcb->sleep_node.sleep_node) ) == FALSE )
    return FALSE ;
  pcb->sleep_node.sleep_time = 0 ;

  /*
  for( loop = 0 ; loop < __MAX_OPEN_FILE ; loop++ )
    pcb->fd_table[loop] = -1 ;
  */
  pcb->mmblob = mmblob ;
  if( init_mm_blob( pcb->mmblob, hash_uint32, \
                    proc_mm_getkey, proc_mm_cmp ) == FALSE )
    return FALSE ;
  if( init_dlist( &(pcb->blobxx_alloc) ) == FALSE )
    return FALSE ;
  if( init_dlist( &(pcb->vaddrnode_alloc) ) == FALSE )
    return FALSE ;
  pcb->all_used_ticks = 0 ;
  pcb->exit_code = 0 ;

  return TRUE ;
}


// 初始化进程队列和进程hash表,成功返回TRUE,失败返回FALSE
bool_t 
init_proc_manage( void )
{
  uint32_t loop ;

  for( loop = 1 ; loop < __PAGE_SIZE ; loop++ )
    pid_map[loop] = 0 ;
  pid_map[0] = 1 ;  //pid == 0 被内核 pcb 占据

  for( loop = 0 ; loop < PROC_TYPE_CNT ; loop++ )
  {
    if( init_dlist( &(proc_que[loop].prio_que[QUE_READ1]) ) == FALSE || \
        init_dlist( &(proc_que[loop].prio_que[QUE_READ2]) ) == FALSE )
      return FALSE ;
    proc_que[loop].running = &(proc_que[loop].prio_que[QUE_READ1] ) ;
  }
  if( init_hashtb( &proc_hash, hash_uint32, proc_pid_getkey, proc_pid_cmp ) == FALSE )
    return FALSE ;

  return TRUE ;
}


// 将 pcb 插入到进程队列的就绪队列中,成功返回TRUE,失败返回FALSE
// 顺带还要插入到 hash 表
bool_t 
insert_proc( pcb_t *pcb, proc_que_type_t pqt )
{
  if( pcb == NULL || pcb->priority > PROC_TYPE_CNT - 1 )
    return FALSE ;
  if( add_dlnode( &(proc_que[pcb->priority].prio_que[pqt].head), &(pcb->pcb_list) ) == FALSE )
    return FALSE ;
  if( insert_hashtb( &proc_hash, &(pcb->hash_list) ) == FALSE )
  {
    del_dlnode( &(pcb->pcb_list) ) ;
    return FALSE ;
  }
  return TRUE ;
}

// 通过 pid 查找 pcb
pcb_t *
search_pcb( pid_t pid )
{
  pcb_t *pcb ;
  dlist_node_t *proc_hash_node ;

  proc_hash_node = srch_hashtb( &proc_hash, (void *)pid ) ;
  if( proc_hash_node == NULL )
    return NULL ;

  pcb = get_struct_pointor( pcb_t, hash_list, proc_hash_node ) ;

  return pcb ;
}

// 从proc_que中删除pcb,通过pid
pcb_t *
rm_proc_que( pid_t pid )
{
  pcb_t *pcb = search_pcb( pid ) ;
  if( pcb == NULL )
    return NULL ;
  del_dlnode( &(pcb->pcb_list) ) ;
  return pcb ;
}

// 将 pcb 插入到 proc_que,成功返回TRUE, 失败返回FALSE
// 总是插入到非运行队列
bool_t
inst_proc_que( pcb_t *pcb )
{
  proc_que_type_t pqt ;
  if( pcb == NULL )
    return FALSE ;

  if( proc_que[pcb->priority].running == &(proc_que[pcb->priority].prio_que[QUE_READ1]) )
    pqt = QUE_READ2 ;
  else
    pqt = QUE_READ1 ;

  if( add_dlnode( &(proc_que[pcb->priority].prio_que[pqt].head), &(pcb->pcb_list) ) == FALSE )
  {
    rm_proc_hash( pcb->pid ) ;
    return FALSE ;
  }
  return TRUE ;
}

// 插入到正在运行的队列中
bool_t 
inst_proc_que_running( pcb_t *pcb )
{
  if( pcb == NULL )
    return FALSE ;
  if( add_dlnode( &(proc_que[pcb->priority].running->head), &(pcb->pcb_list) ) == FALSE )
  {
    rm_proc_hash( pcb->pid ) ;
    return FALSE ;
  }
  return TRUE ;
}

// 从proc_hash中删除pcb,通过pid
pcb_t *
rm_proc_hash( pid_t pid )
{
  pcb_t *pcb ;
  dlist_node_t *proc_hash_node ;
  proc_hash_node =  del_hashtb( &proc_hash, (void *)pid ) ;
  if( proc_hash_node == NULL )
    return NULL ;
  pcb = get_struct_pointor( pcb_t, hash_list, proc_hash_node ) ;
  return pcb ;
}

// 将自定pid的进程从进程队列中删除, 成功返回 pcb ,失败返回NULL
pcb_t *
remove_proc( pid_t pid )
{
  pcb_t *pcb ;
  dlist_node_t *proc_hash_node ;
  
  proc_hash_node =  del_hashtb( &proc_hash, (void *)pid ) ;
  if( proc_hash_node == NULL )
    return NULL ;

  pcb = get_struct_pointor( pcb_t, hash_list, proc_hash_node ) ;
  del_dlnode( &(pcb->pcb_list) ) ;

  return pcb ;
}


// 创建新的pcb,成功返回pcb物理地址指针,失败返回NULL
// 新创建进程使用这个函数创建pcb
// 这里要区分 TASK 进程 和 USER 进程
pcb_t *
mkpcb( char *proc_name, uint32_t func_entry, proc_type_t ptype )
{
  pcb_t *pcb = NULL, *kpcb ;
  mm_blob_t *mblob ;
  uint32_t *pdt ;
  uint32_t pid ;
  uint32_t *self_stack ;
  uint8_t priority = (uint8_t)ptype ;
  intr_regs_t *intr_regs ;

  kpcb = (pcb_t *)(__KERNEL_PCB) ;

  pid = new_pid(  ) ;

  if( pid == 0 ) //进程号用完了
    return NULL ;

  pcb = (pcb_t *)kmalloc( __PAGE_SIZE, kpcb ) ;
  if( pcb == NULL )
    return NULL ;
  mblob = (mm_blob_t *)kmalloc( sizeof(mm_blob_t), kpcb ) ;
  if( mblob == NULL )
  {
    kfree( (void *)pcb, kpcb ) ;
    return NULL ;
  }
  pdt = mk_pdt( kpcb->pdt ) ;
  if( pdt == NULL )
  {
    kfree( (void *)mblob, kpcb ) ;
    kfree( (void *)pcb, kpcb ) ;
    return NULL ;
  }

  if( init_pcb( pcb, pid, priority, \
                proc_name, NULL, pdt, \
                mblob ) == FALSE )
  {
    kfree( (void *)mblob, kpcb ) ;
    kfree( (void *)pcb, kpcb ) ;
    free_pdt( pdt ) ;
    return NULL ;
  }
  self_stack = (uint32_t *)kmalloc( __PAGE_SIZE, pcb ) ;
  if( self_stack == NULL )
  {
    kfree( (void *)mblob, kpcb ) ;
    kfree( (void *)pcb, kpcb ) ;
    free_pdt( pdt ) ;
    return NULL ;
  }
  pcb->self_stack = (uint32_t *)((uint32_t )self_stack+__PAGE_SIZE) ;

  intr_regs = (intr_regs_t *)((uint32_t )pcb + __PAGE_SIZE - sizeof( intr_regs_t )) ;
  intr_regs->edi = 0 ;
  intr_regs->esi = 0 ;
  intr_regs->ebp = 0 ;
  intr_regs->ebx = 0 ;
  intr_regs->edx = 0 ;
  intr_regs->ecx = 0 ;
  intr_regs->eax = 0 ;
  intr_regs->eip = func_entry ;
  intr_regs->eflages = __DEFAULT_EFLAGS ;
  intr_regs->esp = (uint32_t )pcb->self_stack ;

  if( ptype == PROC_USER || ptype == PROC_IDLE )
  {
    intr_regs->gs = __SELECTOR_USER_DATA ;
    intr_regs->fs = __SELECTOR_USER_DATA ;
    intr_regs->es = __SELECTOR_USER_DATA ;
    intr_regs->ds = __SELECTOR_USER_DATA ;
    intr_regs->cs = __SELECTOR_USER_CODE ;
    intr_regs->ss = __SELECTOR_USER_DATA ;
  }
  else
  {
    intr_regs->gs = __SELECTOR_TASK_VIDEO ;
    intr_regs->fs = __SELECTOR_TASK_DATA ;
    intr_regs->es = __SELECTOR_TASK_DATA ;
    intr_regs->ds = __SELECTOR_TASK_DATA ;
    intr_regs->cs = __SELECTOR_TASK_CODE ;
    intr_regs->ss = __SELECTOR_TASK_DATA ;
  }

  pcb->kernel_stack_top = (uint32_t *)intr_regs ;

  return pcb ;
}

// 按照优先级挑选要执行的进程
// 按优先级在正在运行的对列中挑选
// 如果正在运行的队列中都没有可用进程
// 将切换正在运行的队列
// 这个时候已经是第二遍历遍所有进程队列了
// 如果第二遍历遍进程队列还没有找到可用进程,
// 就返回 idle 进程
pcb_t *
pickup_proc( void )
{
  dlist_node_t *proc_dlist_node ;
  int loop ;
  // 从最高优先级开始搜索
  for( loop = PROC_TYPE_CNT - 1 ; loop >= 0  ; loop-- )
  {
    if( proc_que[loop].running->ncnt > 0 )
    {
      proc_dlist_node = del_dlnode( proc_que[loop].running->head.next ) ;
      return get_struct_pointor( pcb_t, pcb_list, proc_dlist_node ) ;
    }
    // 切换正在运行队列
    else 
    {
      if( proc_que[loop].running == &(proc_que[loop].prio_que[QUE_READ1]) )
        proc_que[loop].running = &(proc_que[loop].prio_que[QUE_READ2]) ;
      else
        proc_que[loop].running = &(proc_que[loop].prio_que[QUE_READ1]) ;
    }
  }
  // 第二次历遍进程队列
  for( loop = PROC_TYPE_CNT - 1 ; loop >= 0  ; loop-- )
    if( proc_que[loop].running->ncnt > 0 )
    {
      proc_dlist_node = del_dlnode( proc_que[loop].running->head.next ) ;
      return get_struct_pointor( pcb_t, pcb_list, proc_dlist_node ) ;
    }

  return NULL ;
}


// 进程调度函数
void
schedule( void )
{
  pcb_t *from, *to ;
  get_current_pcb( from ) ;


  if( from->status == RUNNING )  // 因为时间片用完了,才被调度出CPU
  {
    /*
    // 还有内核或者中断消息没处理完,不进行调度继续执行该进程
    if( from->wait_kernel_msg.sem == 0 )
      return;
    */

    // 如果时间片用完了就重制时间片
    if( from->rest_ticks == 0 )  
    {
      if( from->priority == 0 )
        from->rest_ticks = 1 ;
      else
        from->rest_ticks = from->priority ;
    }


    // 将from 插入 就绪队列
    if( inst_proc_que( from ) == FALSE )
      return ;

    //debug
    //kputstr( "insert proc_que: " ); kput_uint32_dec( from->pid ); kputchar('\n') ;
    //enddebug
  }

  to = pickup_proc( ) ;         // 由于有idle进程,所以to不可能为空
  assert( to != NULL ) ;
  //debug
  //kputstr( "schedule to: " ); kput_uint32_dec( to->pid ); kputchar('\n') ;
  //enddebug
  switch_to( from, to ) ;
}

// 获取进程自己的pid
/*
pid_t 
sys_getpid( void )
{
  pcb_t *pcb ;
  get_current_pcb( pcb ) ;
  return pcb->pid ;
}
*/


// 局部函数-------------------
// 初始化哈希表使用的函数,哈希函数以pid为关键字(进程管理)
static void *
proc_pid_getkey( dlist_node_t *lnode )
{
  if( lnode == NULL )
    return NULL ;
  pcb_t *pcb ;
  pcb = get_struct_pointor( pcb_t, hash_list, lnode ) ;

  return (void *)(pcb->pid) ;
}

// 初始化哈希表使用的函数(进程管理)
static int 
proc_pid_cmp( void *key1, void *key2 )
{
  uint32_t k1 = (uint32_t )key1 ;
  uint32_t k2 = (uint32_t )key2 ;
  if( k1 == k2 )
    return 0 ;
  if( k1 > k2 )
    return 1 ;

  return -1 ;
}

// 初始化哈希表使用的函数(内存管理)
static void *
proc_mm_getkey( dlist_node_t *lnode )
{
  if( lnode == NULL )
    return NULL ;
  mm_blobxx_t *blobxx ;
  blobxx = get_struct_pointor( mm_blobxx_t, lnode, lnode ) ;

  return (void *)(blobxx->vaddr) ;
}

// 初始化哈希表使用的函数(内存管理)
static int 
proc_mm_cmp( void *key1, void *key2 )
{
  uint32_t k1 = (uint32_t )key1 ;
  uint32_t k2 = (uint32_t )key2 ;
  if( k1 == k2 )
    return 0 ;
  if( k1 > k2 )
    return 1 ;

  return -1 ;
}