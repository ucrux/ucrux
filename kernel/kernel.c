#include "global_type.h"
#include "kernel.h"
#include "kputx.h"
#include "buddy_sys.h"
#include "page.h"
#include "interrupt.h"
#include "kernel_mm.h"
#include "debug.h"
#include "proc.h"
#include "msg.h"
#include "kernel_service.h"
#include "elf_load.h"
#include "string.h"
#include "dlist.h"
#include "syshd_opt.h"
#include "timer_8253.h"
//#include "syscall.h"
//debug
//#include "kstdio.h"
//enddebug

extern void change_pdt( uint32_t *pdt ) ;

dlist_head_t proc_sleep_list ;

static uint32_t free_mem_begin_phy_addr ;
// blob内存分配的管理结构
static mm_blob_t kernel_mblob ;

// 用于idle进程的函数
static void idle( void ) ;
// 用于接收消息
inline static msg_t *kernel_msg_recv( pid_t pid_recv_from, uint32_t msg_func, msg_type_t msg_type );

// 内核静态变量
static uint32_t mem_total  ;
static uint32_t mem_remain ;
static uint32_t pte_addr ;
static uint32_t phy_pg_addr ;
static unsigned int looppte, loop ;
static tss_t *tss ;
static pcb_t *kpcb ;

static pcb_t  *idle_pcb ;
static void *idle_func_entry, *file_buff ;
static intr_regs_t *idle_intr_regs ;
static uint32_t idle_func_entry_phy_addr ;

static msg_t *msg, msg_ret ;

// 消息返回时使用的临时变量
static pid_t reciever_pid ;
static bool_t bool_msg_ret ;
static pcb_t *proc_pcb ;
static uint32_t elf_phy_addr ;
static dlist_node_t *dlnode ;
static uint32_t proc_sleep_time ;
//static uint32_t user_mem_addr, user_mem_size ;



