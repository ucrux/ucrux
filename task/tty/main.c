#include "syscall.h"
#include "tty.h"
#include "global_type.h"
#include "msg.h"
#include "video.h"
#include "string.h"
#include "proc.h"

//debug
//#include "kputx.h"
//enddebug

#define __CHAR_BUFF_SIZE  256U       //buff里面最多保存255个字符
#define __PS1             "ucore > "
#define __DELETE_LINE     ('d'-'a')

typedef struct s_kb_buff
{
  uint32_t index ;   //字符个数等于index
  uint32_t tab_cnt ; //记录有多少个'\t'
  char keyboard_buff[__CHAR_BUFF_SIZE] ;
} kb_buff_t ;

static kb_buff_t kb_buff ;
static msg_t msg_out ;

static inline void init_kb_buff( void ) ;
static inline void print_ps1( pid_t v_pid ) ;


// 显示驱动主进程
int
main( void )
{
  msg_t *msg = NULL ;
  char kb_char ;
  pid_t v_pid = __PID_UNDEF ;
  uint32_t loop ;

  // debug
  //kputstr( "tty begin" ) ;
  // enddebug

  //初始化kb_buff
  init_kb_buff( ) ;
  // 获取video task的pid
  while( v_pid == __PID_UNDEF )
    v_pid = task_search( __VIDEO_TASK_ALIAS ) ;
  //t_pid = getpid( ) ;

  //debug
  //kputstr( "video pid:" ); kput_uint32_dec((uint32_t )v_pid); kputchar('\n');
  //kputstr( "tty pid:" ); kput_uint32_dec((uint32_t )t_pid); kputchar('\n');
  //enddebug

  msg_out.sender = __PID_UNDEF ;
  msg_out.msg_func = __VIDEO_FUN_PRINT_STRING ;


  if( regist_task( __TTY_TASK_ALIAS ) == FALSE )
  {
    strcpy( (char *)msg_out.msg_buff, "!!! tty task can NOT regist !!!\n" ) ;
    msg_send( v_pid, &msg_out ) ;
    exit( ) ;
  }

  // 打印命令提示符
  print_ps1( v_pid ) ;

  while( TRUE )
  {
    msg = msg_recv( __PID_UNDEF, __MSG_FUNC_UNDEF, PROC ) ;
    if( msg == NULL )  //没有内存咯
      exit( ) ;

    //debug
    //sleep( 100 ) ;
    //enddebug

    switch (msg->msg_func)
    {
      case __TTY_GET_KEYBOARD :
        // 如果不是'\n'回车键,不是功能键,将字符保存到缓冲区
        kb_char = (char )(msg->msg_buff[0]) ;

        if( kb_char != __DELETE_LINE && \
            kb_char != '\b' && \
            kb_buff.index <= __CHAR_BUFF_SIZE - 1 )
        {
          if( kb_buff.index == __CHAR_BUFF_SIZE - 1 && kb_char != '\n' )
            break ;

          kb_buff.keyboard_buff[kb_buff.index] = (kb_char == '\n'  ? '\0' : kb_char ) ;
          kb_buff.index++ ;

          // 特殊处理
          if( kb_char == '\t' )
          {
            kb_buff.tab_cnt++ ;
            // 打印4个字符
            strcpy( (char *)msg_out.msg_buff, "    " ) ;
          }
          else
          {
            msg_out.msg_buff[0] = kb_char ;
            msg_out.msg_buff[1] = '\0' ;
            if( kb_char == '\n' )
            {
              // 输出当前字符
              msg_send( v_pid, &msg_out ) ;

              if( strlen(kb_buff.keyboard_buff) )
              {
                // 命令处理逻辑
                /*************************************************/
                /*                 命令处理逻辑代码块                */
                strcpy( (char *)msg_out.msg_buff, kb_buff.keyboard_buff ) ;
                strcat( (char *)msg_out.msg_buff, ": unkown commad\n" ) ;
                // 输出命令输出
                msg_send( v_pid, &msg_out ) ;
                /*************************************************/
              }

              // 输出命令输出后,打印命令提示符
              print_ps1( v_pid ) ;
              // 清空缓存区
              init_kb_buff( ) ;
              break ;
            }
          }
        }
        else if( kb_char == __DELETE_LINE )
        {
          //计算要删除的字符数
          for( loop = 0 ; loop < kb_buff.index + kb_buff.tab_cnt * 3 ; loop++ )
            msg_out.msg_buff[loop] = '\b' ;
          msg_out.msg_buff[loop] = '\0' ;
          // 重新初始化kb_buff
          init_kb_buff( ) ;
        }
        else if( kb_char == '\b' )
        {
          if( kb_buff.index == 0 )
            break ;
          kb_buff.index-- ;
          if( kb_buff.keyboard_buff[kb_buff.index] == '\t' )
          {
            strcpy( (char *)msg_out.msg_buff, "\b\b\b\b" ) ;
            kb_buff.tab_cnt-- ;
          }
          else
          {
            msg_out.msg_buff[0] = '\b' ;
            msg_out.msg_buff[1] = '\0' ;
          }
        }

        msg_send( v_pid, &msg_out ) ;
        break ;

      case __TTY_PRINT_STR :
        strcpy( (char *)msg_out.msg_buff, (const char *)(msg->msg_buff) ) ;
        msg_send( v_pid, &msg_out ) ;
        break ;
    }

    free( (void *)msg ) ;
    msg = NULL ;
  }

  return 0 ;
}

//局部函数
// 初始化 kb_buff
static inline void
init_kb_buff( void )
{
  uint32_t loop ;
  kb_buff.index = 0 ;
  kb_buff.tab_cnt = 0 ;

  for( loop = 0 ; loop < __CHAR_BUFF_SIZE ; loop++ )
    kb_buff.keyboard_buff[loop] = '\0' ;
}
// 打印命令提示符
// v_pid 为 video task的进程号
// t_pid 为 tty task 的进程号
static inline void
print_ps1( pid_t v_pid )
{
  msg_out.sender = __PID_UNDEF ;
  msg_out.msg_func = __VIDEO_FUN_PRINT_STRING ;
  strcpy( (char *)msg_out.msg_buff, __PS1 ) ;
  msg_send( v_pid, &msg_out ) ;
}


