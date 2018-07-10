; MBR需要完成的任务有,开启保护模式,开启分页,加载内核

%include "protect_mode.inc.asm"
%include "paging.inc.asm"
%include "const.inc.asm"


SECTION MBR vstart=0x7c00
;--------------------------
[bits 16]
mbr_start: 
  mov    ax, cs
  mov    ds, ax
  mov    es, ax
  mov    ss, ax
  mov    sp, MBR_STACK_BASE

; 清屏
  mov    ax, BIOS_SCREEN_FUN_CLEAR << 8
  mov    bx, SCREEN_BLANK_LINE_ATTR << 8
  mov    cx, SCREEN_SCROLL_LINES
  mov    dx, SCREEN_POSITION_END
  int    BIOS_SCREEN_SERVICE
; 设置光标到0x0000处
  mov    ax, BIOS_SCREEN_FUN_SET_CURSOR << 8
  xor    bx, bx
  mov    dx, SCREEN_POSITION_BEGIN
  int    BIOS_SCREEN_SERVICE
; 打印CHAR_NUL到光标起始位置,为内核库打印程序做准备
  mov    bp, msg_init_screen
  mov    cx, msg_init_screen_len
  mov    ax, BIOS_SCREEN_FUN_WR_STR_WITH_CUR  ; al=0x01:显示字符串,光标跟随移动
  mov    bx, BIOS_SCREEN_FUN_ATTR_BLACK_WHITE ; bl=0x07:黑底白字
  int    BIOS_SCREEN_SERVICE

; 获取内存总数
  xor    esi, esi         ; 保存内存总数
  xor    ebx, ebx
  mov    di,  ARDS_BUF
  mov    edx, BIOS_SMAP
.get_mem_info_e820:
  mov    eax, BIOS_GET_MEM_INFO_E820
  mov    ecx, ARDS_SIZE
  int    BIOS_MEM_SERVICE
  jc     .get_mem_info_e801    ;使用0xe820功能失败,尝试0xe801功能
  mov    eax, [di]      ; 内存基地址
  add    eax, [di+8]    ; 此段内存长度
  cmp    esi, eax
  cmovb  esi, eax
  cmp    ebx, 0
  jnz    .get_mem_info_e820
  jmp    .get_mem_info_ok
.get_mem_info_e801:
  mov    ax, BIOS_GET_MEM_INFO_E801
  int    BIOS_MEM_SERVICE
;  jc     .get_mem_info_88      ;使用0xe801功能失败,尝试0x88功能
  jc     .mbr_init_err
  and    eax, 0x0000ffff
  shl    eax, 10               ;eax * 1024
  and    edx, 0x0000ffff
  shl    edx, 10+6             ;edx * 1024 * 64
  add    eax, edx
  mov    esi, eax
  add    eax, 0x100000
  or     edx, edx
  cmovnz esi, eax              ;如果edx不为0的话,总内存大小要加1M
  jmp    .get_mem_info_ok
;.get_mem_info_88:
;  mov    ah,  BIOS_GET_MEM_INFO_88
;  int    BIOS_MEM_SERVICE
;  jc     .mbr_init_err
;  and    eax, 0x0000ffff
;  shl    eax, 10               ;eax * 1024
;  add    eax, 0x100000
;  mov    esi, eax
.get_mem_info_ok:
  mov    [MEM_SIZE_ADDR], esi

; 用BIOS读取bootload file
  mov    si, FILE_MAX_READ_TIMES ;官方BIOS说最少尝试3次

  mov    cx, (BOOTLOADER_FILE_CYLINDER << 8) | BOOTLOADER_FILE_SECTOR
  mov    dx, (BOOTLOADER_FILE_HEADER << 8) | BOOTLOADER_FILE_DRIVE  
  mov    bx, BOOTLOADER_FILE_LOAD_ADDR
.try_read_bootload_file :
  mov    ax, (BIOS_DISK_FUN_CHS_READ << 8) | BOOTLOADER_FILE_MAX_SECTORS
  int    BIOS_DISK_SERVICE
  jnc    .bootload_file_read_ok
  dec    si
  jnz    .try_read_bootload_file
.mbr_init_err :      
  hlt
.bootload_file_read_ok :
; 以下代码进入保护进入保护模式
  ; 打开A20地址线
  OPEN_A20_ADDRLINE
  ; 加载gdt
  lgdt    [GDT_PTR]
  ; 进入保护模式
  ENTER_PROTECT_MODE
  ; 刷新流水线,避免分支预测的副作用
  jmp    SELECTOR_FLAT_CODE:protect_mode_start

[bits 32]
protect_mode_start :
  mov    eax, SELECTOR_FLAT_DATA  ;数据段
  mov    ds, eax
  mov    es, eax
  mov    ss, eax
  mov    fs, eax
  mov    eax, SELECTOR_VIDEO      ;显存段
  mov    gs, eax