int main( void )
{
  //初始化内核静态变量
  mem_total = *(uint32_t *)__MEM_SIZE_ADDR ;
  pte_addr = __INIT_BEGIN_ADDR ;
  phy_pg_addr = __PAGE_INIT_MAX_LINE_ADDR ;
  tss = (tss_t *)__TSS_ADDR ;
  kpcb = (pcb_t *)(__KERNEL_PCB) ;
  msg = NULL ;


  kputstr( "\n               kernel started              \n" ) ;
  kputstr( "********************************************\n" ) ;

  kputstr( "initial process sleeping list " ) ;
  if( init_dlist( &proc_sleep_list ) == TRUE )
  {
    kputstr( "          [OK]\n" ) ;
  }
  else
  {
    kputstr( "      [FAILED]\n" ) ;
    goto kernel_halt ;
  }

  kputstr( "memory capacity check ........" ) ;

  //debug
  //asm volatile ( "cli;hlt");
  //enddebug

  // 系统中没有多余的内存了,本内核最大支持1G的内存
  if( __PAGE_INIT_MAX_LINE_ADDR <= mem_total || mem_total > __KERNEL_MAX_ADDR )
  {
    kputstr( "          [OK]\n" ) ;
  }
  else
  {
    kputstr( "      [FAILED]\n" ) ;
    goto kernel_halt ;
  }

  if( __PAGE_INIT_MAX_LINE_ADDR < mem_total )
  //需要把所有可用内存都一比一对等映射,为初始化伙伴系统做准备
  {
    mem_remain = mem_total - __PAGE_INIT_MAX_LINE_ADDR ;
    //计算外循环,需要多少个页表项表
    for( looppte = 0 ; looppte < (mem_remain >> 22 ) ; looppte++ )
    {
      for( loop = 0 ; loop < __PAGE_SIZE / __PTE_SIZE ; loop++ )
      {
        //一对一对等映射
        add_page( __PAGE_DIR_TABLE_ADDR, pte_addr, \
          phy_pg_addr, phy_pg_addr, __PG_P | __PG_RWW | __PG_USS ) ;
        phy_pg_addr += __PAGE_SIZE ;
      } 
      pte_addr += __PAGE_SIZE ;
    }
  }

  free_mem_begin_phy_addr = pte_addr ;

  kputstr( "initial memory buddy system .." ) ;
  //初始化伙伴系统,需要用到每一页内存来保存伙伴系统的数据结构,已经内核空间内存分配
  if( init_buddy_system( free_mem_begin_phy_addr, *(uint32_t *)__MEM_SIZE_ADDR ) )
    kputstr( "          [OK]\n" ) ;
  else
  {
    kputstr( "      [FAILED]\n" ) ;
    goto kernel_halt ;
  }

  

  kputstr( "initial interrupt ............" ) ;
  interrupt_init( ) ; // 初始化中断
  init_syscall( ) ;   // 初始化系统调用
  kputstr( "          [OK]\n" ) ;

  kputstr( "initial process management ..." ) ;
  if( init_proc_manage( ) == FALSE )
  {
    kputstr( "      [FAILED]\n" ) ;
    goto kernel_halt ;
  }
  kputstr( "          [OK]\n" ) ;

  // 内核主线程不需要进入proc_que
  // 起始 kernel_pcb 只是为了管理内存用的
  kputstr( "initial kernel main thread PCB" ) ;
  if( init_pcb(  kpcb, \
                 0, PROC_KERNEL, "kernel", \
                 NULL, (uint32_t *)__PAGE_DIR_TABLE_ADDR, \
                 &kernel_mblob ) == FALSE )
  {
    kputstr( "      [FAILED]\n" ) ;
    goto kernel_halt ;
  }
  if( insert_proc( kpcb, QUE_READ1 ) == FALSE )
  {
    kputstr( "      [FAILED]\n" ) ;
    goto kernel_halt ;
  }
  rm_proc_que( 0 ) ;  //因为内核主进程正在执行,所以要从进程队列上删除
  tss->ss0 = __SELECTOR_KERNEL_DATA ;
  kpcb->all_used_ticks = 1 ;
  kputstr( "          [OK]\n" ) ;

  // 初始化内核消息系统
  kputstr( "initial message system ......." ) ;
  init_msg_sys( ) ;
  kputstr( "          [OK]\n" ) ;



  // 初始化 idle 进程
  idle_pcb = mkpcb( "idle", 0, PROC_IDLE ) ;
  idle_func_entry = kmalloc( __PAGE_SIZE, idle_pcb ) ;
  idle_func_entry_phy_addr = get_pg_phy_addr( (uint32_t )idle_func_entry, (uint32_t )idle_pcb->pdt) ;
  memcpy( (void *)idle_func_entry_phy_addr, (void *)idle, __PAGE_SIZE ) ;
  idle_intr_regs = (intr_regs_t *)((uint32_t )idle_pcb + __PAGE_SIZE - sizeof( intr_regs_t )) ;
  idle_intr_regs->eip = (uint32_t )idle_func_entry ;
  insert_proc( idle_pcb, QUE_READ1 ) ;

  // 注册内核服务消息类型
  k_msg_regist( __KERNEL_PID, "kernel service" ) ;

  // 启动video驱动程序
  // 分配文件去读buff
  
  file_buff = kmalloc( __TASK_FILE_DEFAULT_SZIE, kpcb ) ;
  if( file_buff == NULL )
    goto kernel_halt ;
  
  sys_read_pri_disk( file_buff, __TASK_FILE_DEFAULT_SZIE / 512, __VIDEO_TASK_BEGIN_SEC ) ;

  
  if( ( proc_pcb = mkpcb( "video_task", __USER_TASK_ENTRY, PROC_TASK ) ) == NULL )
    goto kernel_halt ;

  if( kmalloc( __TASK_FILE_DEFAULT_SZIE + __PAGE_SIZE, proc_pcb ) == NULL )
  {
    kfree( proc_pcb, kpcb ) ;
    goto kernel_halt ;
  }

  change_pdt( proc_pcb->pdt ) ;
  elf_load( (uint32_t )file_buff ) ;
  change_pdt( kpcb->pdt ) ;
  insert_proc( proc_pcb, QUE_READ1 ) ;

  // 启动tty服务进程
  
  sys_read_pri_disk( file_buff, __TASK_FILE_DEFAULT_SZIE / 512, __TTY_TASK_BEGIN_SEC ) ;

  
  if( ( proc_pcb = mkpcb( "tty_task", __USER_TASK_ENTRY, PROC_TASK ) ) == NULL )
    goto kernel_halt ;

  if( kmalloc( __TASK_FILE_DEFAULT_SZIE + __PAGE_SIZE, proc_pcb ) == NULL )
  {
    kfree( proc_pcb, kpcb ) ;
    goto kernel_halt ;
  }

  change_pdt( proc_pcb->pdt ) ;
  elf_load( (uint32_t )file_buff ) ;
  change_pdt( kpcb->pdt ) ;
  insert_proc( proc_pcb, QUE_READ1 ) ;

  // 启动键盘驱动
  sys_read_pri_disk( file_buff, __TASK_FILE_DEFAULT_SZIE / 512, __KEYBOARD_TASK_BEGIN_SEC ) ;

  
  if( ( proc_pcb = mkpcb( "keyboard_task", __USER_TASK_ENTRY, PROC_TASK ) ) == NULL )
    goto kernel_halt ;

  if( kmalloc( __TASK_FILE_DEFAULT_SZIE + __PAGE_SIZE, proc_pcb ) == NULL )
  {
    kfree( proc_pcb, kpcb ) ;
    goto kernel_halt ;
  }

  change_pdt( proc_pcb->pdt ) ;
  elf_load( (uint32_t )file_buff ) ;
  change_pdt( kpcb->pdt ) ;
  insert_proc( proc_pcb, QUE_READ1 ) ;
  
  // 启动硬盘驱动程序

  // 启动文件系统驱动

  // 释放文件buff
  kfree( file_buff, kpcb ) ;
  // 内核正式启动
  kputstr( "\nkernel lived ...\n") ;

  while( TRUE )
  {
    // 开始接收消息
    /*
    asm volatile ( \
      "int $0x30" : \
      "=a"(msg) : \
      "a"(SYS_RECV_MSG),"b"(__PID_UNDEF),"c"(__MSG_FUNC_UNDEF),"d"(PROC) : \
      "memory" ) ;
    */
    msg = kernel_msg_recv( __PID_UNDEF, __MSG_FUNC_UNDEF, PROC );

    //debug
    //asm volatile ( "cli;hlt");
    //enddebug
    if( msg == NULL )
    {
      kputstr( "WARNING: no more memory!\n" ) ;
      asm volatile( "sti; hlt" ) ;
    }

    switch (msg->msg_func)
    {
      case __KERNEL_MSG_SEARCH :
        reciever_pid = k_msg_search( (char *)(msg->msg_buff) ) ;
        init_msg( &msg_ret, __KERNEL_PID, __KERNEL_MSG_SEARCH, \
          (void *)(&reciever_pid), sizeof(reciever_pid) ) ;
        //debug
        //printfk("__KERNEL_MSG_SEARCH, form %d, seacrh %s, ret %d\n", msg->sender, (char *)(msg->msg_buff), reciever_pid);
        //enddebug
        k_msg_send( msg->sender, &msg_ret, KERNEL ) ;
        break ;

      case __KERNEL_MSG_REGIST :
        bool_msg_ret = k_msg_regist( \
                        msg->sender, \
                        ((msg_reg_t *)(msg->msg_buff))->task_alias ) ;
        init_msg( &msg_ret, __KERNEL_PID, __KERNEL_MSG_REGIST, \
          (void *)(&bool_msg_ret), sizeof(bool_msg_ret) ) ;
        //debug
        //printfk("__KERNEL_MSG_REGIST, PID %d\n", msg->sender);
        //enddebug
        k_msg_send( msg->sender, &msg_ret, KERNEL ) ;
        break ;
      
      case __KERNEL_MSG_GETPID :
        reciever_pid = msg->sender ;
        init_msg( &msg_ret, __KERNEL_PID, __KERNEL_MSG_GETPID, \
          (void *)(&reciever_pid), sizeof(reciever_pid) ) ;
        k_msg_send( msg->sender, &msg_ret, KERNEL ) ;
        break ;
      /*
      case __KERNEL_MSG_MALLOC :
        proc_pcb = search_pcb( msg->sender ) ;
        user_mem_size = *((uint32_t *)(msg->msg_buff));
        user_mem_addr = (uint32_t )kmalloc( user_mem_size, proc_pcb );
        msg_ret.sender = __KERNEL_PID ;
        msg_ret.msg_func = 0xffffffffU ;
        *((uint32_t *)(msg_ret.msg_buff)) = user_mem_addr ;
        k_msg_send( msg->sender, &msg_ret ) ;
        break ;

      case __KERNEL_MSG_FREE :
        proc_pcb = search_pcb( msg->sender ) ;
        user_mem_addr = *((uint32_t *)(msg->msg_buff));
        kfree( (void *)user_mem_addr, proc_pcb );
        break ;
      */
      case __KERNEL_MSG_SLEEP :
        proc_pcb = search_pcb( msg->sender ) ;
        proc_sleep_time = *((uint32_t *)(msg->msg_buff));
        proc_pcb->sleep_node.sleep_time = proc_sleep_time * __IRQ0_FREQUENCY ;
        init_dlnode( &(proc_pcb->sleep_node.sleep_node) ) ;
        add_dlnode( &(proc_sleep_list.head), &(proc_pcb->sleep_node.sleep_node) ) ;
        break ;

      /*
      case __KERNEL_MSG_WAKEUP :
        break ;
      */
      case __KERNEL_MSG_PROC_CREATE :
        bool_msg_ret = FALSE ;
        // 为了简化,就不对elf格式的文件进行检查了(是否为可用的elf格式,在内存中的文件是否完整等)
        if( ( proc_pcb = \
                mkpcb( ((msg_proc_create_t *)(msg->msg_buff))->proc_name, \
                        __USER_TASK_ENTRY, \
                        ((msg_proc_create_t *)(msg->msg_buff))->ptype ) ) == NULL )
          goto creat_proc_return ; 
          // 分配重定向efl文件使用的内存空间
          // 分配文件大小加1页的内存
        if( kmalloc( ((msg_proc_create_t *)(msg->msg_buff))->file_size + __PAGE_SIZE, proc_pcb ) == NULL )
        {
          kfree( proc_pcb, kpcb ) ;
          goto creat_proc_return ;
        }
        // 重定向elf
        // 获取elf的物理地址
        elf_phy_addr = get_pg_phy_addr( \
                        ((msg_proc_create_t *)(msg->msg_buff))->elf_vaddr, \
                        (uint32_t )(((msg_proc_create_t *)(msg->msg_buff))->parent_pcb->pdt) ) ;

        change_pdt( proc_pcb->pdt ) ;
        elf_load( elf_phy_addr ) ;
        change_pdt( kpcb->pdt ) ;

        insert_proc( proc_pcb, QUE_READ1 ) ;
        bool_msg_ret = TRUE ;
        creat_proc_return :
          init_msg( &msg_ret, __KERNEL_PID, __KERNEL_MSG_PROC_CREATE, \
                    (void *)(&bool_msg_ret), sizeof(bool_msg_ret) ) ;
          k_msg_send( msg->sender, &msg_ret, KERNEL ) ;
          break ;
      case __KERNEL_MSG_PROC_DESTORY :
        proc_pcb = search_pcb( msg->sender ) ;
        // 一般来讲,这个消息是通过exit调用实现的
        // 而 exit 会先一个 k_msg_send
        // 再一个 k_msg_recv
        // 所以进程会阻塞在消息队列上
        // 所以一般不回出现 RUNNING 状态
        // 只会出现 WAIRTING 状态
        if( proc_pcb->status == RUNNING )
          remove_proc( proc_pcb->pid ) ;
        else
          rm_proc_hash( proc_pcb->pid ) ;
        //释放pdt
        free_pdt( proc_pcb->pdt ) ;
        // 释放blobxx分配管理节点
        while( ( dlnode = del_dlnode( proc_pcb->blobxx_alloc.head.next ) ) != NULL )
          free_pages( ((uint32_t )dlnode) & __PG_MASK, 1 ) ;
        // 释放vaddrnode分配管理节点
        while( ( dlnode = del_dlnode( proc_pcb->vaddrnode_alloc.head.next ) ) != NULL )
          free_pages( ((uint32_t )dlnode) & __PG_MASK, 1 ) ;
        //释放pcb
        kfree( (void *)proc_pcb, kpcb ) ;
        break ;
    }

    kfree( (void *)msg, kpcb ) ;
    msg = NULL ;

  }
 
  kernel_halt :
    asm volatile ( "cli; hlt" ) ;
  return 0 ;
}


