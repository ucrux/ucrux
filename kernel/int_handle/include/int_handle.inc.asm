; 显卡段选择子
SELECTOR_VIDEO          equ  0x18
; 内核数据段选择子
SELECTOR_DATA           equ  0x10

; 8259a 端口常数
ICW1_8259A_MASTER_PORT  equ  0x20
ICW1_8259A_SLAVE_PORT   equ  0xa0
ICW2_8259A_MASTER_PORT  equ  0x21
ICW2_8259A_SLAVE_PORT   equ  0xa1
ICW3_8259A_MASTER_PORT  equ  0x21
ICW3_8259A_SLAVE_PORT   equ  0xa1
ICW4_8259A_MASTER_PORT  equ  0x21
ICW4_8259A_SLAVE_PORT   equ  0xa1
OCW1_8259A_MASTER_PORT  equ  0x21
OCW1_8259A_SLAVE_PORT   equ  0xa1
OCW2_8259A_MASTER_PORT  equ  0x20
OCW2_8259A_SLAVE_PORT   equ  0xa0
OCW3_8259A_MASTER_PORT  equ  0x20
OCW3_8259A_SLAVE_PORT   equ  0xa0

; 8259a 命令常数
CMD_8259A_EOI           equ  0x20

%define ERROR_CODE  nop
%define ZERO        push dword 0

; 通用中断处理函数
; %1 中断号, %2 有没有错误码
%macro intr_vec 2
  intr%1entry :
    %2
    ;保存中断现场,CS EIP (SS ESP有特权等级转移的话) 被自动保存了
    push   ds
    push   es
    push   fs
    push   gs
    pushad      ;入栈顺序: eax,ecx,edx,ebx,esp,ebp,esi,edi
    ; 中断需不需要切换GDT这个我还没有考虑好
    ; 但是系统调用肯定是需要的,比如说内存分配
    ; 这里要切换内核GDT,很多函数要用到内核的心分配的内存

    ; 切换内核段
    mov    eax, SELECTOR_DATA
    mov    ds, eax
    mov    es, eax
    mov    fs, eax
    mov    eax, SELECTOR_VIDEO
    mov    gs, eax 
    
    ; 发送中断结束指令
    mov    al, CMD_8259A_EOI
    out    OCW2_8259A_SLAVE_PORT, al
    out    OCW2_8259A_MASTER_PORT, al
    ; 调用中断处理函数
    push   dword %1   ;中断向量入栈
    call   [intr_handle_func_table + %1 * 4]   ;调用中断处理函数中的处理函数
    ; 跳转到中断出口
    jmp    intr_exit
%endmacro

; 中断处理函数入口地址列表
; %1 中断号
%macro entry_addr 1
  dd intr%1entry
%endmacro