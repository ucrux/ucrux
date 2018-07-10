#ifndef __STDIO_H
#define __STDIO_H
#include "global_type.h"

typedef char * va_list ;

//单个栈原素的长度,32bit计算机的单个栈原素的长度为4
#define __STACK_ITEM_LEN 4




//获取可变参数
#define va_start(arg_ap, first_begin) ((arg_ap) = (va_list)(&(first_begin)))
#define va_arg( arg_ap, arg_type) (*((arg_type *)((arg_ap) += __STACK_ITEM_LEN)))
#define va_end( arg_ap ) ((arg_ap) = NULL)

//第二个参数事buf的大小
uint32_t vsnprintf(char* buf, uint32_t bufsize, const char* format, va_list ap);
uint32_t snprintf(char* buf, uint32_t bufsize, const char* format, ...);
void printf( const char *format, ... ) ;

#endif
