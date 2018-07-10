#ifndef __DEBUG_H
#define __DEBUG_H


void panic_spin( char *filename, \
                 int line, \
                 const char *func, \
                 const char *condition ) ;

/* 
 * __VA_ARGS__ 是预处理器锁支持的专用标识符.
 * 代表所有与省略号向对应的参数
 * "..."表示定义的宏其参数可变
 */
#define PANIC( ... ) \
  panic_spin( __FILE__, __LINE__, __func__, __VA_ARGS__ )
// 预定义宏
// __FILE__ 被编译的文件名
// __LINE__ 被编译文件中的行号
// __func__ 被编译的函数名 

#ifdef NO_DEBUG
  #define assert( condition ) ((void )0)
#else
  #define assert( condition ) \
    if( !(condition) ) PANIC( #condition )
// #condition 把condition转换成字符串常量
// 如 condition 为 var != 0,则 #condition 变成字符串 "var != 0"
#endif // NO_DEBUG



#endif // __DEBUG_H