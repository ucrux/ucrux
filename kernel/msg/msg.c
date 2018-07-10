#include "msg.h"
#include "hash.h"
#include "get_struct_pointor.h"
#include "string.h"
#include "kernel.h"
#include "kernel_mm.h"
#include "ksync.h"
#include "debug.h"
#include "kernel_service.h"


extern void change_pdt( uint32_t *pdt ) ;
// debug
//#include "kputx.h"
// enddegbug

static hash_table_t msg_type ;


static void *msg_get_key( dlist_node_t *lnode ) ;
static int msg_cmp( void *key1, void *key2 ) ;
static uint32_t msg_hash( void *key ) ;
static uint32_t str_to_uint32( const char *str ) ;


// 初始化消息系统,主要是初始化服务进程注册hash表
void
init_msg_sys( void )
{
  init_hashtb( &msg_type, msg_hash, msg_get_key, msg_cmp ) ;
}

// 初始化 msg_t 类型的数据
bool_t 
init_msg( msg_t *msg, pid_t sender, uint32_t msg_func, void *msg_buff, uint32_t msg_buff_size )
{
  if( msg == NULL || msg_buff_size > __MSG_MAX_BUFF )
    return FALSE ;
  msg->sender = sender ;
  msg->msg_func = msg_func ;
  init_dlnode( &(msg->msg_node) ) ;
  if( msg_buff != NULL )
    memcpy( (void *)msg->msg_buff, (const void *)msg_buff, msg_buff_size) ;
  return TRUE ;
}
// 发送消息,成功返回TRUE,失败返回FALSE
// 内核不可重入.所有发送消息的时候不需要同步机制
// 这个msg的内存怎么管理需要斟酌一下
//  1. 首先是用户空间的地址
//  2. 通过int 0x80传过来还是使用用户空间的地址
//  3. 在内核空间分配地址,将原msg的内容拷贝过来
//  4. 用户空间的内存由用户空间自行管理
bool_t 
k_msg_send( pid_t pid_accept, msg_t *msg, msg_type_t msg_type )
{
  pcb_t *kernel_pcb = (pcb_t *)__KERNEL_PCB ;
  pcb_t *reciever_pcb ;
  msg_t *kmsg ;

  if( msg == NULL )
    return FALSE ;

  reciever_pcb = search_pcb( pid_accept ) ;
  if( reciever_pcb == NULL )    // 查询pid失败
    return FALSE ;

  kmsg = (msg_t *)kmalloc( sizeof( msg_t), kernel_pcb ) ;
  if( kmsg == NULL )  // 分配内存失败
    return FALSE ;

  memcpy( (void *)kmsg, (const void *)msg, sizeof(msg_t) ) ;

  // 初始化 kmsg->msg_node
  init_dlnode( &(kmsg->msg_node) ) ;

  // 准备插入消息,在消息队列尾插入消息, up_sem
  add_dlnode( reciever_pcb->msg_que.msg_list[msg_type].tail.prev, &(kmsg->msg_node) ) ;

  sem_up( &(reciever_pcb->msg_que.sem[msg_type]) ) ;
  return TRUE ;

}

/*这个是提供给系统调用使用的,所以只会是PROC(普通进程,task进程)类型的进程会调用*/
bool_t 
sys_msg_send( pid_t pid, msg_t *msg )
{
  pcb_t *pcb ;
  if( msg->sender == __PID_UNDEF ) //即pid没有初始化
  {
    get_current_pcb( pcb ) ;
    msg->sender = pcb->pid ;
  }
  return k_msg_send( pid, msg, PROC ) ;
}

