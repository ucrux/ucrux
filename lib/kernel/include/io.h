#ifndef __IO_H
#define __IO_H
#include "global_type.h"

// 从端口读取一个字节到 val
static inline uint8_t
inb( uint16_t port )
{
  uint8_t val ;
  asm volatile ( "inb %w1, %b0; nop; nop" : "=a"(val) : "d"(port) ) ;
  return val ;
}
// 把val的值向端口写入一个字节
static inline void
outb( uint8_t val, uint16_t port )
{
  asm volatile ( "outb %b1, %w0; nop; nop" : : "d"(port), "a"(val) ) ;
}
// 从端口读取一个字到val
static inline uint16_t
inw( uint16_t port )
{
  uint16_t val ;
  asm volatile ( "inw %w1, %w0; nop; nop" : "=a"(val) : "d"(port) ) ;
  return val ;
}
// 把val向端口写入一个字
static inline void
outw( uint16_t val, uint16_t port )
{
  asm volatile ( "outw %w1, %w0; nop; nop" : : "d"(port), "a"(val) ) ;
}
// 一次性向端口读取count个字节,保存到以buff为首的地址当中
static inline void
insb( void *buff, uint16_t port, uint32_t count )
{
  asm volatile ( \
    "cld ; \
    rep insb; nop; nop" : : \
    "D"(buff),"d"(port),"c"(count) : \
    "cc","memory" ) ;
}
// 一次性从buff为首的地址向端口写入count个字节
static inline void
outsb( void *buff, uint16_t port, uint32_t count )
{
  asm volatile ( \
    "cld ; \
    rep outsb; nop; nop" : : \
    "S"(buff),"d"(port),"c"(count) : \
    "cc","memory" ) ;
}
// 一次性向端口读取count个字,保存到以buff为首的地址当中
static inline void
insw( void *buff, uint16_t port, uint32_t count )
{
  asm volatile ( \
    "cld ; \
    rep insw; nop; nop" : : \
    "D"(buff),"d"(port),"c"(count) : \
    "cc","memory" ) ;
}
// 一次性从buff为首的地址向端口写入count个字
static inline void
outsw( void *buff, uint16_t port, uint32_t count )
{
  asm volatile ( \
    "cld ; \
    rep outsw; nop; nop" : : \
    "S"(buff),"d"(port),"c"(count) : \
    "cc","memory" ) ;
}
#endif