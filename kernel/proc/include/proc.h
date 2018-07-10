#ifndef __PROC_H
#define __PROC_H
#include "global_type.h"
#include "dlist.h"
#include "mm.h"
#include "page.h"
#include "ksync.h"


/*
 * 任何进程的栈大小最多为4K
 * 用户进程初始化使用 idle 进程的 pdt
 * 其他服务进程使用 内核主进程的 pdt
 */


#define __MAX_PROC_NAME       14U // 包含terminal符号,也就是 '\0'
// 进程可以打开的最大文件数量
#define __MAX_OPEN_FILE       8U   // 最大打开的文件数量

#define get_current_pcb( pcb_addr ) \
          asm volatile ( "movl %%esp, %k0; \
                          andl $0xfffff000, %k0" : \
                          "=g"((pcb_addr)) : : \
                          "memory" )

#define __PID_UNDEF    -1

typedef int32_t pid_t ;


// 进程状态
typedef enum e_task_status
{
  RUNNING ,
  WAITING ,
  SLEEPING ,
  EXIT,
  STATUS_CNT
} task_status_t ;

// 进程头队列数类型
typedef enum  e_proc_que_type
{
  QUE_READ1 ,
  QUE_READ2 ,
  QUE_TYPE_CNT
} proc_que_type_t ;

// 进程类型
// 每个优先级有一个优先级队列
// 最大优先级数量, 0~3
typedef enum e_proc_type 
{
  PROC_IDLE ,
  PROC_USER ,
  PROC_TASK ,
  PROC_KERNEL ,
  PROC_TYPE_CNT
} proc_type_t ;

// 消息队列数据结构
typedef struct s_msg_que
{
  dlist_head_t msg_list[MSG_TYPE_CNT] ;
  sem_t sem[MSG_TYPE_CNT] ;
} msg_que_t ;

typedef struct s_sleep_list_node
{
  dlist_node_t sleep_node ;  //用于加入睡眠链表
  uint32_t sleep_time ;      //睡眠时间,单位秒
} sleep_list_node_t ;

// tss结构
typedef struct s_tss
{
  uint32_t prev_tss ;
  uint32_t esp0 ;
  uint32_t ss0 ;
  uint32_t esp1 ;
  uint32_t ss1 ;
  uint32_t esp2 ;
  uint32_t ss2 ;
  uint32_t cr3 ;
  uint32_t eip ;
  uint32_t eflages ;
  uint32_t eax ;
  uint32_t ecx ;
  uint32_t edx ;
  uint32_t ebx ;
  uint32_t esp ;
  uint32_t ebp ;
  uint32_t esi ;
  uint32_t edi ;
  uint32_t es ;
  uint32_t cs ;
  uint32_t ss ;
  uint32_t ds ;
  uint32_t fs ;
  uint32_t gs ;
  uint32_t ldt ;
  uint32_t iomap ;
} tss_t ;


// 与发生中断时保存在栈中的数据一致
typedef struct s_intr_regs
{
  uint32_t vec_no ;    //中断向量号
  uint32_t edi ;
  uint32_t esi ;
  uint32_t ebp ;
  uint32_t esp_dump ;  //这个esp指针没有用
  uint32_t ebx ;
  uint32_t edx ;
  uint32_t ecx ;
  uint32_t eax ;
  uint32_t gs ;
  uint32_t fs ;
  uint32_t es ;
  uint32_t ds ;
  uint32_t err_no ;
  uint32_t eip ;
  uint32_t cs ;
  uint32_t eflages ;
  uint32_t esp ;       //出现特权级转换时,原特权等级的esp
  uint32_t ss ;        //出现特权级转换时,原特权级别的ss
} intr_regs_t ;

