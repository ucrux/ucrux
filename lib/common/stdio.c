#include "stdio.h"
#include "string.h"


/* 将整型转换成字符(integer to ascii) */
static void itoa(uint32_t value, char** buf_ptr_addr, int32_t *buf_ptr_size, uint8_t base) ;


uint32_t
vsnprintf(char* buf, uint32_t bufsize, const char* format, va_list ap)
{
  
  uint32_t loop ;
  char* buf_ptr = buf;
  const char* index_ptr = format;
  char index_char = *index_ptr;
  int32_t arg_int;
  char* arg_str;
  int32_t remain_size ;

  for( loop = 0 ; loop < bufsize ; loop++)
    buf[loop] = '\0';

  while(index_char)
  {
    if(index_char != '%')
    {
      if( strlen(buf) >= bufsize - 1 )
        return strlen(buf);

      *(buf_ptr++) = index_char;
      index_char = *(++index_ptr);
      continue;
    }
    
    index_char = *(++index_ptr);   // 得到%后面的字符
    switch (index_char)
    {
      case 's':
        arg_str = va_arg(ap, char*);
        if( strlen(buf) + strlen(arg_str) >= bufsize - 1)
        {
          memcpy(buf_ptr, arg_str, bufsize-strlen(buf)-1 );
          return strlen(buf);
        }
        
        strcpy(buf_ptr, arg_str);
        buf_ptr += strlen(arg_str);
        index_char = *(++index_ptr);
        break;

      case 'c':
        if( strlen(buf) >= bufsize - 1 )
          return strlen(buf);
        *(buf_ptr++) = va_arg(ap, char);
        index_char = *(++index_ptr);
        break;

      case 'd':
        arg_int = va_arg(ap, int);
        /* 若是负数, 将其转为正数后,再正数前面输出个负号'-'. */
        if (arg_int < 0)
        {
          arg_int = 0 - arg_int;
          *buf_ptr++ = '-';
        }
        remain_size = bufsize-strlen(buf)-1;
        itoa(arg_int, &buf_ptr, &remain_size, 10); 
        index_char = *(++index_ptr);
        break;

      case 'x':
        arg_int = va_arg(ap, int);
        remain_size = bufsize-strlen(buf)-1;
        itoa(arg_int, &buf_ptr, &remain_size, 16); 
        index_char = *(++index_ptr); // 跳过格式字符并更新index_char
        break;
    }
  }
  return strlen(buf);
}

uint32_t
snprintf(char* buf, uint32_t bufsize, const char* format, ...)
{
  va_list args;
  uint32_t retval;
  va_start(args, format);
  retval = vsnprintf(buf, bufsize, format, args);
  va_end(args);
  return retval;
}
/*
void
printf( const char *format, ... )
{
  char buf[1024];
  msg_t msg;
  pid_t t_pid ;  // tty的pid
  if( (t_pid = task_search( __TTY_TASK_ALIAS )) == __PID_UNDEF ) //tty进程没有启动
    return ;

  va_list args;
  va_start(args, format);
  vsnprintf(buf, 1024, format, args);
  va_end(args);
  //向tty进程发送字符串
  msg.sender = __PID_UNDEF ;
  msg.msg_func = __TTY_PRINT_STR ;
  strcpy( (char *)(msg.msg_buff), buf );
  msg_send( t_pid, &msg ) ;
}
*/

static void 
itoa(uint32_t value, char** buf_ptr_addr, int32_t *buf_ptr_size, uint8_t base )
{
   uint32_t m = value % base;     // 求模,最先掉下来的是最低位   
   uint32_t i = value / base;     // 取整
   
   if( *buf_ptr_size <= 0)
    return ;

   if (i) 
   {         // 如果倍数不为0则递归调用。
      itoa(i, buf_ptr_addr, buf_ptr_size, base);
   }

   if (m < 10)
   {  
      // 如果余数是0~9
      *((*buf_ptr_addr)++) = m + '0';   // 将数字0~9转换为字符'0'~'9'
      (*buf_ptr_size)--;
   } 
   else
   {       // 否则余数是A~F
      *((*buf_ptr_addr)++) = m - 10 + 'A'; // 将数字A~F转换为字符'A'~'F'
      (*buf_ptr_size)--;
   }
}