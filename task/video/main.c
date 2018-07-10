#include "syscall.h"
#include "kputx.h"
#include "video.h"
#include "global_type.h"
#include "msg.h"


// 显示驱动主进程
int
main( void )
{
  msg_t *msg = NULL ;
  if( regist_task( __VIDEO_TASK_ALIAS ) == FALSE )
  {
    kputstr( "!!! video task can NOT regist !!!\n" ) ;
    exit( ) ;
  }

  /*
  // debug
  while( 1 )
  {
  msg = malloc( 100 ) ;
  kputstr( "msg addr: " ); kput_uint32_hex( (uint32_t )msg ); kputchar('\n') ;
  // enddebug
  free( (void *)msg ) ;
  msg = NULL ;
  // debug
  } 
  // enddebug
  */

  while( TRUE )
  {
    // debug
    //kputstr( "begin recieving message\n") ;
    // enddebug
    msg = msg_recv( __PID_UNDEF, __MSG_FUNC_UNDEF, PROC) ;
    if( msg == NULL )  //没有内存咯
      exit( ) ;
    //debug
    //sleep( 100 ) ;
    //enddebug
    switch (msg->msg_func)
    {
      case __VIDEO_FUN_PRINT_STRING :
        kputstr( (char *)(msg->msg_buff) ) ;
        break ;
      case __VIDEO_FUN_PRINT_CHAR :
        kputchar( (char )(msg->msg_buff[0]) ) ;
        break ;
      case __VIDEO_FUN_PRINT_NUM_HEX :
        kput_uint32_hex( *((uint32_t *)(msg->msg_buff)) ) ;
        break ;
      case __VIDEO_FUN_PRINT_NUM_DEC :
        kput_uint32_dec( *((uint32_t *)(msg->msg_buff)) ) ;
        break ;
    }
    // debug
    //kputstr( "video pid: " ); kput_uint32_dec( (uint32_t )getpid() ); //kputchar('\n') ;
    // enddebug
    free( (void *)msg ) ;
    msg = NULL ;

    // debug
    //kputstr( "message handle done\n") ;
    // enddebug
  }

  return 0 ;
}


