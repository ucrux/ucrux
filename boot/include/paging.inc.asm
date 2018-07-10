; PDE : (bits)
;
; |31|30|29|28|27|26|25|24|23|22|21|20|19|18|17|16|15|14|13|12|11|10| 9| 8| 7| 6| 5| 4| 3| 2| 1| 0|
; |                                                           |________| |  |  |  |  |  |  |  |  |
; |___________________________________________________________|    |     |  |  |  |  |  |  |  |  |--P:present
;                               |                                  |     |  |  |  |  |  |  |  |-----R/W:read/write (0 read, 1 read/write)
;                               |                                  |     |  |  |  |  |  |  |--------U/S:user/supervisor (0 supervisor, 1 user)
;                               |                                  |     |  |  |  |  |  |-----------PWT:write-through
;                               |                                  |     |  |  |  |  |--------------PCD:cache disabled
;                               |                                  |     |  |  |  |-----------------A:accessed
;                               |                                  |     |  |  |--------------------0:reserved (set to 0)
;                               |                                  |     |  |-----------------------PS:page size (0 indicates 4K bytes)
;                               |                                  |     |--------------------------G:global page (ignored)
;                               |                                  |--------------------------------Avail:available for system programmer's user
;                               |-------------------------------------------------------------------page-table base address


; PTE : (bits)
;
; |31|30|29|28|27|26|25|24|23|22|21|20|19|18|17|16|15|14|13|12|11|10| 9| 8| 7| 6| 5| 4| 3| 2| 1| 0|
; |                                                           |________| |  |  |  |  |  |  |  |  |
; |___________________________________________________________|    |     |  |  |  |  |  |  |  |  |--P:present
;                               |                                  |     |  |  |  |  |  |  |  |-----R/W:read/write (0 read, 1 read/write)
;                               |                                  |     |  |  |  |  |  |  |--------U/S:user/supervisor (0 supervisor, 1 user)
;                               |                                  |     |  |  |  |  |  |-----------PWT:write-through
;                               |                                  |     |  |  |  |  |--------------PCD:cache disabled
;                               |                                  |     |  |  |  |-----------------A:accessed
;                               |                                  |     |  |  |--------------------D:dirty
;                               |                                  |     |  |-----------------------PAT:page table attribute index
;                               |                                  |     |--------------------------G:global page (ignored)
;                               |                                  |--------------------------------Avail:available for system programmer's user
;                               |-------------------------------------------------------------------page base address



;----------------------------------------------------------------------------
; some const in paging 
;----------------------------------------------------------------------------
PG_P        equ   1         ; page present
PG_RWR      equ   0         ; R/W  read and excute
PG_RWW      equ   2         ; R/W  read, write and excute
PG_USS      equ   0         ; U/S  supervisor Privilege level
PG_USU      equ   4         ; U/S  user Privilege level
PG_SIZE     equ   0x1000
;----------------------------------------------------------------------------