; 初始化页目录表
  mov    ebx, PAGE_BASE_ADDR
  mov    dword [ebx], PTE_TABLE_0_ADDR | PG_P | PG_RWW | PG_USS
; 低1M内存对等映射页表项表地址
  mov    ebx, PTE_TABLE_0_ADDR
  mov    eax, [MEM_SIZE_ADDR]
  mov    ecx, ADDR_INIT_MAX
  cmp    ecx, eax
  cmova  ecx, eax
  shr    ecx, 12
  mov    eax, PG_P | PG_RWW | PG_USS
.setup_page:
  mov    [ebx], eax
  add    eax, PG_SIZE
  add    ebx, PTE_SIZE
  loop .setup_page
; 开启分页
  mov    eax, PAGE_BASE_ADDR
  mov    cr3, eax
  mov    eax, cr0
  or     eax, 0x80000000
  mov    cr0, eax
; 保存GDT_PTR
  mov   dword [GDT_PTR_ADDR], GDT_PTR
; 加载bootloader到正确的位置
  push  dword BOOTLOADER_FILE_LOAD_ADDR
  call  elf_load
  add   esp, 4
  
; 让内核使用的栈更大一些
  mov     esp, KERNEL_STACK_BASE
; 清空 TSS任务状态段
  mov     ecx, TSS_SIZE / 4
  xor     eax, eax
  mov     edi, TSS_BASE
  rep     stosd
; 不需要io位图,将IO位图的偏移放到 TSS_LIMIT 的外面
  mov     ebx, TSS_BASE + 102
  mov     word [ebx], TSS_LIMIT+1
; 加载 TSS任务状态段
  mov     eax, SELECTOR_TSS
  ltr     ax
  push    dword DEFAUL_EFLAGS   ;IOPL = 1
  popfd

; 跳转到内核代码
  jmp     SELECTOR_FLAT_CODE:BOOTLOADER_ENTRY_ADDR

%include  "elf_load.inc.asm"


;Data for protect mode
; -------------------------------------------------------------------
; GDT ------------------------------------------------
;                 macro           base               limit               attr
GDT_BEGIN_ADDR :  SEG_DESC        0,                 0,                  0                                        ;nil Descriptor
DESC_FLAT_CODE :  SEG_DESC        FLAT_BASE,         FLAT_LIMIT,         DA_C   | DA_32 | DA_LIMIT_4K             ;read/excutable segment 4G (kernel)
DESC_FLAT_DATA :  SEG_DESC        FLAT_BASE,         FLAT_LIMIT,         DA_DRW | DA_32 | DA_LIMIT_4K             ;read/write segment 4G
DESC_VIDEO     :  SEG_DESC        VIDEO_BASE,        VIDEO_LIMIT,        DA_DRW | DA_32 | DA_DPL1                 ;video card segment. prepare for kernel and task
DESC_TSS       :  SEG_DESC        TSS_BASE,          TSS_LIMIT,          DA_386_TSS                               ;任务状态段
; 内核服务所使用的描述符
DESC_TASK_CODE :  SEG_DESC        FLAT_BASE,         FLAT_LIMIT,         DA_C   | DA_32 | DA_LIMIT_4K | DA_DPL1
DESC_TASK_DATA :  SEG_DESC        FLAT_BASE,         FLAT_LIMIT,         DA_DRW | DA_32 | DA_LIMIT_4K | DA_DPL1
; 用户进程所用描述符
DESC_USER_CODE :  SEG_DESC        FLAT_BASE,         FLAT_LIMIT,         DA_C   | DA_32 | DA_LIMIT_4K | DA_DPL3
DESC_USER_DATA :  SEG_DESC        FLAT_BASE,         FLAT_LIMIT,         DA_DRW | DA_32 | DA_LIMIT_4K | DA_DPL3
;end of GDT

GDT_LEN               equ       $ - GDT_BEGIN_ADDR
GDT_PTR               dw        GDT_LEN - 1               ; GDT limit
                      dd        GDT_BEGIN_ADDR            ; GDT offset address
; The GDT is not a segment itself; instead, it is a data structure in linear address space.
; The base linear address and limit of the GDT must be loaded into the GDTR register. 
; -- IA-32 Software Developer’s Manual, Vol.3A

; GDT selector
SELECTOR_FLAT_CODE   equ        DESC_FLAT_CODE - GDT_BEGIN_ADDR
SELECTOR_FLAT_DATA   equ        DESC_FLAT_DATA - GDT_BEGIN_ADDR  
SELECTOR_VIDEO       equ        DESC_VIDEO     - GDT_BEGIN_ADDR
SELECTOR_TSS         equ        DESC_TSS       - GDT_BEGIN_ADDR

; -----------
msg_init_screen      db         0x00
msg_init_screen_len  equ        $ - msg_init_screen

times     510-($-$$) db 0     ;fill with 0
dw        0xaa55              ;MBR end flag
