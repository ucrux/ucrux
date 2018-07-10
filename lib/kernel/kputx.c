#include "kputx.h"
#include "disp_char_with_color.h"
#include "binary32_to_packed_bcd.h"

#define __SRCEEN_ATTR_BLACK_WRITE 0x07


void
kputchar( char ch )
{
  disp_char_with_color( ch, __SRCEEN_ATTR_BLACK_WRITE ) ;
}

void
kputstr( const char *str )
{
  if( str == NULL )
    return ;
  while( *str != '\0' )
  {
    kputchar( *str ) ;
    str++ ;
  }
}

//以2进制打印一个uint32_t类型
void
kput_uint32_bin( uint32_t num )
{
  uint32_t mask = 0x00000001 ;
  int loop = __LENGTH_OF_INT ;
  uint32_t tmpnum ;
  int begin_with_zore = 1 ;
  char tmpchar ;

  if( num == 0 ) 
  {
    kputstr("0B") ;
    return ;
  }

  while( loop )
  {
    loop-- ;
    tmpnum = num ;
    tmpnum >>= loop  ;
    tmpnum &= mask ;
    if( tmpnum == 1 && begin_with_zore == 1 )
      begin_with_zore = 0 ;
    if( tmpnum == 1 || begin_with_zore == 0 )
    {
      tmpchar = (char )tmpnum ;
      tmpchar += '0' ; 
      kputchar( tmpchar ) ;
    }
  }
  kputchar('B') ;
}

//以16进制形式打印一个uint32_t类型
void
kput_uint32_hex( uint32_t num )
{
  uint32_t mask = 0x0000000f ;
  int loop = __LENGTH_OF_INT ;
  uint32_t tmpnum ;
  int begin_with_zore = 1 ;
  char tmpchar ;

  if( num == 0 ) 
  {
    kputstr("0x0") ;
    return ;
  }

  kputstr( "0x" ) ;
  while(loop)
  {
    loop -= 4 ;
    tmpnum = num ;
    tmpnum >>= loop ;
    tmpnum &= mask ;
    if( tmpnum != 0 && begin_with_zore == 1 )
      begin_with_zore = 0 ;
    if( tmpnum != 0 || begin_with_zore == 0 )
    {
      tmpchar = (char )tmpnum ;
      if( tmpchar <= 9 )
        tmpchar += '0' ;
      else
        tmpchar = tmpchar - 10 + 'A' ;
      kputchar( tmpchar ) ;
    }
  }
}
//以10进制打印一个uint32_t类型
void
kput_uint32_dec( uint32_t num )
{
  uint64_t ret, tmpnum, mask = 0x0f ;
  char tmpchar ;
  int begin_with_zore = 1 ;
  int loop = __LENGTH_OF_INT_BCD * __COLUMN_SIZE ;
  ret = bin32_to_bcd( num ) ;


  if( num == 0 ) 
  {
    kputstr("0") ;
    return ;
  }

  while( loop )
  {
    loop -= 4 ;
    tmpnum = ret >> loop ;
    tmpnum &= mask ;

    if( tmpnum != 0 && begin_with_zore == 1 )
      begin_with_zore = 0 ;
    if( tmpnum != 0 || begin_with_zore == 0 )
    {
      tmpchar = (char )tmpnum ;
        tmpchar += '0' ;
      kputchar( tmpchar ) ;
    }
  }
}