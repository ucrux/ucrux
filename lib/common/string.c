#include "string.h"
#include "global_type.h"


void 
memset( void *begin_addr, uint8_t val, uint32_t size )
{
  uint32_t loop ;
  if( begin_addr == NULL )
    return ;
  uint8_t *tmp = (uint8_t *)begin_addr ;
  for( loop = 0 ; loop < size ; loop++ )
    tmp[loop] = val ;
}

// 将src的前size个字节拷贝到dst中
void 
memcpy( void *dst, const void *src, uint32_t size )
{
  uint8_t *dst_ = (uint8_t *)dst ;
  const uint8_t *src_ = (const uint8_t *)src ;
  if( dst == NULL || src == NULL )
    return ;
  while( size-- > 0 )
    *dst_++ = *src_++ ;
}
// 比较两段内存前 size个字节.
// src > dest,return 1 ;
// src == dest,return 0 ;
// src < dest, return -1 ;
int
memcmp( const void *src, const void *dst, uint32_t size )
{
  const uint8_t *src_ = (const uint8_t *)src ;
  const uint8_t *dst_ = (const uint8_t *)dst ;
  if( src == NULL && dst == NULL )
    return 0 ;
  if( src == NULL )
    return -1 ;
  if( dst == NULL )
    return 1 ;

  while( size-- > 0 )
  {
    if( *src_ > *dst_ )
      return 1 ;
    if( *src_ < *dst_ )
      return -1 ;
    src_++ ;
    dst_++ ;
  }

  return 0 ;
}

// 字符串长度
uint32_t 
strlen( const char *str )
{
  const char *tmp = str ;
  if( str == NULL )
    return 0 ;
  while( *tmp++ ) ;
  return (tmp - str - 1) ;
}
// 字符串复制
void 
strcpy( char *dst, const void *src )
{
  char *dst_ = dst ;
  const char *src_ = src ;
  if( dst == NULL || src == NULL )
    return ;
  while( *src_ )
    *dst_++ = *src_++ ;
  *dst_ = '\0' ;
}
// 字符串比较
int 
strcmp( const char *str1, const char *str2 )
{
  const char *str1_ = str1 ;
  const char *str2_ = str2 ;
  if( str1 == NULL && str2 == NULL )
    return 0 ;
  if( str1 == NULL )
    return -1 ;
  if( str2 == NULL )
    return 1 ;
  while( *str1_ && *str2_ )
  {
    if( *str1_ > *str2_ )
      return 1 ;
    if( *str1_ < *str2_ )
      return -1 ;
    str1_++ ;
    str2_++ ;
  }

  if( *str1_ == '\0' && *str2_ == '\0' )
    return 0 ;
  if( *str1_ == '\0' )
    return -1 ;
  return 1 ;
}


// 将src连接到dst后面,返回连接后的地址
char *
strcat( char *dst, const char *src )
{
  char *dst_ = dst ;
  const char *src_ = src ;
  uint32_t dstlen ;
  if( dst == NULL )
    return (char *)src ;
  
  dstlen = strlen( dst ) ;

  dst_ += dstlen ;
  while( src_ != NULL && *src_ )
    *dst_++ = *src_++ ;
  *dst_ = '\0' ;
  return dst ;
}

// 从左向右查找第一次出现ch的地址
char *
strchr( const char *str, char ch )
{
  const char *str_ = str ;
  if( str == NULL )
    return NULL ;
  while( *str_ && *str_ != ch )
    str_++ ;
  if( *str_ )
    return (char *)str_ ;
  return NULL ;
}
// 从右到左查找第一次出现ch的地址
char *
strrchr( const char *str, char ch )
{
  const char *str_ = str ;
  uint32_t len ;
  if( str == NULL )
    return NULL ;
  len = strlen( str_ ) ;
  str_ += len - 1 ;
  while ( str_ >= str && *str_ != ch )
    str_-- ;
  if( str_ >= str )
    return (char *)str_ ;
  
  return NULL ;
}
// 查找ch再字符串中出现的次数
uint32_t
strchrs( const char *str, char ch )
{
  const char *str_ = str ;
  uint32_t count = 0 ;
  if( str == NULL )
    return count ;
  while( *str_ )
    if( *str_++ == ch )
      count++ ;

  return count ;
}

