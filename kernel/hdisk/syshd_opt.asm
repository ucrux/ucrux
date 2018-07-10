%include "syshd_opt.inc.asm"

[bits 32]
section .text
;导出函数列表
GLOBAL sys_read_pri_disk

; 从硬盘中将内核elf文件读入到指定内存位置
sys_read_pri_disk :
; void sys_read_pri_disk( void *buff, uint32_t sectors, uint_32_t begin_sec )
    push   ebp      ;
    mov    ebp, esp ;  begin_sec
    push   eax      ;  sectors
    push   ecx      ;  buff
    push   edx      ;  retaddr
    push   edi      ;  ebp

    mov    edi, [ebp+8]                    ; buff地址
    mov    edx, PRI_SEC_COUNT_PORT
    mov    eax, [ebp+12]
    out    dx, al                          ; 读取的扇区数
    nop
    nop

    mov    eax, [ebp+16]                   ; 起始山区

    mov    edx, PRI_LBA_LOW_PORT           ; 0x1f3
    out    dx,  al                         ; LBA地址7~0
    nop
    nop

    mov    edx, PRI_LBA_MID_PORT           ; 0x1f4        
    shr    eax, 8
    out    dx, al                          ; LBA地址15~8
    nop
    nop

    mov    edx, PRI_LBA_HIGH_PORT          ; 0x1f5
    shr    eax, 8
    out    dx, al                          ; LBA地址23~16
    nop
    nop

    mov    edx, PRI_DEVICE_PORT            ; 0x1f6
    shr    eax, 8
    and    al, 0x0f
    or     al, DISK_LBA_MODE               ; 第一硬盘  LBA地址27~24
    out    dx, al
    nop
    nop

    mov    edx, PRI_STAT_CMD_PORT          ; 0x1f7
    mov    al, DISK_CMD_READ               ; 读命令
    out    dx, al
    nop
    nop

    wait_hd                                ; 等待硬盘不忙

    ; KERNEL_ELF_FILE_SECTORS * 256
    mov    ecx, [ebp+8]                      ; 总共要读取的字数
    shr    ecx, 8
    mov    edx, PRI_DATA_PORT                ; 0x01f0
.readw:
    in     ax, dx
    nop
    nop
    cld
    stosw
    loop   .readw
  
    pop    edi
    pop    edx
    pop    ecx
    pop    eax
    pop    ebp
    ret