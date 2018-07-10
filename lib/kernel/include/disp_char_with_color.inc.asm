; -----------------------------
; 显卡相关常数和操作宏
; -----------------------------
; 特殊字符
CHAR_NUL         equ    0x00
CHAR_LF          equ    0x0a
CHAR_BS          equ    0x08
CHAR_BLANK       equ    0x20
; 屏幕属性
SCREEN_LINE_SIZE equ    0x50   ;80


; 获取屏幕光标当前位置
; 破坏 dx, ax, bx寄存器
; 输出 ax = 当前光标位置
%macro GET_SCREEN_CURSOR 0
  mov    dx, 0x3d4
  mov    al, 0x0e
  out    dx, al
  mov    dx, 0x3d5
  in     al, dx                        ;高8位 
  mov    ah, al

  mov    dx, 0x3d4
  mov    al, 0x0f
  out    dx, al
  mov    dx, 0x3d5
  in     al, dx                        ;低8位 
%endmacro

; 写入光标位置
; bx = 要写入的光标位置
; 破坏 dx,ax寄存器
%macro SET_SCREEN_CURSOR 0
  mov    dx, 0x3d4
  mov    al, 0x0e
  out    dx, al
  mov    dx, 0x3d5
  mov    al, bh
  out    dx, al
  mov    dx, 0x3d4
  mov    al, 0x0f
  out    dx, al
  mov    dx, 0x3d5
  mov    al, bl
  out    dx, al
%endmacro

; 写入字符,切光标位置加1
; ebx = 要写入字符的位置
; cx = 要写入的字符及颜色属性
%macro WRITE_CHAR_TO_SCREEN 0
  shl    bx, 1
  mov    [gs:ebx], cx
  shr    bx, 1
%endmacro

; 向上卷屏一行
; 注意仅能在光标超出屏幕范围时,才能使用
%macro ROLL_SCREEN 0
  push   ds
  push   es
  mov    eax, gs
  mov    ds, eax
  mov    es, eax
  cld
  mov    esi, 0xa0
  mov    edi, 0x00
  mov    ecx, 1920
  rep    movsw
  mov    ebx, 3840                      ;清除屏幕最底一行
  mov    ecx, 80
.cls:
  mov    word[es:ebx], 0x0720
  add    ebx, 2
  loop   .cls
  pop    es
  pop    ds
  mov    ebx, 1921
%endmacro