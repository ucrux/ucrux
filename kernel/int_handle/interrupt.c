#include "interrupt.h"
#include "idt_init.h"
#include "pic.h"
#include "timer_8253.h"
#include "kputx.h"
#include "timer_intr_handle.h"
#include "kernel_mm.h"
#include "proc.h"
#include "msg.h"
#include "keyboard_intr.h"


#define __EFLAGS_IF 0x00000200U

#define get_eflags( eflags ) asm volatile ( "pushfl; popl %k0" : "=g"(eflags) )

p_intr_handle_func_t intr_handle_func_table[__IDT_DESC_COUNT] ;
static char* intr_info_table[__IDT_DESC_COUNT] ;

static void intr_common_handle( uint32_t intr_vec ) ;
static void idle_run( void ) ;

// 系统调用函数表
uint32_t syscall_func_table[SYSCALL_CNT] ;

// 初始换系统调用
void 
init_syscall( void )
{
  syscall_func_table[SYS_MALLOC] = (uint32_t )sys_malloc ;
  syscall_func_table[SYS_FREE] = (uint32_t )sys_free ;
  syscall_func_table[SYS_SEND_MSG] = (uint32_t )sys_msg_send ;
  syscall_func_table[SYS_RECV_MSG] = (uint32_t )k_msg_recv ;
  //syscall_func_table[SYS_GETPID] = (uint32_t )sys_getpid ;
  syscall_func_table[SYS_IDLE] = (uint32_t )idle_run ;
}

// 获取当前中断状态
intr_status_t
intr_get_status( void )
{
  uint32_t eflags ;
  get_eflags(eflags) ;
  return (eflags & __EFLAGS_IF) ? INTR_ON : INTR_OFF ;
}

// 关闭中断,并返回之前的中断状态
intr_status_t
intr_turn_off( void )
{
  uint32_t eflags ;
  get_eflags( eflags ) ;
  if( eflags & __EFLAGS_IF ) //原先是是开中断的
  {
    asm volatile( "cli" ) ;
    return INTR_ON ;
  }
  //原先是关中断的
  return INTR_OFF ;
}
// 开启中断,并返回之前的中断状态
intr_status_t 
intr_turn_on( void )
{
  uint32_t eflags ;
  get_eflags( eflags ) ;
  if( eflags & __EFLAGS_IF ) //原先是开中断的
    return INTR_ON ;
  //原先是关中断的
  asm volatile( "sti" ) ;
  return INTR_OFF ;
}

// 设置中断位指定值,并返回之前的值
intr_status_t
intr_set_status( intr_status_t status )
{
  return status == INTR_ON ? intr_turn_on( ) : intr_turn_off( ) ;
}

void 
inst_intr_handle( p_intr_handle_func_t func, uint32_t intr_vec )
{
  intr_handle_func_table[intr_vec] = func ;
}

void
uninst_intr_handle( uint32_t intr_vec )
{
  intr_handle_func_table[intr_vec] = intr_common_handle ;
}

void
exception_init( void )
{
  uint32_t loop ;
  for( loop = 0 ; loop < __IDT_DESC_COUNT ; loop++ )
  {
    //为所有异常初始化一个处理函数
    intr_handle_func_table[loop] = intr_common_handle ;
    //初始化异常信息统一为 unknown
    intr_info_table[loop] = "unknown" ;
  }

  intr_info_table[0] = "#DE Divide Error" ;
  intr_info_table[1] = "#DB Debug Exception" ;
  intr_info_table[2] = "NMI interrupt" ;
  intr_info_table[3] = "#BP Breakpoint Exception" ;
  intr_info_table[4] = "#OF Overflow Exception" ;
  intr_info_table[5] = "#BR BOUND Range Exceeded Exception" ;
  intr_info_table[6] = "#UD Invalid Opcode Exception" ;
  intr_info_table[7] = "#NM Device Not Available Exception" ;
  intr_info_table[8] = "#DF Double Fault Exception" ;
  intr_info_table[9] = "Coprocessor Segment Overrun" ;
  intr_info_table[10] = "#TS Invalid TSS Exception" ;
  intr_info_table[11] = "#NP Segment Not Present" ;
  intr_info_table[12] = "#SS Stack Fault Exception" ;
  intr_info_table[13] = "#GP General Protection Exception" ;
  intr_info_table[14] = "#PF Page-Fault Exception" ;
  // intr_info_table[15] 保留,现在未使用
  intr_info_table[16] = "#MF x87 FPU Floating-Point Error" ;
  intr_info_table[17] = "#AC Aligment Check Exception" ;
  intr_info_table[18] = "#MC Machine-Check Exception" ;
  intr_info_table[19] = "#XF SIMD Floating-Point Exception" ;
}

void
interrupt_init( void )
{
  idt_init( ) ;
  exception_init( ) ;
  pic_init( ) ;
  timer_init( ) ;
  //安装时间中断处理函数
  inst_intr_handle( intr_timer_handle, __TIMER_INTR_VEC ) ;
  //安装键盘中断处理函数
  inst_intr_handle( intr_keyboard_handler, __KEYBOARD_INTR_VEC ) ;
}



static void 
intr_common_handle( uint32_t intr_vec )
{
  // 0x2f是从片8259A上的最后一个irq引脚,保留
  // IRQ7和IRQ15会产生伪中断(spurious interrupt),无须处理。
  if (intr_vec == 0x27 || intr_vec == 0x2f) 
      return;   
  
  //忽略硬盘中断
  if( intr_vec == 0x2E )
    return;

  kputstr("\n!!!!!!!      excetion message begin  !!!!!!!!\n");
  kputstr( "interrupt vector : " ) ;
  kput_uint32_hex( intr_vec ) ;
  kputchar( '\n' ) ;
  kputstr( intr_info_table[intr_vec] ) ;
  // 若为Pagefault,将缺失的地址打印出来并悬停
  if (intr_vec == 14) 
  {   
    int page_fault_vaddr = 0 ;
    // cr2是存放造成page_fault的地址
    asm ("movl %%cr2, %0" : "=r" (page_fault_vaddr)) ;
    kputstr("\npage fault addr is: ") ;
    kput_uint32_hex(page_fault_vaddr) ; 
   }
  kputstr("\n!!!!!!!      excetion message end    !!!!!!!!\n");
  //if( intr_vec == 0x0d )
  asm volatile ( "cli; hlt" ) ;
}

// 这个函数的调用流程
// int 0x30     ;进入 0x30 中断时是关中断的
//   call idle_run
//     开中断
//     hlt
//     中断来了
//       执行中断处理
//          被调度出CPU - 其他进程 - 调度
//       中断返回  ;这个时候中断时开着的
//     关中断
//  int 0x30 返回 ; 中断再次开启
static void 
idle_run( void )
{
  asm volatile ( "sti; hlt; cli " ) ;
}



