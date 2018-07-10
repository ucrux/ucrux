/*此文件保存一些全局变量*/
#ifndef __KERNEL_H
#define __KERNEL_H
#include "global_type.h"

/*
 * 1.内核是不可重入的
 * 2.在使用信号量的时候,进程被调度出去,在被调度回来的时候如何回到原点
 * 3.系统调用就3个,内存分配,发送消息,接收消息
 * 4.由于有消息队列,所以发送消息的时候不会被阻塞(内核是不可重入的),
 *   但是接收消息的时候可能被阻塞,因为消息队列有可能是空的(这个时候进程就被阻塞在内核态了,哈哈哈,还说内核是不可重入的)
 */

// 页大小
#define __PAGE_SIZE                 0x00001000U
// 页目录表项和页表项大小
#define __PTE_SIZE                  0x04U
// 指向gdt_ptr结构的指针的指针
#define __GDT_PTR_ADDR              0x00007004U
// 内核主线程PCB
#define __KERNEL_PCB                0x00006000U
// 内核栈起始地址
#define __KERNEL_STACK              0x00007000U
// 指向总内存大小的指针
#define __MEM_SIZE_ADDR             (__KERNEL_STACK)
// 指向页目录表的指针
#define __PAGE_DIR_TABLE_ADDR       0x00008000U
// 指向初始化页表项表的指针
#define __PAGE_INIT_ENT_TABLE_ADDR  0x00009000U
// 初始化页表时最大的线性地址(如果物理内存比1M大)
#define __PAGE_INIT_MAX_LINE_ADDR   0x00400000U
// 指向TSS任务状态段结构的指针
#define __TSS_ADDR                  0x00007100U
// 内核初始化时可用内存的开始位置
#define __INIT_BEGIN_ADDR           0x00100000U
// 内核代码段选择子
#define __SELECTOR_KERNEL_CODE      0x08U
// 内核数据段选择子
#define __SELECTOR_KERNEL_DATA      0x10U
// 内核视频段选择子
#define __SELECTOR_KERNEL_VIDEO     0x18U
// 服务进程视频段选择子
#define __SELECTOR_TASK_VIDEO       0x19U
// task进程代码段选择子
#define __SELECTOR_TASK_CODE        0x29U
// task进程数据段选择子
#define __SELECTOR_TASK_DATA        0x31U
// user进程代码段选择子
#define __SELECTOR_USER_CODE        0x3BU
// user进程数据段选择子
#define __SELECTOR_USER_DATA        0x43U
// 内核最大地址空间
#define __KERNEL_MAX_ADDR           0x40000000U
// 最大虚拟地址空间
#define __VADDR_MAX                 0xffffffffU
// 函数入口,用户进程函数默认入口
#define __FUNC_ENTRY                0x00000000U
// 默认的eflags值
#define __DEFAULT_EFLAGS            0x00001202U  // IOPL=1, IF
// 内核最大拥有的页目录表项
#define __KERNEL_MAX_PDT_ECNT       256U
// 服务进程文件默认大小
#define __TASK_FILE_DEFAULT_SZIE    (8*__PAGE_SIZE)
// video task elf 文件在硬盘中的起始扇区
#define __VIDEO_TASK_BEGIN_SEC      300
// tty task elf 文件在硬盘中的起始扇区
#define __TTY_TASK_BEGIN_SEC        400
// keyboard task elf 文件在硬盘中的起始扇区
#define __KEYBOARD_TASK_BEGIN_SEC   500




//门描述符结构体
typedef struct s_gate_desc
{
  uint16_t func_offset_low ;
  uint16_t cs_selector ;
  uint8_t param_count ;
  uint8_t gate_attr ;
  uint16_t func_offset_high ;
} gate_desc_t ;

#endif