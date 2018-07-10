#ifndef __KERNEL_SERVICE_H
#define __KERNEL_SERVICE_H
#include "proc.h"
#include "msg.h"



// 内核进程ID
#define __KERNEL_PID                0U
// 内核服务消息类型
#define __KERNEL_MSG_SEARCH         0x00U //用于查询消息类型的消息功能号
#define __KERNEL_MSG_REGIST         0x01U //用于注册消息的消息类型
#define __KERNEL_MSG_MALLOC         0x02U //用于进程申请内存
#define __KERNEL_MSG_FREE           0x03U //用于进程释放内存
#define __KERNEL_MSG_GETPID         0x04U //用于获取进程id
#define __KERNEL_MSG_SLEEP          0x05U //进程休眠,由进程发送消息,单位秒
#define __KERNEL_MSG_WAKEUP         0x06U //进程唤醒,由时间中断处理程序发送
// 创建进程的时候
//  1.用户进程已经将ELF二进制文件读取到内存中
//  2.内核分配pcb,pdt
//  3.内核重定向elf文件
#define __KERNEL_MSG_PROC_CREATE    0x07U //用户创建新进程
#define __KERNEL_MSG_PROC_DESTORY   0x08U //进程销毁

// 用户或者服务进程的默认入口地址
#define __USER_TASK_ENTRY           0x40002000U

// 下面定义各个内核服务消息所使用的数据结构,这些数据结构存放在 msg_buff中
typedef char msg_search_t ;         // __KERNEL_MSG_SEARCH
typedef struct s_msg_reg            // __KERNEL_MSG_REGIST
{
  //pid_t reciever ;
  char task_alias[__MSG_TASK_ALIAS] ;
} msg_reg_t ;
typedef struct s_msg_proc_create    // __KERNEL_MSG_PROC_CREATE
{
  pcb_t *parent_pcb ;               // 创建子进程的父进程的pcb
  proc_type_t ptype ;
  uint32_t file_size ;
  char proc_name[__MAX_PROC_NAME] ;
  uint32_t elf_vaddr ;                 // 父进程的将elf文件读入的父进程的虚拟地址
} msg_proc_create_t ;
typedef pid_t msg_proc_destory_t ;  // __KERNEL_MSG_PROC_DESTORY
                                    // 直接使用发送者的pid(sender)
// 下面定义各个内核服务消息返回的数据结构,这些数据结构存放在 msg_buff中
typedef pid_t msg_search_ret_t ;   // __KERNEL_MSG_SEARCH
typedef bool_t msg_reg_ret_t ;        // __KERNEL_MSG_REGIST
typedef bool_t msg_proc_create_ret_t ;// __KERNEL_MSG_PROC_CREATE
// __KERNEL_MSG_PROC_DESTORY 不返回


#endif