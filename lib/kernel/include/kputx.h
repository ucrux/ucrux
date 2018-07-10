#ifndef __KLIBC_H
#define __KLIBC_H
/*
在屏幕上输出字符,字符串及整数
*/
#include "global_type.h"
//打印一个字符
void kputchar( char ch ) ;
//打印字符串,以'\0'为结尾字符
void kputstr( const char *str ) ;
//以2进制打印一个uint32_t类型
void kput_uint32_bin( uint32_t num ) ;
//以16进制形式打印一个uint32_t类型
void kput_uint32_hex( uint32_t num ) ;
//以10进制打印一个uint32_t类型
void kput_uint32_dec( uint32_t num ) ;


#endif