// 接收消息,成功返回消息结构,失败返回NULL
// 内存路径
//  以当前 pcb 分配内存
//  把内核空间的消息拷贝到用户空间
msg_t *
k_msg_recv( pid_t pid_recv_from, uint32_t msg_func, msg_type_t msg_type )
{
  pcb_t *pcb, *kernel_pcb = (pcb_t *)__KERNEL_PCB ;
  dlist_node_t *dlnode = NULL ;
  msg_t *kmsg, *msg ;

  
  get_current_pcb( pcb ) ;
  //debug 
  //    kputstr( "step0\n" ) ;
  //enddebug

  msg = (msg_t *)kmalloc( sizeof(msg_t), pcb ) ;
  // 如果不是内核本身在接收消息,就要刷新TLB
  if( pcb != kernel_pcb )
    change_pdt( pcb->pdt ) ;

  //debug
  //assert( msg != NULL ) ;
  //enddebug

  if( msg == NULL ) //没有内存了
    return NULL ;

  //debug 
  //  kputstr( "step1\n" ) ;
  //enddebug
  // down sem
  sem_down( &(pcb->msg_que.sem[msg_type]) ) ; 
  //debug 
  //  kputstr( "step2\n" ) ;
  //enddebug

  // 获取消息队列的第一条消息
  assert( pcb->msg_que.msg_list[msg_type].ncnt > 0 ) ;
  //debug 
  //  kputstr( "step5\n" ) ;
  //enddebug

  //开始便利消息列表,找到需要的消息
  for_each_dlnode( &(pcb->msg_que.msg_list[msg_type]), dlnode )
  {
    kmsg = get_struct_pointor( msg_t, msg_node, dlnode ) ;

    if( pid_recv_from != __PID_UNDEF && msg_func != __MSG_FUNC_UNDEF )
    {
      if( kmsg->sender != pid_recv_from || kmsg->msg_func != msg_func )
        continue ;
    }
    else if( pid_recv_from != __PID_UNDEF )
    {
      if( kmsg->sender != pid_recv_from )
        continue ;
    }
    else if( msg_func != __MSG_FUNC_UNDEF )
    {
      if( kmsg->msg_func != msg_func )
        continue ;
    }
    break ;
  }

  if( dlnode == &(pcb->msg_que.msg_list[msg_type].tail) )
  {
    //没有找到需要的msg
    sem_up( &(pcb->msg_que.sem[msg_type]) ) ; //将信号量还原
    msg->msg_func = __MSG_FUNC_UNEXP ;
  }
  else
  {
    // 找到了想要的消息
    dlnode = del_dlnode( dlnode ) ;
    kmsg = get_struct_pointor( msg_t, msg_node, dlnode ) ;

    // 拷贝消息到用户空间
    memcpy( (void *)msg, (const void *)kmsg, sizeof(msg_t) ) ;
    // 释放内核空间消息内存
    kfree( (void *)kmsg, kernel_pcb ) ;
  }
  return msg ;
}

// 消息注册,成功返回TRUE,失败饭后FALSE
bool_t 
k_msg_regist( pid_t pid, char *task_alias)
{
  pcb_t *kernel_pcb = (pcb_t *)__KERNEL_PCB ;
  msg_type_node_t *mtnode ;
  if( task_alias == NULL )
    return FALSE ;

  mtnode = (msg_type_node_t *)kmalloc( sizeof(msg_type_node_t), kernel_pcb ) ;
  mtnode->reciever = pid ;
  memcpy( (void *)(mtnode->task_alias), (const void *)task_alias, __MSG_TASK_ALIAS ) ;
  // 别名直接截断,并不返回注册错误
  mtnode->task_alias[__MSG_TASK_ALIAS-1] = '\0' ;

  init_dlnode( &(mtnode->hash_list) ) ;

  if( insert_hashtb( &msg_type, &(mtnode->hash_list) ) == FALSE )
  {
    kfree( (void *)mtnode, kernel_pcb ) ;
    return FALSE ;
  }

  return TRUE ;
}
// 消息类型查询,成功返回进程pid,失败返回0xffffffff
pid_t
k_msg_search( char *task_alias )
{
  msg_type_node_t *mtnode ;
  dlist_node_t *dlnode ;

  if( task_alias == NULL )
    return __PID_UNDEF ;

  dlnode = srch_hashtb( &msg_type, (void *)task_alias ) ;

  if( dlnode == NULL )
    return __PID_UNDEF ;

  mtnode = get_struct_pointor( msg_type_node_t, hash_list, dlnode ) ;

  return mtnode->reciever ;
}



static void *
msg_get_key( dlist_node_t *lnode )
{
  msg_type_node_t *mtnode ;
  if( lnode == NULL )
    return NULL ;
  mtnode = get_struct_pointor( msg_type_node_t, hash_list, lnode ) ;
  return (void *)(mtnode->task_alias) ;
}

static int 
msg_cmp( void *key1, void *key2 )
{
  return strcmp( (const char *)key1, (const char *)key2 ) ;
}

static uint32_t 
msg_hash( void *key )
{
  uint32_t val ;
  val = str_to_uint32( (const char *)key ) ;
  return hash_uint32( (void *)val ) ;
}

unsigned int
str_to_uint32( const char *str )
{ 
  uint32_t retval = 0 ;
  uint32_t mvbits = 0 ;

  if( str == NULL || strlen( str ) > __MSG_TASK_ALIAS - 1  )
    return 0 ;

  while( *str != '\0' )
  {
    retval += ( (uint32_t) *str ) << mvbits ;
    mvbits++ ;
    str++ ;   
  }

  if( retval == 0 )
    retval = (unsigned int)'0' ;

  return retval ;
}