// 进程控制块
// 为了简化,进程不具有父子关系
typedef struct s_pcb pcb_t ;
struct s_pcb
{
  uint32_t *kernel_stack_top ;         // 内核空间时使用的栈,这个应该保存内核栈的栈顶
  uint32_t all_used_ticks ;            // 进程在cpu上运行的总ticks
  uint32_t *pdt ;                      // 页目录表地址
  uint32_t *self_stack ;               // 自己使用的栈
  pid_t pid ;                          // hash的关键字
  task_status_t status ;
  uint8_t priority ;                   // 进程优先级,一共8个优先级
  uint8_t rest_ticks ;                 // 初始值是 priority << 4
  char  proc_name[__MAX_PROC_NAME] ;
  //sem_t wait_kernel_msg ;              // 等待内核或者是中断来的消息
  msg_que_t msg_que ;                  // 消息队列
  dlist_node_t pcb_list ;              // pcb管理链表
  dlist_node_t hash_list ;             // 用于进程查找的hash链表
  sleep_list_node_t sleep_node ;       // 用于进程睡眠
  vaddr_manage_t vaddr_manage ;        // 虚拟地址管理
  // 文件描述符相关移动至文件系统服务进程
  //int32_t fd_table[__MAX_OPEN_FILE] ;  // 文件描述符表
  mm_blob_t *mmblob ;                  // blob分配的管理结构,这个结构有2页大
  dlist_head_t blobxx_alloc ;          // 用于分配此mm_blobxx_t结构的内存管理链表
  dlist_head_t vaddrnode_alloc ;       // 用于分配 vaddr_node_t 结构的内存管理链表
  int32_t exit_code ;                  // 进程退出码
}  ;


// 进程切换
// to 已经从 proc_que 上删除了
// 如果 from 为NULL, 则不会保存原来的栈指针
// 如果to->all_used_ticks == 0,即第一次调度上CPU
extern void switch_to( pcb_t *from, pcb_t *to ) ;

// 分配进程id
// 这里需要一个数据结构
// 为了简便就不使用位图了
// 直接使用数组记录进程ip是否被占用
// 最大进程号为4095
// 分配不成功返回0
uint32_t new_pid( void ) ;

// 初始化的 status 为 READY
bool_t init_pcb( pcb_t *pcb, pid_t pid, uint8_t priority, \
               char* proc_name, uint32_t *self_stack, \
               uint32_t *pdt, mm_blob_t *mmblob ) ;

// 初始化进程队列和进程hash表,成功返回TRUE, 失败返回FALSE
bool_t init_proc_manage( void ) ;

// 将 pcb 插入到进程队列的就绪队列中,成功返回TRUE,失败返回FALSE
// 顺带还要插入到 hash 表
bool_t insert_proc( pcb_t *pcb, proc_que_type_t pqt ) ;

// 通过 pid 查找 pcb
pcb_t *search_pcb( pid_t pid ) ;

// 从proc_que中删除pcb,通过pid
pcb_t *rm_proc_que( pid_t pid ) ;

// 将 pcb 插入到 proc_que,成功返回TRUE, 失败返回FALSE
// 总是插入到非运行队列
bool_t inst_proc_que( pcb_t *pcb ) ;

// 插入到正在运行的队列中
bool_t inst_proc_que_running( pcb_t *pcb ) ;

// 从proc_hash中删除pcb,通过pid
pcb_t *rm_proc_hash( pid_t pid ) ;

// 将自定pid的进程从进程队列中删除, 成功返回 pcb ,失败返回NULL
pcb_t *remove_proc( pid_t pid ) ;


// 创建新的pcb,成功返回pcb物理地址指针,失败返回NULL
// 新创建进程使用这个函数创建pcb
// 由于创建进程栈的时候会占用1个页面的地址
// 所以可用虚拟地址的起始地址为 0x40001000
// 所以规定所有用户或者服务进程的入口地址都为0x40002000
// 对于所有进程,我们默认分配从0x40001000(虚拟地址)起始的内存,
pcb_t *mkpcb( char *proc_name, uint32_t func_entry, proc_type_t ptype ) ;

// 按照优先级挑选要执行的进程
// 按优先级在正在运行的对列中挑选
// 如果正在运行的队列中都没有可用进程
// 将切换正在运行的队列
// 这个时候已经是第二遍历遍所有进程队列了
// 如果第二遍历遍进程队列还没有找到可用进程,
// 就返回 idle 进程
pcb_t *pickup_proc( void ) ;

// 进程的创建可以分为两种情况
//  1.创建新的进程
//  2.在原有进程的基础上创建进程

// 进程调度函数
void schedule( void ) ;

// 获取进程自己的pid
//pid_t sys_getpid( void ) ;

#endif