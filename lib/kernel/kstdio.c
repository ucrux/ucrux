#include "kstdio.h"
#include "kputx.h"
#include "stdio.h"

void
printfk( const char *format, ...)
{
  va_list args ;
  char buf[1024] = {0};
  va_start( args, format );
  vsnprintf( buf, 1024, format, args );
  va_end( args );
  kputstr( buf );
}