inline static msg_t *
kernel_msg_recv( pid_t pid_recv_from, uint32_t msg_func, msg_type_t msg_type )
{
  msg_t *msg ;
  asm volatile ( \
    "int $0x30" : \
    "=a"(msg): \
    "a"(SYS_RECV_MSG),"b"(pid_recv_from),"c"(msg_func),"d"(msg_type) : \
    "memory" ) ;

  return msg ;
}

// 用于idle进程的函数
static void 
idle( void )
{

  /*
  //debug
  pid_t pid = 2 ;
  msg_t test_msg ;
  
  //init_msg( &test_msg, 1, 0, "fuck\n", strlen( "fuck\n" )+1 ) ;
  test_msg.sender = 1 ;
  test_msg.msg_func = 0 ;
  test_msg.msg_buff[0] = 'f' ;
  test_msg.msg_buff[1] = 'u' ;
  test_msg.msg_buff[2] = 'c' ;
  test_msg.msg_buff[3] = 'k' ;
  test_msg.msg_buff[4] = '\n' ;
  test_msg.msg_buff[5] = '\0' ;

  while( 1 )
  {
    asm volatile ( \
    "int $0x30" : : \
    "a"(SYS_SEND_MSG),"b"(pid),"c"(&test_msg) : \
    "memory") ;
  }
  //enddebug
  */
  
  
  while( TRUE )
  {
    asm volatile ( \
      "int $0x30" : : \
      "a"(SYS_IDLE) : \
      "memory" ) ;
  }
  
  
}
