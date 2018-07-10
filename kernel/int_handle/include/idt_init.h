#ifndef __IDT_INIT_H
#define __IDT_INIT_H
#include "global_type.h"

#define __IDT_DESC_COUNT      0x31U     // 最后一个 0x30 是系统调用
#define __IDT_DESC_ATTR_DPL0  0x8eU     
#define __IDT_DESC_ATTR_DPL3  0xEEU     // 用户系统调用

void idt_init( void ) ;
#endif