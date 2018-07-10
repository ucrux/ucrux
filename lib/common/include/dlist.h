#ifndef __DOUBLE_LIST_H
#define __DOUBLE_LIST_H
#include "global_type.h"

typedef struct s_dlist_node dlist_node_t ;
typedef struct s_dlist_head dlist_head_t ;


struct s_dlist_node
{
  dlist_head_t *dlhead ;
  struct s_dlist_node *prev ;
  struct s_dlist_node *next ;
}  ;

struct s_dlist_head
{
  unsigned int ncnt ;             //此链表的节点数量
  dlist_node_t head ;
  dlist_node_t tail ;
}  ;

// 历遍每一个节点
#define for_each_dlnode( dlhead, dlnode ) for( (dlnode) = (dlhead)->head.next ; (dlnode) != &((dlhead)->tail) ; (dlnode) = (dlnode)->next )
// 初始化链表,内存空间由调用者提供
bool_t init_dlist( dlist_head_t *dlhead ) ;
// 初始化链表节点,内存空间由调用者提供
bool_t init_dlnode( dlist_node_t *dlnode ) ;
// 删除链表节点,返回指向被删除节点的指针,返回的内存空间由调用者管理
dlist_node_t *del_dlnode( dlist_node_t *dlnode ) ;
// 向链表中添加节点,新节点的内存空间由调用者提供
bool_t add_dlnode( dlist_node_t *prev, dlist_node_t *new ) ;     //after prev,add new
//travfun is a bool_t function, if true, return dlist_node_t which make the travfun true
dlist_node_t *trav_dlist( dlist_head_t *dlhead, bool_t (*travfun)( void *arg1, const dlist_node_t *arg2 ),  void *arg1 ) ;

#endif