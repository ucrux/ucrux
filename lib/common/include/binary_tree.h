#ifndef __BINARY_TREE_H
#define __BINARY_TREE_H
#include "global_type.h"

//用户需要定义   
//
//        
//         visit函数,用来遍历树
//           compare函数 返回1 a>b, 返回0 a=b, 返回-1 a<b,(降序)
//           返回1 a<b, 返回0 a=b, 返回-1 a>b,(升序)
//
//另外,关键字惟一


//二叉树的节点
typedef struct s_binary_tree_node bintree_node_t ;
struct s_binary_tree_node
{
    bintree_node_t *p_left ;
    bintree_node_t *p_right ;
    bintree_node_t *p_parent ;
} ;

//二叉树的根
typedef struct s_bintree bintree_t ;
struct s_bintree
{
  bintree_node_t *p_root ;
  uint32_t tnode_cnt ;                                    // 二叉树中的节点总数
  void *(*get_key)( const bintree_node_t *tnode ) ;       // 获取二叉树节点的关键字
  int (*compare)( const void *key1, const void *key2 ) ;  // 用于比较两个key值大小的函数
} ;

//二叉树中序遍历
void bintree_mid_trav( const bintree_t *tree, void (*visit)( const bintree_node_t *tnode ) ) ;

//二叉树搜索
bintree_node_t *bintree_search( const bintree_t *tree, const void *key ) ;

//二叉树,某节点的前驱节点(中序历遍顺序)
bintree_node_t *pre_bintree_node( const bintree_node_t *tnode ) ;

//二叉树,某节点的后继节点(中序历遍顺序)
bintree_node_t *suc_bintree_node( const bintree_node_t *tnode ) ;

//以 avl_node-avl_node->p_right 为轴左旋.成功返回TRUE,否则返回FALSE
bool_t left_rotate( bintree_t *tree, bintree_node_t *btnode ) ;
//以 avl_node-alv_node->p_left 为轴右旋.成功返回TRUE,否则返回FALSE
bool_t right_rotate( bintree_t *tree, bintree_node_t *btnode ) ; 

#endif
