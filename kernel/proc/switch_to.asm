TSS_ADDR    equ    0x00007100
PAGE_SIZE   equ    0x00001000
SLT_K_CODE  equ    0x08         ; 内核代码段选择子

[bits 32]
EXTERN intr_exit

GLOBAL switch_to

;void swich_to( pcb_t *from, pcb_t *to )
switch_to :
  push   esi
  push   edi
  push   ebx
  push   ebp

  mov    eax, [esp+20]        ; from -> eax
  ;cmp    eax, 0               ; 判断from是不是NULL
  ;jz     .get_to_ptr          ; from 是从schedule传过来的,所以不会为NULL
  mov    [eax], esp           ; 保存的内核栈栈顶到 from
.get_to_ptr :
  mov    eax, [esp+24]        ; to -> eax
  mov    esp, [eax]           ; 现在栈顶已经指向 to 的内核栈
  mov    ebx, TSS_ADDR        ; 进程切换是需要更换TSS中的内核栈
  mov    ecx, eax             ; eax 是 to, to的地址加PAGE_SIZE=内核栈栈顶
  add    ecx, PAGE_SIZE       ; 进程内核栈的栈顶
  mov    [ebx+4], ecx         ; 更换TSS中的esp0
  mov    ecx, [eax+8]         ; pdt的地址
  mov    cr3, ecx             ; 更换pdt
  jmp    SLT_K_CODE:is_first  ; 刷新tlb

is_first :
  mov    ecx, [eax+4]         ; 将 to->all_used_ticks 拿出来
                              ; 判断是否是第一次调度上线
  cmp    ecx, 0               ; all_used_ticks == 0,表示第一次上cpu
  ja     .normal_exit         ; 不是第一次运行,正常结束
  inc    dword [eax+4]        ; 增加all_used_ticks
  jmp    intr_exit            ; 直接跳转退出

.normal_exit :
  pop    ebp
  pop    ebx
  pop    edi
  pop    esi
  ret  