#include "debug.h"
#include "kputx.h"
#include "interrupt.h"

void
panic_spin( char *filename, \
            int line, \
            const char *func, \
            const char *condition )
{
  intr_turn_off( ) ;

  kputstr("\n\n\n!!!!! error !!!!!\n");
  kputstr("filename: "); kputstr(filename); kputchar('\n');
  kputstr("line: "); kput_uint32_dec(line); kputchar('\n');
  kputstr("function: "); kputstr((char *)func); kputchar('\n');
  kputstr("condition: "); kputstr((char *)condition); kputchar('\n');
  asm volatile( "hlt" ) ;
}