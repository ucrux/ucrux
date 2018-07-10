#ifndef __SYSCALL_H
#define __SYSCALL_H

#include "global_type.h"
#include "proc.h"
#include "kernel_service.h"


/*以下关于消息的数据结构和 msg.h 一样*/
#define __MSG_MAX_FUNC    8U    //一个服务进程最大支持的功能数
#define __MSG_TASK_ALIAS  24U   //task进程的别名,包含'\0'
#define __MSG_MAX_BUFF    1536U //最多可以保存3个扇区




/*
 * 系统调用的入口是 int 0x30
 * eax 系统调用号
 * ebx 系统调用第一个参数
 * ecx 系统调用第二个参数
 * edx 系统调用第三个参数
 */

// 以下函数可在用户空间调用
// 获取自己的pid
pid_t getpid( void ) ;
// 寻找task进程的pid
pid_t task_search( char *task_alias ) ;
// 分配内存
void *malloc( uint32_t size ) ;
// 释放内存
void free( void *ptr ) ;
// 发送消息
bool_t msg_send( pid_t pid, msg_t *msg ) ;
// 接收消息
msg_t *msg_recv( pid_t pid_recv_from, uint32_t msg_func, msg_type_t msg_type ) ;
// 向内核注册服务
bool_t regist_task( char *task_alias ) ;
// 进程退出
void exit( void ) ; 
// 进程休眠,单位秒
void sleep( uint32_t time ) ;
#endif