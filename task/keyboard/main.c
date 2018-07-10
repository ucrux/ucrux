#include "keyboard.h"
#include "syscall.h"
#include "proc.h"
#include "video.h"
#include "msg.h"
#include "tty.h"
#include "global_type.h"
#include "string.h"

//debug
//#include "kputx.h"
//enddebug



/* 用转义字符定义部分控制字符 */
#define ESC         '\033'      // 八进制表示字符,也可以用十六进制'\x1b'
#define TAB         '\t'        // 制表符算4个空格
#define BACKSPACE   '\b'
#define ENTER       '\n'
#define DELETE      '\177'      // 八进制表示字符,十六进制为'\x7f'

/* 以上不可见字符一律定义为0 */
#define CHAR_INVISIBLE    0
#define CTRL_L_CHAR       CHAR_INVISIBLE
#define CTRL_R_CHAR       CHAR_INVISIBLE
#define SHIFT_L_CHAR      CHAR_INVISIBLE
#define SHIFT_R_CHAR      CHAR_INVISIBLE
#define ALT_L_CHAR        CHAR_INVISIBLE
#define ALT_R_CHAR        CHAR_INVISIBLE
#define CAPS_LOCK_CHAR    CHAR_INVISIBLE

/* 定义控制字符的通码和断码 */
#define SHIFT_L_MAKE      0x2a
#define SHIFT_R_MAKE      0x36
#define ALT_L_MAKE        0x38
#define ALT_R_MAKE        0xe038
#define CTRL_L_MAKE       0x1d
#define CTRL_R_MAKE       0xe01d
#define CAPS_LOCK_MAKE    0x3a


/* 定义以下变量记录相应键是否按下的状态,
 * ext_scancode用于记录makecode是否以0xe0开头 */


