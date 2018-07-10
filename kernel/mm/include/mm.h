#ifndef __MM_H
#define __MM_H
#include "dlist.h"
#include "global_type.h"
#include "bitmap.h"
#include "hash.h"

/*
 * 凡事在内核地址空间使用的东西都需要对等映射
 * 因为每一个进程都会有一份内核页表的副本
 * 内核地址空间在地1G内存
 * 内存管理需要的数据结构
 *  
 *   - 虚拟地址管理( 保存在内核地址空间, 对等映射 )
 *     - 虚拟地址管理数据结构
 *   - blob内存分配( 保存在内核地址空间, 对应映射 )
 *     - blob内存分配管理数据结构
 */

typedef enum e_mm_blob_type
{
  BLOB_32 ,
  BLOB_64 ,
  BLOB_128 ,
  BLOB_256 ,
  BLOB_512 ,
  BLOB_1024 ,
  BLOB_2048 ,
  BLOB_TYPE_CNT
} mm_blob_type_t ;


// 内核空间内存分配函数专门有函数分配这个结构的内存
typedef struct s_mm_blobxx
{
  uint32_t vaddr ;
  uint32_t all_blob_cnt ;
  uint32_t remain_blob_cnt ;
  mm_blob_type_t btype ;
  dlist_node_t lnode ;    //此节点用于hash表
  dlist_node_t freenode ; //此节点用于空闲列表
  bitmap_t bmap ;
} mm_blobxx_t ;

// 用于blob分配的只会有一个页面
// 释放内存的时候根据其物理地址判断是从那个blob中分配
typedef struct s_mm_blob32
{
  uint32_t vaddr ;
  uint32_t all_blob_cnt ;
  uint32_t remain_blob_cnt ;
  mm_blob_type_t btype ;
  dlist_node_t lnode ;    //此节点用于hash表
  dlist_node_t freenode ; //此节点用于空闲列表
  bitmap32_t bmap ;
} mm_blob32_t ;

typedef struct s_mm_blob64
{
  uint32_t vaddr ;
  uint32_t all_blob_cnt ;
  uint32_t remain_blob_cnt ;
  mm_blob_type_t btype ;
  dlist_node_t lnode ;    //此节点用于hash表
  dlist_node_t freenode ; //此节点用于空闲列表
  bitmap64_t bmap ;
} mm_blob64_t ;

typedef struct s_mm_blob128
{
  uint32_t vaddr ;
  uint32_t all_blob_cnt ;
  uint32_t remain_blob_cnt ;
  mm_blob_type_t btype ;
  dlist_node_t lnode ;    //此节点用于hash表
  dlist_node_t freenode ; //此节点用于空闲列表
  bitmap128_t bmap ;
} mm_blob128_t ;

typedef struct s_mm_blob256
{
  uint32_t vaddr ;
  uint32_t all_blob_cnt ;
  uint32_t remain_blob_cnt ;
  mm_blob_type_t btype ;
  dlist_node_t lnode ;    //此节点用于hash表
  dlist_node_t freenode ; //此节点用于空闲列表
  bitmap256_t bmap ;
} mm_blob256_t ;

typedef struct s_mm_blob512
{
  uint32_t vaddr ;
  uint32_t all_blob_cnt ;
  uint32_t remain_blob_cnt ;
  mm_blob_type_t btype ;
  dlist_node_t lnode ;    //此节点用于hash表
  dlist_node_t freenode ; //此节点用于空闲列表
  bitmap512_t bmap ;
} mm_blob512_t ;

typedef struct s_mm_blob1024
{
  uint32_t vaddr ;
  uint32_t all_blob_cnt ;
  uint32_t remain_blob_cnt ;
  mm_blob_type_t btype ;
  dlist_node_t lnode ;    //此节点用于hash表
  dlist_node_t freenode ; //此节点用于空闲列表
  bitmap1024_t bmap ;
} mm_blob1024_t ;

typedef struct s_mm_blob2048
{
  uint32_t vaddr ;
  uint32_t all_blob_cnt ;
  uint32_t remain_blob_cnt ;
  mm_blob_type_t btype ;
  dlist_node_t lnode ;    //此节点用于hash表
  dlist_node_t freenode ; //此节点用于空闲列表
  bitmap2048_t bmap ;
} mm_blob2048_t ;

typedef struct s_mm_blob
{
  // blob分配内存有空闲块的 mm_blobxx_t 链表
  dlist_head_t barray[BLOB_TYPE_CNT] ;
  // 虚拟地址的hash表
  hash_table_t hashtb ;
} mm_blob_t ;

// 初始化mm_blob_t结构,mblob的内存由调用者管理
bool_t init_mm_blob( mm_blob_t *mblob, \
                     hash_fun_t hash, \
                     hash_getkey_fun_t getkey, \
                     hash_keycmp_fun_t cmp  ) ;
// 初始化 mm_blobxx_t 节点, blobxx节点由点用者管理
bool_t init_mm_blobxx( mm_blobxx_t* blobxx, mm_blob_type_t btype, uint32_t vaddr ) ;
// 向 mm_blob_t 插入 mm_blobxx_t节点
// 向 barray[btype] 加入一个节点(barray[btype]之前必须 == NULL)
// 将新节点插入到 hashtb
bool_t insert_mm_blobxx( mm_blob_t *mblob, mm_blobxx_t *blobxx, mm_blob_type_t btype ) ;
// 从 mm_blob_t 中删除 mm_blobxx_t 节点
mm_blobxx_t *delete_mm_blobxx( mm_blob_t *mblob, uint32_t vaddr ) ;
// 分配一个单位的blob内存,成功返回这块内存的虚拟地址
uint32_t alloc_blob( mm_blob_t *mblob, mm_blob_type_t btype ) ;
// 释放一个单位的blob内存
// 如果这个blob已经全空,就返回这个blob的线性地址
// 否则就返回 0
// 如果释放的内存不在blob分配里面或者出错,返回 -1
int free_blob( mm_blob_t *mblob, uint32_t vaddr ) ;

#endif


