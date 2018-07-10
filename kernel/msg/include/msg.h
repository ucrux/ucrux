#ifndef __MSG_H
#define __MSG_H
#include "global_type.h"
#include "dlist.h"
#include "proc.h"



/*
 * 消息发送
 *   将消息从用户空间拷贝到内核空间
 *   发送消息不会被阻塞
 *   因为内核空间不可重入
 *   发送消息时需要唤醒在这个消息队列上等待接收的进程
 * 消息接收
 *   将消息从内核空间拷贝到用户空间
 *   接收消息时,可能被阻塞,因为消息队列有可能为空
 * 消息注册
 *   创建服务进程时需要进行消息注册
 * 消息类型查询
 */


/*
#define __INTR_MSG    0x00000001U   //来自中断的消息
#define __KERNEL_MSG  0x00000002U   //来自内核的消息
#define __TASK_MSG    0x00000004U   //来自任务进程的消息
#define __PROC_MSG    0x00000008U   //来自进程的消息

#define IS_INTR_MSG(msg_type) \
  ( ((msg_type) & __INTR_MSG) != 0 ? TRUE : FALSE )
#define IS_KERNEL_MSG(msg_type) \
  ( ((msg_type) & __KERNEL_MSG) != 0 ? TRUE : FALSE )
#define IS_TASK_MSG(msg_type) \
  ( ((msg_type) & __TASK_MSG) != 0 ? TRUE : FALSE )
#define IS_PROC_MSG(msg_type) \
  ( ((msg_type) & __PROC_MSG) != 0 ? TRUE : FALSE )
*/

#define __MSG_FUNC_UNDEF  0xffffffffU   //为定义的工呢
#define __MSG_FUNC_UNEXP  0xfffffffeU   //不是想要的消息

#define __MSG_MAX_FUNC    8U    //一个服务进程最大支持的功能数
#define __MSG_TASK_ALIAS  24U   //task进程的别名,包含'\0'
#define __MSG_MAX_BUFF    1536U //最多可以保存3个扇区



/*
 * 消息发送应该区分是否由中断发起的中断和由普通进程发送的消息
 */
/*
typedef enum e_sender
{
  FROM_PROC ,   //由进程发送的消息
  FROM_INTR     //由中断发送的消息
} sender_t ;
*/


/*
 * 所有服务进程或者驱动程序在操作系统注册时
 * 必须提供一个别名,并且提公共所有功能号
 * 正常的功能号不能为0,功能不能超过8个
 * 未满8个时,以0表示结束
 */
// 用于服务进程注册消息类型,发送的消息必须在这些消息类型中
typedef struct s_msg_type_node
{
  pid_t reciever ;         //服务进程的pid
  char task_alias[__MSG_TASK_ALIAS] ; // 服务进程别名
  dlist_node_t hash_list ;           // 此散列表将以task_alias作为关键字
} msg_type_node_t ;


// 至于每个服务进程的功能及消息格式
// 在各个服务进程自己的头文件中
typedef struct s_msg
{
  pid_t sender ;
  uint32_t msg_func ;
  dlist_node_t msg_node ;   //消息队列节点
  uint8_t msg_buff[__MSG_MAX_BUFF] ;  
} msg_t ;


// 初始化消息系统,主要是初始化服务进程注册hash表
void init_msg_sys( void ) ;
// 初始化 msg_t 类型的数据
bool_t init_msg( msg_t *msg, pid_t sender, uint32_t msg_func, void *msg_buff, uint32_t msg_buff_size ) ;
// 发送消息,成功返回TRUE,失败返回FALSE
// 内核不可重入.所有发送消息的时候不需要同步机制
// 这个msg的内存怎么管理需要斟酌一下
//  1. 首先是用户空间的地址
//  2. 通过int 0x80传过来还是使用用户空间的地址
//  3. 在内核空间分配地址,将原msg的内容拷贝过来
//  4. 用户空间的内存由用户空间自行管理
bool_t k_msg_send( pid_t pid_accept, msg_t *msg, msg_type_t msg_type ) ;
// 系统调用使用的函数
bool_t sys_msg_send( pid_t pid, msg_t *msg ) ;
// 接收消息,成功返回消息结构,失败返回NULL
// 内存路径
//  以当前 pcb 分配内存
//  把内核空间的消息拷贝到用户空间
msg_t *k_msg_recv( pid_t pid_recv_from, uint32_t msg_func, msg_type_t msg_type ) ;
// 消息注册,成功返回TRUE,失败饭后FALSE
bool_t k_msg_regist( pid_t pid, char *task_alias) ;
// 消息类型查询,成功返回进程pid,失败返回0xffffffff
pid_t k_msg_search( char *tasl_alias ) ;


#endif