/* 键盘中断处理程序 */
int
main( void )
{
  bool_t ctrl_status = FALSE, \
         shift_status = FALSE, \
         caps_lock_status = FALSE, \
         ext_scancode = FALSE ;
  //暂时没有使用alt
  //bool_t alt_status = FALSE ;

  // 用于向tty发送消息
  msg_t keyboard_msg ;

  /* 以通码make_code为索引的二维数组 */
  char keymap[][2] = {
  /* 扫描码      未与shift组合   与shift组合   */
  /* ------------------------------------- */
  /* 0x00 */ {         0,         0         },
  /* 0x01 */ {        ESC,       ESC        },
  /* 0x02 */ {        '1',       '!'        },   
  /* 0x03 */ {        '2',       '@'        },   
  /* 0x04 */ {        '3',       '#'        },   
  /* 0x05 */ {        '4',       '$'        },   
  /* 0x06 */ {        '5',       '%'        },   
  /* 0x07 */ {        '6',       '^'        },   
  /* 0x08 */ {        '7',       '&'        },   
  /* 0x09 */ {        '8',       '*'        },   
  /* 0x0A */ {        '9',       '('        },   
  /* 0x0B */ {        '0',       ')'        },   
  /* 0x0C */ {        '-',       '_'        },   
  /* 0x0D */ {        '=',       '+'        },   
  /* 0x0E */ {     BACKSPACE, BACKSPACE     }, 
  /* 0x0F */ {        TAB,       TAB        },   
  /* 0x10 */ {        'q',       'Q'        },   
  /* 0x11 */ {        'w',       'W'        },   
  /* 0x12 */ {        'e',       'E'        },   
  /* 0x13 */ {        'r',       'R'        },   
  /* 0x14 */ {        't',       'T'        },   
  /* 0x15 */ {        'y',       'Y'        },   
  /* 0x16 */ {        'u',       'U'        },   
  /* 0x17 */ {        'i',       'I'        },   
  /* 0x18 */ {        'o',       'O'        },   
  /* 0x19 */ {        'p',       'P'        },   
  /* 0x1A */ {        '[',       '{'        },   
  /* 0x1B */ {        ']',       '}'        },   
  /* 0x1C */ {       ENTER,     ENTER       },
  /* 0x1D */ {   CTRL_L_CHAR, CTRL_L_CHAR   },
  /* 0x1E */ {        'a',       'A'        },   
  /* 0x1F */ {        's',       'S'        },   
  /* 0x20 */ {        'd',       'D'        },   
  /* 0x21 */ {        'f',       'F'        },   
  /* 0x22 */ {        'g',       'G'        },   
  /* 0x23 */ {        'h',       'H'        },   
  /* 0x24 */ {        'j',       'J'        },   
  /* 0x25 */ {        'k',       'K'        },   
  /* 0x26 */ {        'l',       'L'        },   
  /* 0x27 */ {        ';',       ':'        },   
  /* 0x28 */ {        '\'',      '"'        },   
  /* 0x29 */ {        '`',       '~'        },   
  /* 0x2A */ {  SHIFT_L_CHAR, SHIFT_L_CHAR  }, 
  /* 0x2B */ {        '\\',      '|'        },   
  /* 0x2C */ {        'z',       'Z'        },   
  /* 0x2D */ {        'x',       'X'        },   
  /* 0x2E */ {        'c',       'C'        },   
  /* 0x2F */ {        'v',       'V'        },   
  /* 0x30 */ {        'b',       'B'        },   
  /* 0x31 */ {        'n',       'N'        },   
  /* 0x32 */ {        'm',       'M'        },   
  /* 0x33 */ {        ',',       '<'        },   
  /* 0x34 */ {        '.',       '>'        },   
  /* 0x35 */ {        '/',       '?'        },
  /* 0x36 */ {  SHIFT_R_CHAR, SHIFT_R_CHAR  }, 
  /* 0x37 */ {        '*',       '*'        },     
  /* 0x38 */ {    ALT_L_CHAR, ALT_L_CHAR    },
  /* 0x39 */ {        ' ',       ' '        },   
  /* 0x3A */ {CAPS_LOCK_CHAR, CAPS_LOCK_CHAR}
  /*其它按键暂不处理*/
  };

  bool_t shift ;   // 判断是否与shift组合,用来在一维数组中索引对应的字符
  /* 这次中断发生前的上一次中断,以下任意三个键是否有按下 */
  bool_t break_code ;
  uint16_t scancode ;  // 保存扫描码
  uint16_t make_code ; // 保存选通码
  uint8_t index ;      // keymap中的索引
  char cur_char ;      // 通码对应的字符
  msg_t *msg ;         // 从键盘中断获取的消息
  pid_t v_pid ; // 键盘驱动自己的pid,显示驱动的pid
  pid_t tty_pid ;      // tty服务进程的pid

  //debug
  //kputstr( "keyboard start\n" ) ;
  //enddebug

  shift = FALSE ;
  v_pid = __PID_UNDEF ;
  
  // 获取video task的pid,用于注册服务进程时报错
  while( v_pid == __PID_UNDEF )
    v_pid = task_search( __VIDEO_TASK_ALIAS ) ;

  //k_pid = getpid( ) ;

  

  keyboard_msg.sender = __PID_UNDEF ;
  keyboard_msg.msg_func = __VIDEO_FUN_PRINT_STRING ;


  if( regist_task( __KEYBOARD_ALIAS ) == FALSE )
  {
    strcpy( (char *)keyboard_msg.msg_buff, "!!! keyboard task can NOT regist !!!\n" ) ;
    msg_send( v_pid, &keyboard_msg ) ;
    exit( ) ;
  }
   
  tty_pid = __PID_UNDEF ;
  // 获取tty进程的pid
  while( tty_pid == __PID_UNDEF ) //没有获取到 tty 的 pid
    tty_pid = task_search( __TTY_TASK_ALIAS ) ;
 
  //debug
  //kputstr( "keyboard pid:" ); kput_uint32_dec((uint32_t )k_pid); kputchar('\n');
  //kputstr( "video pid:" ); kput_uint32_dec((uint32_t )v_pid); kputchar('\n');
  //kputstr( "tty pid:" ); kput_uint32_dec((uint32_t )tty_pid); kputchar('\n');
  //enddebug

  while( TRUE )
  {
    //debug
    //kputstr( "k reciever\n" );
    //enddebug
    msg = msg_recv( __KERNEL_PID, __KEYBOARD_GET_SCANCODE, INTR ) ;
    //debug
    //kputstr( "recieve done\n" );
    //enddebug
    if( msg == NULL )
      exit() ;

    switch (msg->msg_func)
    {
      case __KEYBOARD_GET_SCANCODE :
        scancode = (uint16_t )msg->msg_buff[0] ;
        /* 
         * 若扫描码是e0开头的,表示此键的按下将产生多个扫描码,
         * 所以马上结束此次中断处理函数,等待下一个扫描码进来
         */ 
        if( scancode == 0xe0 )
        { 
          ext_scancode = TRUE ;    // 打开e0标记
          break ;
        }
          
        /* 如果上次是以0xe0开头,将扫描码合并 */
        if( ext_scancode )
        {
          scancode = ((0xe000) | scancode);
          ext_scancode = FALSE;   // 关闭e0标记
        }   
         
        break_code = ( (scancode & 0x0080) != 0 ) ;   // 获取是否为break_code
         
        if( break_code )                              // 若是断码break_code(按键弹起时产生的扫描码)
        {   
         /* 由于ctrl_r 和alt_r的make_code和break_code都是两字节,
            所以可用下面的方法取make_code,多字节的扫描码暂不处理 */
          make_code = (scancode &= 0xff7f) ;   // 得到其make_code(按键按下时产生的扫描码)
          
         /* 若是任意以下三个键弹起了,将状态置为false */
          if( make_code == CTRL_L_MAKE || make_code == CTRL_R_MAKE )
            ctrl_status = FALSE ;
          else if( make_code == SHIFT_L_MAKE || make_code == SHIFT_R_MAKE )
            shift_status = FALSE ;
          //else if (make_code == ALT_L_MAKE || make_code == ALT_R_MAKE)
          //  alt_status = FALSE ;
         /* 由于caps_lock不是弹起后关闭,所以需要单独处理 */
         
          break ;   // 直接返回结束此次中断处理程序
        } 
        /* 若为通码,只处理数组中定义的键以及alt_right和ctrl键, 全是make_code */
        else if( (scancode > 0x00 && scancode < 0x3b) || \
                 (scancode == ALT_R_MAKE) || (scancode == CTRL_R_MAKE) ) 
        {
          if( (scancode < 0x0e)  || (scancode == 0x29) || \
              (scancode == 0x1a) || (scancode == 0x1b) || \
              (scancode == 0x2b) || (scancode == 0x27) || \
              (scancode == 0x28) || (scancode == 0x33) || \
              (scancode == 0x34) || (scancode == 0x35) ) 
          {  
            /****** 代表两个字母的键 ********
               0x0e 数字'0'~'9',字符'-',字符'='
               0x29 字符'`'
               0x1a 字符'['
               0x1b 字符']'
               0x2b 字符'\\'
               0x27 字符';'
               0x28 字符'\''
               0x33 字符','
               0x34 字符'.'
               0x35 字符'/' 
            *******************************/
            if( shift_status )          // 如果同时按下了shift键
              shift = TRUE ;
          }
          else                            // 默认为字母键
          {                          
            if( shift_status && caps_lock_status )       // 如果shift和capslock同时按下
              shift = FALSE ;
            else if( shift_status || caps_lock_status )  // 如果shift和capslock任意被按下
              shift = TRUE ;
            else
              shift = FALSE ;
          }
           
          index = (uint8_t )(scancode &= 0x00ff) ;     // 将扫描码的高字节置0,主要是针对高字节是e0的扫描码.
          cur_char = keymap[index][shift] ;  // 在数组中找到对应的字符
          //debug
          //kputstr( "get make_code begin:" );kputchar( cur_char );kputchar('\n');
          //kputstr( "scancode:" );kput_uint32_hex( (uint32_t)scancode );kputchar('\n');
          //kputstr( "index:" );kput_uint32_hex( (uint32_t)index );kputchar('\n');
          //kputstr( "shift:" );kput_uint32_hex( (uint32_t)shift );kputchar('\n');
          //enddebug
          /* 如果cur_char不为0,也就是ascii码为除'\0'外的字符就加入键盘缓冲区中 */
          if( cur_char )
          {
            
            /*****************  快捷键ctrl+l和ctrl+u的处理 *********************
             * 下面是把ctrl+d组合键产生的字符置为:
             * cur_char的asc码-字符a的asc码, 此差值比较小,
             * 属于asc码表中不可见的字符部分.故不会产生可见字符.
             * 我们在shell中将ascii值为c-d,删除一行输入
             */
            if( (ctrl_status && cur_char == 'd') )
              cur_char -= 'a' ;
            /****************************************************************/
            
            /* 将普通字符或者特殊含义的组合键发送给tty进程 */
            // 初始化 keyboard_msg
            keyboard_msg.sender = __PID_UNDEF ; //中断发出的消息都是由键盘驱动发出去的发出去的
            keyboard_msg.msg_func = __TTY_GET_KEYBOARD ;
            keyboard_msg.msg_buff[0] = (uint8_t )cur_char ;
          
            //debug
            //kputstr( "tty pid:" ); kput_uint32_dec((uint32_t )tty_pid); kputchar('\n');
            //enddebug

            msg_send( tty_pid, &keyboard_msg ) ;
          
            break ;
          }
          
          /* 记录本次是否按下了下面几类控制键之一,供下次键入时判断组合键 */
          // scancode的高两位被清零了,所以不需要判断 CTRL_R_MAKE, ALT_R_MAKE
          if( scancode == CTRL_L_MAKE )
            ctrl_status = TRUE ;
          else if( scancode == SHIFT_L_MAKE || scancode == SHIFT_R_MAKE )
            shift_status = TRUE ;
          //else if( scancode == ALT_L_MAKE )
          //  alt_status = TRUE ;
          else if( scancode == CAPS_LOCK_MAKE )
            /* 不管之前是否有按下caps_lock键,当再次按下时则状态取反,
             * 即:已经开启时,再按下同样的键是关闭。关闭时按下表示开启。*/
            caps_lock_status = !caps_lock_status ;

        }
        break ;
    }
    free((void *)msg) ;
    msg = NULL ;
  }
  return 0 ;
}

