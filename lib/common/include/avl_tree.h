#ifndef __AVL_TREE_H
#define __AVL_TREE_H
#include "global_type.h"
#include "binary_tree.h"

// 定义平衡因子变量
typedef enum e_balance
{
  EQU_HEIGHT,       // 左右一样高
  LEFT_HIGH,        // 左高
  RIGHT_HIGH        // 右高
} balance_t ;




//数据结构定义
typedef struct s_avl_tree_node avl_tree_node_t ;
struct s_avl_tree_node
{
    balance_t balance ;
    bintree_node_t btnode ;
} ;

typedef bintree_t avltree_t ; 

//初始化二叉平衡树,*avl_tree的内存空间由调用者管理
bool_t init_avl_tree( avltree_t *avl_tree, \
  void *(*getkey)( const bintree_node_t *btnode ), \
  int (*cmp)( const void *key1, const void *key2 ) ) ;

// avl_node 必须是个指针
#define avl_get_left_child( avl_node ) \
  get_struct_pointor( avl_tree_node_t, btnode, (avl_node)->btnode.p_left ) ;
#define avl_get_right_child( avl_node ) \
  get_struct_pointor( avl_tree_node_t, btnode, (avl_node)->btnode.p_right ) ;


//初始化二叉平衡树节点, avl_tree_node的内存空间由调用者管理
bool_t init_avl_tree_node( avl_tree_node_t *avl_node ) ;

//插入节点, 
bool_t insert_avl( avltree_t *avl_tree, avl_tree_node_t *avl_node ) ;

//删除节点
avl_tree_node_t *delete_avl( avltree_t *avl_tree, void *key ) ;


#endif