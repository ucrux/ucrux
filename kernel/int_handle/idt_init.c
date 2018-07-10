#include "idt_init.h"
#include "kernel.h"


// 声明中断处理函数入口地址列表
extern uint32_t intr_handle_entry_table[__IDT_DESC_COUNT] ;
static gate_desc_t idt[__IDT_DESC_COUNT] ;

static inline void mk_intr_gate_desc( uint32_t func_entry_addr, gate_desc_t *intr_gate_desc, uint32_t attr ) ;


static inline void
mk_intr_gate_desc( uint32_t func_entry_addr, gate_desc_t *intr_gate_desc, uint32_t attr )
{
  intr_gate_desc->func_offset_low = (uint16_t )(func_entry_addr & 0x0000ffff) ;
  intr_gate_desc->cs_selector = __SELECTOR_KERNEL_CODE ;
  intr_gate_desc->param_count = 0 ;
  intr_gate_desc->gate_attr = attr ;
  intr_gate_desc->func_offset_high = (uint16_t)( (func_entry_addr & 0xffff0000) >> 16 ) ;
}


void
idt_init( void )
{
  uint32_t loop ;
  uint64_t lidt_operand ;
  //初始化中断门描述符表
  for( loop = 0 ; loop < __IDT_DESC_COUNT - 1 ; loop++ )
    mk_intr_gate_desc( intr_handle_entry_table[loop], &idt[loop], __IDT_DESC_ATTR_DPL0 ) ;

  mk_intr_gate_desc( intr_handle_entry_table[__IDT_DESC_COUNT-1], \
                     &idt[__IDT_DESC_COUNT-1], __IDT_DESC_ATTR_DPL3 ) ;

  lidt_operand = (sizeof(idt)-1) | (((uint64_t )(uint32_t )idt) << 16) ;
  asm volatile ("lidt %0" : : "m"(lidt_operand)) ;
}