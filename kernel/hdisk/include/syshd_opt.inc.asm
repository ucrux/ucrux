; 硬盘相关常量
DISK_CMD_READ                 equ         0x20
DISK_LBA_MODE                 equ         0xe0
DISK_MASTER                   equ         0x00

PRI_DATA_PORT                 equ         0x01f0
PRI_SEC_COUNT_PORT            equ         0x01f2
PRI_LBA_LOW_PORT              equ         0x01f3
PRI_LBA_MID_PORT              equ         0x01f4
PRI_LBA_HIGH_PORT             equ         0x01f5
PRI_DEVICE_PORT               equ         0x01f6
PRI_STAT_CMD_PORT             equ         0x01f7


; 等待硬盘不忙的宏

%macro wait_hd 0
.wait_disk :
    mov    edx, PRI_STAT_CMD_PORT
    in     al, dx
    nop
    nop
    and    al, 0x88
    cmp    al, 0x08
    jnz    .wait_disk               ;不忙，且硬盘已准备好数据传输 
%endmacro