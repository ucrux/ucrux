#ifndef __HASH_H
#define __HASH_H
#include "global_type.h"
#include "dlist.h"

#define __BITS_PER_BYTE  8
#define __BITS_OF_HASH_TABLE_SIZE   8
#define __HASH_TABLE_SIZE (1 << __BITS_OF_HASH_TABLE_SIZE )
#define __HASH_UINT_VALUE_SHIT_BITS ( sizeof(uint32_t) * __BITS_PER_BYTE - __BITS_OF_HASH_TABLE_SIZE )
#define __HASH_MAGIC_NUM 0x9e370001U

// hash关键字的函数
typedef uint32_t (*hash_fun_t)( void *key ) ;
// 获取关键字的函数
typedef void *(*hash_getkey_fun_t)( dlist_node_t *lnode ) ;
// 比较关键字的函数
// == 0
// > 1
// < -1
typedef int (*hash_keycmp_fun_t)( void *key1, void *key2 ) ;

typedef struct s_hash_table
{
  hash_fun_t hash_fun ;
  hash_getkey_fun_t get_key ;
  hash_keycmp_fun_t compare ;
  dlist_head_t hashhead[__HASH_TABLE_SIZE] ;
} hash_table_t ;

// 计算无符号32位整数的hash值
uint32_t hash_uint32( void* key ) ;
// 初始化hash表,内存空间由调用者管理
bool_t init_hashtb( hash_table_t *hash_table, \
                    hash_fun_t hash, \
                    hash_getkey_fun_t getkey, \
                    hash_keycmp_fun_t cmp ) ;
// 此hash表中,关键字唯一
// 向哈希表中插入节点,
// 成功返回 TRUE
// 失败返回 FALSE
bool_t insert_hashtb( hash_table_t *hash_table, dlist_node_t *lnode ) ;
// 在哈希表中查找关键字,返回找打节点的指针,不存在返回 NULL 
dlist_node_t *srch_hashtb( hash_table_t *hash_table, void *key ) ;
// 删除包含关键字的节点,并返回节点指针,不存在返回 NULL 
dlist_node_t *del_hashtb( hash_table_t *hash_table, void *key ) ;

#endif