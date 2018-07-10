%include "int_handle.inc.asm"

SELECTOR_KERNEL_CODE  equ  0x08

[bits 32]

;声明外部变量
       ; 中断处理函数表
extern intr_handle_func_table
       ; 系统调用函数表
extern syscall_func_table
;导出数据
global intr_handle_entry_table, intr_exit, change_pdt 



section .data

intr_handle_entry_table :
    entry_addr 0x00
    entry_addr 0x01
    entry_addr 0x02
    entry_addr 0x03
    entry_addr 0x04
    entry_addr 0x05
    entry_addr 0x06
    entry_addr 0x07
    entry_addr 0x08
    entry_addr 0x09
    entry_addr 0x0a
    entry_addr 0x0b
    entry_addr 0x0c
    entry_addr 0x0d
    entry_addr 0x0e
    entry_addr 0x0f
    entry_addr 0x10
    entry_addr 0x11
    entry_addr 0x12
    entry_addr 0x13
    entry_addr 0x14
    entry_addr 0x15
    entry_addr 0x16
    entry_addr 0x17
    entry_addr 0x18
    entry_addr 0x19
    entry_addr 0x1a
    entry_addr 0x1b
    entry_addr 0x1c
    entry_addr 0x1d
    entry_addr 0x1e
    entry_addr 0x1f
    entry_addr 0x20
    entry_addr 0x21
    entry_addr 0x22
    entry_addr 0x23
    entry_addr 0x24
    entry_addr 0x25
    entry_addr 0x26
    entry_addr 0x27
    entry_addr 0x28
    entry_addr 0x29
    entry_addr 0x2a
    entry_addr 0x2b
    entry_addr 0x2c
    entry_addr 0x2d
    entry_addr 0x2e
    entry_addr 0x2f
    dd syscall_handle

section .text
intr_vec 0x00, ZERO
intr_vec 0x01, ZERO
intr_vec 0x02, ZERO
intr_vec 0x03, ZERO
intr_vec 0x04, ZERO
intr_vec 0x05, ZERO
intr_vec 0x06, ZERO
intr_vec 0x07, ZERO
intr_vec 0x08, ERROR_CODE
intr_vec 0x09, ZERO
intr_vec 0x0a, ERROR_CODE
intr_vec 0x0b, ERROR_CODE
intr_vec 0x0c, ERROR_CODE
intr_vec 0x0d, ERROR_CODE
intr_vec 0x0e, ERROR_CODE
intr_vec 0x0f, ZERO
intr_vec 0x10, ZERO
intr_vec 0x11, ERROR_CODE
intr_vec 0x12, ZERO
intr_vec 0x13, ZERO
intr_vec 0x14, ZERO
intr_vec 0x15, ZERO
intr_vec 0x16, ZERO
intr_vec 0x17, ZERO
intr_vec 0x18, ERROR_CODE
intr_vec 0x19, ZERO
intr_vec 0x1a, ERROR_CODE
intr_vec 0x1b, ERROR_CODE
intr_vec 0x1c, ZERO
intr_vec 0x1d, ERROR_CODE
intr_vec 0x1e, ERROR_CODE
intr_vec 0x1f, ZERO
intr_vec 0x20, ZERO ;时钟中断对应的入口
intr_vec 0x21, ZERO ;键盘中断对应的入口
intr_vec 0x22, ZERO ;级联用的
intr_vec 0x23, ZERO ;串口2对应的入口
intr_vec 0x24, ZERO ;串口1对应的入口
intr_vec 0x25, ZERO ;并口2对应的入口
intr_vec 0x26, ZERO ;软盘对应的入口
intr_vec 0x27, ZERO ;并口1对应的入口
intr_vec 0x28, ZERO ;实时时钟对应的入口
intr_vec 0x29, ZERO ;重定向
intr_vec 0x2a, ZERO ;保留
intr_vec 0x2b, ZERO ;保留
intr_vec 0x2c, ZERO ;ps/2鼠标
intr_vec 0x2d, ZERO ;fpu浮点单元异常
intr_vec 0x2e, ZERO ;硬盘
intr_vec 0x2f, ZERO ;保留



; 一共四个系统调用 
;    void *sys_malloc( size_t size ) ;
;    void sys_free( void *void )
;    void sys_msgsend( pid_t *send_to, msg_t *msg ) ;
;    msg_t* sys_msgrecv(  ) ;
;               
; 系统调用 eax 是系统调用功能号
;    edx 系统调用第三个参数
;    ecx 系统调用第二个参数
;    ebx 系统调用第一个参数
; 进入中断的时候会自动关闭if标志
; 所以
;   1.其他中断无法产生
;   2.进入系统调用时,进程是不会调出的
;   3.所以此中断也不会被重入
;   4.所以可以没有同步机制
syscall_handle :
  push   0              ; 和其他中断格式保存一致
  ; 保存运行环境
  push   ds
  push   es
  push   fs
  push   gs
  pushad

  push   0x30           ; 和其他中断保持一致
; 系统调用参数
  push   edx            ; 系统调用第三个参数
  push   ecx            ; 系统调用第二个参数
  push   ebx            ; 系统调用第一个参数

  push   eax
  ; 切换内核段
  mov    eax, SELECTOR_DATA
  mov    ds, eax
  mov    es, eax
  mov    fs, eax
  mov    eax, SELECTOR_VIDEO
  mov    gs, ax
  pop    eax

  call   [syscall_func_table + eax * 4]
  add    esp, 12

  mov    [esp + 8 * 4 ], eax ;保存系统调用的返回值 esp+8*4 保存着 eax
  jmp    intr_exit

intr_exit :
; void intr_exit( void ) ;
  add    esp, 4 ;清除中断向量
  popad
  pop    gs
  pop    fs
  pop    es
  pop    ds
  add    esp, 4 ;清除错误码
  iretd

change_pdt :
; void change_pdt( uint32_t *pdt )
  push   ebp
  mov    ebp, esp
  mov    eax, [ebp+8]
  mov    cr3, eax
  jmp    SELECTOR_KERNEL_CODE:flush_tlb
flush_tlb :
  pop   ebp
  ret 
