#include "syscall.h"
#include "proc.h"
#include "interrupt.h"
#include "string.h"




// 以下函数可在用户空间调用
// 获取自己的pid

pid_t 
getpid( void )
{
  pid_t pid ;
  msg_t msg, *msg_get ;
  /*
  asm volatile ( \
    "int $0x30" : \
    "=a"(pid) : \
    "a"(SYS_GETPID) : \
    "memory" ) ;
  */
  msg.sender = __PID_UNDEF ;
  msg.msg_func = __KERNEL_MSG_GETPID ; 

  msg.msg_buff[0] = '\0' ;

  if( msg_send( __KERNEL_PID, &msg ) == FALSE )
    return __PID_UNDEF ;

  get_message :
    msg_get = msg_recv( __KERNEL_PID, __KERNEL_MSG_GETPID, KERNEL ) ;
    if( msg_get == NULL )
      return __PID_UNDEF ;
    if( msg_get->msg_func == __MSG_FUNC_UNEXP )
    {
      free( (void *)msg_get ) ;
      sleep(1);
      goto get_message ;
    }
  //获取返回值
  pid = *(pid_t *)(msg_get->msg_buff) ;

  free( (void *)msg_get ) ;
  return pid ;
}


// 寻找task进程的pid
pid_t 
task_search( char *task_alias )
{
  pid_t pid ;
  msg_t msg, *msg_get ;

  if( task_alias == NULL )
    return __PID_UNDEF ;

  //pid = getpid( ) ;


  msg.sender = __PID_UNDEF ;
  msg.msg_func = __KERNEL_MSG_SEARCH ; 

  memcpy( (void *)(msg.msg_buff), task_alias, __MSG_TASK_ALIAS ) ;
  msg.msg_buff[__MSG_TASK_ALIAS-1] = '\0' ;

  if( msg_send( __KERNEL_PID, &msg ) == FALSE )
    return __PID_UNDEF ;

  get_message :
    msg_get = msg_recv( __KERNEL_PID, __KERNEL_MSG_SEARCH, KERNEL ) ;
    if( msg_get == NULL )
      return __PID_UNDEF ;
    if( msg_get->msg_func == __MSG_FUNC_UNEXP )
    {
      free( (void *)msg_get ) ;
      sleep(1) ;
      goto get_message ;
    }
  //获取返回值
  pid = *(pid_t *)(msg_get->msg_buff) ;

  free( (void *)msg_get ) ;

  return pid ;
}

// 分配内存
void *
malloc( uint32_t size )
{
  void *ptr ;
  //msg_t msg, *msg_get ;
  
  if(size == 0)
    return NULL ;

  asm volatile ( \
    "int $0x30" : \
    "=a"(ptr) : \
    "a"(SYS_MALLOC),"b"(size) : \
    "memory" ) ;

/*
  msg.sender = __PID_UNDEF ;
  msg.msg_func = __KERNEL_MSG_MALLOC ; 

  *((uint32_t *)msg.msg_buff) = size ;

  if( msg_send( __KERNEL_PID, &msg ) == FALSE )
    return NULL ;

  get_message :
    msg_get = msg_recv( ) ;
    if( msg_get == NULL )
      return NULL ;
    if( msg_get->sender != __KERNEL_PID )
    {
      free( (void *)msg_get ) ;
      goto get_message ;
    }

  ptr = (void *)(*(uint32_t *)(msg_get->msg_buff)) ;

  free( (void *)msg_get ) ;
*/
  return ptr ;
}

// 释放内存
void 
free( void *ptr )
{
  //msg_t msg ;
  if( ptr == NULL )
    return ;
  asm volatile ( \
    "int $0x30" : : \
    "a"(SYS_FREE),"b"(ptr) : \
    "memory" ) ;
  //msg.sender = __PID_UNDEF ;
  //msg.msg_func = __KERNEL_MSG_FREE ; 
  //*((uint32_t *)msg.msg_buff) = (uint32_t )ptr ;
  //msg_send( __KERNEL_PID, &msg ) ;
}

// 发送消息
bool_t 
msg_send( pid_t pid, msg_t *msg )
{
  bool_t retval ;
  asm volatile ( \
    "int $0x30" : \
    "=a"(retval): \
    "a"(SYS_SEND_MSG),"b"(pid),"c"(msg) : \
    "memory" ) ;

  return retval ;
}
// 接收消息
msg_t *
msg_recv( pid_t pid_recv_from, uint32_t msg_func, msg_type_t msg_type )
{
  msg_t *msg ;
  asm volatile ( \
    "int $0x30" : \
    "=a"(msg): \
    "a"(SYS_RECV_MSG),"b"(pid_recv_from),"c"(msg_func),"d"(msg_type) : \
    "memory" ) ;

  return msg ;
}

// 向内核注册服务
bool_t 
regist_task( char *task_alias )
{
  //pid_t pid ;
  msg_t msg, *msg_get ;
  msg_reg_t *msg_reg = (msg_reg_t *)(msg.msg_buff) ;
  bool_t retval ;

  if( task_alias == NULL )
    return FALSE ;

  //pid = getpid( ) ;


  msg.sender = __PID_UNDEF ;
  msg.msg_func = __KERNEL_MSG_REGIST ; 

  //msg_reg->reciever = pid ;
  memcpy( msg_reg->task_alias, task_alias, __MSG_TASK_ALIAS ) ;
  msg_reg->task_alias[__MSG_TASK_ALIAS-1] = '\0' ;

  if( msg_send( __KERNEL_PID, &msg ) == FALSE )
    return FALSE ;

  get_message :
    msg_get = msg_recv( __KERNEL_PID, __KERNEL_MSG_REGIST, KERNEL) ;
    if( msg_get == NULL )
      return FALSE ;
    if( msg_get->msg_func == __MSG_FUNC_UNEXP )
    {
      free( (void *)msg_get ) ;
      sleep(1) ;
      goto get_message ;
    }

  retval = *(msg_reg_ret_t *)(msg_get->msg_buff) ;

  free( (void *)msg_get ) ;

  return retval ;
}

// 进程退出
void 
exit( void )
{
  msg_t msg ;
  msg.sender = __PID_UNDEF ;
  msg.msg_func = __KERNEL_MSG_PROC_DESTORY ;
  msg_send( __KERNEL_PID, &msg ) ;
  
  while( TRUE )
    free( msg_recv( __KERNEL_PID, __KERNEL_MSG_PROC_DESTORY, KERNEL) ) ;
}

// 进程休眠
void
sleep( uint32_t time )
{
  msg_t msg, *msg_get ;
  /*
  asm volatile ( \
    "int $0x30" : \
    "=a"(pid) : \
    "a"(SYS_GETPID) : \
    "memory" ) ;
  */
  msg.sender = __PID_UNDEF ;
  msg.msg_func = __KERNEL_MSG_SLEEP ; 

  *((uint32_t *)(msg.msg_buff)) = time ;

  if( msg_send( __KERNEL_PID, &msg ) == FALSE )
    return ;

  get_message :
    msg_get = msg_recv( __KERNEL_PID, __KERNEL_MSG_WAKEUP, INTR ) ;
    if( msg_get == NULL )
      return ;
    if( msg_get->msg_func == __MSG_FUNC_UNEXP )
    {
      free( (void *)msg_get ) ;
      //sleep(1);
      goto get_message ;
    }

  free( (void *)msg_get ) ;
}
