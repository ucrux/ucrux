#ifndef __STRING_H
#define __STRING_H

#include "global_type.h"
// 串操作相关

// 将以 begin_addr 为起始地址,设置 size 长度的内存为 val, 以字节为单位
void memset( void *begin_addr, uint8_t val, uint32_t size ) ;
// 将src的前size个字节拷贝到dst中
void memcpy( void *dst, const void *src, uint32_t size ) ;
// 比较两段内存前 size个字节.
// src > dest,return 1 ;
// src == dest,return 0 ;
// src < dest, return -1 ;
int memcmp( const void *src, const void *dst, uint32_t size ) ;
// 字符串长度
uint32_t strlen( const char *str ) ;
// 字符串复制
void strcpy( char *dst, const void *src ) ;
// 字符串比较
int strcmp( const char *str1, const char *str2 ) ;
// 将src连接到dst后面,返回连接后的地址,调用者须保证dst的内存空间足够容纳两个字符串
char *strcat( char *dst, const char *src ) ;
// 从左向右查找第一次出现ch的地址
char *strchr( const char *str, char ch ) ;
// 从右到左查找第一次出现ch的地址
char *strrchr( const char *str, char ch ) ;
// 查找ch再字符串中出现的次数
uint32_t strchrs( const char *str, char ch ) ;


#endif