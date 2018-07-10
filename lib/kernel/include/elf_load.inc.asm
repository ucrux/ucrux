; ELF 相关常数
ELF_EHDR_PHENTSIZE_OFF        equ         0x2a
ELF_EHDR_PHNUM_OFF            equ         0x2c
ELF_EHDR_PHOFF_OFF            equ         0x1c
ELF_PHDR_PTYPE_OFF            equ         0x00
ELF_PHDR_POFF_OFF             equ         0x04
ELE_PHDR_PFILESZ_OFF          equ         0x10
ELF_PHDR_PVADDR_OFF           equ         0x08
; 程序头段类型
PT_NULL                       equ         0x00; 此类型可忽略

; [bits 32]
elf_load :
; 将在内存中的elf文件加载到正确的内存位置
; 输入: 栈顶 -> | elf_file_addr |
; 加载内核到正确的位置
  push    ebp
  mov     ebp, esp
  pushad
  xor     ecx, ecx
  xor     ebx, ebx
  mov     edx, [ebp + 8]                      ; elf_file_addr
  mov     cx, [edx + ELF_EHDR_PHNUM_OFF]      ; ecx <- pELFHdr->e_phnum
  mov     esi, [edx + ELF_EHDR_PHOFF_OFF]     ; esi <- PELFHdr->e_phoff
  mov     bx, [edx + ELF_EHDR_PHENTSIZE_OFF]  ; ebx <- pElFHdr->e_phentsize
  add     esi, edx                            ; esi <- offset of program head
.relocata_elf :
  push    esi
  push    ecx
  mov     eax, [esi + ELF_PHDR_PTYPE_OFF]     ; p_type
  cmp     eax, PT_NULL
  jz      .no_action
  mov     ecx, [esi + ELE_PHDR_PFILESZ_OFF]   ; Elf32_phdr->p_filesz                
  or      ecx, ecx
  jz      .no_action
  mov     edi, [esi + ELF_PHDR_PVADDR_OFF]    ; Elf32_phdr->p_vaddr
  mov     esi, [esi + ELF_PHDR_POFF_OFF]      ; Elf32_Phdr->p_offset
  add     esi, edx                            ; offset of segment in mem
  cld
  rep     movsb
.no_action :
  pop     ecx
  pop     esi
  add     esi, ebx                                             ; size of program head
  loop    .relocata_elf

  popad
  pop     ebp
  ret