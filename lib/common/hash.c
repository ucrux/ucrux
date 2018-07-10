#include "hash.h"



// 计算无符号32位整数的hash值
uint32_t
hash_uint32( void *key )
{
  uint32_t val = (uint32_t )key ;
  uint32_t hval = val * __HASH_MAGIC_NUM ;
  return (hval >> __HASH_UINT_VALUE_SHIT_BITS) ;
}


// 初始化hash表,内存空间由调用者管理
bool_t 
init_hashtb( hash_table_t *hash_table, \
             hash_fun_t hash, \
             hash_getkey_fun_t getkey, \
             hash_keycmp_fun_t cmp )
{
  int loop ;

  if( hash_table == NULL )
    return FALSE ;

  for( loop = 0 ; loop < __HASH_TABLE_SIZE ; loop++ )
    if( !init_dlist( &(hash_table->hashhead[loop])) )
      return FALSE ;

  hash_table->hash_fun = hash ;
  hash_table->get_key = getkey ;
  hash_table->compare = cmp ;

  return TRUE ;
}

// 此hash表中,关键字唯一
// 向哈希表中插入节点,
// 成功返回 TRUE
// 失败返回 FALSE
bool_t
insert_hashtb( hash_table_t *hash_table, dlist_node_t *lnode )
{
  uint32_t hashval ;
  dlist_head_t *hashtb_hdr ;
  dlist_node_t *dlnode ;

  if( hash_table == NULL || lnode == NULL )
    return FALSE ;

  hashval = hash_table->hash_fun( hash_table->get_key(lnode) ) ;
  hashtb_hdr = &(hash_table->hashhead[hashval]) ;

  for_each_dlnode( hashtb_hdr, dlnode )
  {
    // 关键字重复
    if( hash_table->compare(hash_table->get_key(dlnode),hash_table->get_key(lnode)) == 0 )
      return FALSE ;
  }

  if( add_dlnode( &(hashtb_hdr->head), lnode ) == FALSE )
    return FALSE ;

  return TRUE ;
}

// 在哈希表中查找关键字,返回找打节点的指针,不存在返回 NULL 
dlist_node_t *
srch_hashtb( hash_table_t *hash_table, void *key )
{
  uint32_t hashval ;
  dlist_head_t *hashtb_hdr ;
  dlist_node_t *dlnode = NULL ;

  hashval = hash_table->hash_fun( key ) ;
  hashtb_hdr = &(hash_table->hashhead[hashval]) ;


  for_each_dlnode( hashtb_hdr, dlnode )
  {
    // 找到关键字
    if( hash_table->compare(hash_table->get_key(dlnode), key) == 0 )
    {
      return dlnode ;
      break ;
    }
  }
  return NULL ;
}

dlist_node_t *
del_hashtb( hash_table_t *hash_table, void *key )
{
  uint32_t hashval ;
  dlist_head_t *hashtb_hdr ;
  dlist_node_t *dlnode = NULL ;

  hashval = hash_table->hash_fun( key ) ;
  hashtb_hdr = &(hash_table->hashhead[hashval]) ;

  for_each_dlnode( hashtb_hdr, dlnode )
  {
    // 找到关键字
    if( hash_table->compare(hash_table->get_key(dlnode), key) == 0 )
    {
      del_dlnode( dlnode ) ;
      return dlnode ;
      break ;
    }
  }

  return NULL ;
}