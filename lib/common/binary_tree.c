#include "binary_tree.h"


//===============中需遍历
void
bintree_mid_trav( const bintree_t *tree, void (*visit)( const bintree_node_t *tnode ) )
{
    const bintree_node_t *tnode = (const bintree_node_t *)tree->p_root ;
    while( tnode != NULL )
    {
        if( tnode->p_left != NULL )   // stage1,找到该子树的最左节点
        {
            tnode = (const bintree_node_t *)tnode->p_left ;
            continue ;
        }
        else
        {
            visit( tnode ) ;          // 访问该子树的最左节点
            if( tnode->p_right != NULL )  // 如果该节点有右子树,在右子树重新找最左叶节点
            {
                tnode = (const bintree_node_t *)tnode->p_right ;
                continue ;
            }
        }

        // 回溯树指针
        while( TRUE )
        {
            if( tnode->p_parent == NULL ) //此时已经回到树根,遍历结束
              return ;
            if( tnode->p_parent->p_left == tnode ) //如果该节点是父节点的左孩子
            {
                visit( tnode->p_parent ) ;         //访问父节点
                if( tnode->p_parent->p_right != NULL ) //如果父节点有右孩子
                {
                    tnode = (const bintree_node_t *)tnode->p_parent->p_right ; //以该右孩子为新子树,重复stage1
                    break ;
                }
            }
            tnode = (const bintree_node_t *)tnode->p_parent ;     
                                          //如果该子节点是父节点的右孩子
                                          //或者该子节点是父节点的左孩子且父节点没有右孩子
                                          //就继续回溯树指针
        }
    }
    return ;
}


//=======================
// 返回NULL就是没有前驱节点
bintree_node_t *
pre_bintree_node( const bintree_node_t *tnode )   //Precursor of mid-order traval
{
    const bintree_node_t *tmp ;
    const bintree_node_t *tmps ;
    if( tnode->p_left != NULL )
    {
        tmp = (const bintree_node_t *)tnode->p_left ;
        while( tmp->p_right != NULL )
          tmp = (const bintree_node_t *)tmp->p_right ;
    }
    else
    {
        tmp = (const bintree_node_t *)tnode->p_parent ;
        tmps = tnode ;
        while( tmp != NULL && tmp->p_right != tmps )
        {
            tmps = tmp ;
            tmp = (const bintree_node_t *)tmp->p_parent ;
        }
    }
    return (bintree_node_t *)tmp ;
}

//==========================
// 返回NULL就是没有后继节点
bintree_node_t *
suc_bintree_node( const bintree_node_t *tnode )   //Succeed of mid-order traval
{
    const bintree_node_t *tmp ;
    const bintree_node_t *tmps ;
    if( tnode->p_right != NULL )
    {
        tmp = (const bintree_node_t *)tnode->p_right ;
        while( tmp->p_left != NULL )
          tmp = (const bintree_node_t *)tmp->p_left ;
    }
    else
    {
        tmp = (const bintree_node_t *)tnode->p_parent ;
        tmps = tnode ;
        while( tmp != NULL && tmp->p_left != tmps )
        {
            tmps = tmp ;
            tmp = (const bintree_node_t *)tmp->p_parent ;
        }
    }
    return (bintree_node_t *)tmp ;
}

//===================================
// 二叉树搜索
// 没找到就返回NULL
bintree_node_t *
bintree_search( const bintree_t *tree, const void *key )
{
    int cmp ;
    const bintree_node_t *tnode = (const bintree_node_t *)tree->p_root ;

    while( tnode != NULL )
    {
        cmp = tree->compare( key, tree->get_key( tnode ) ) ;
        if( cmp == 0 )
            return (bintree_node_t *)tnode ;
        else if( cmp == 1 )
            tnode = (const bintree_node_t *)tnode->p_right ;
        else
            tnode = (const bintree_node_t *)tnode->p_left ;
    }
    return (bintree_node_t *)tnode ;
}

//===============局部函数===================
bool_t
left_rotate( bintree_t *tree, bintree_node_t *btnode )
{
    bintree_node_t *np = btnode->p_parent, *nr = btnode->p_right  ;
    if( nr == NULL )                                    //如果没有右孩子就不能左旋
      return FALSE ;

    //下面进行旋转
    //处理nr的parent
    nr->p_parent = np ;                                 
    if( np != NULL )                                    //不是根节点
    {
        if( np->p_left == btnode )
            np->p_left = nr ;
        else
            np->p_right = nr ;
    }
    else
      tree->p_root = nr ;                           //是根节点,则sr成为新的树根

    //处理nr的左节点
    if( nr->p_left != NULL )
        nr->p_left->p_parent = btnode ;
    btnode->p_right = nr->p_left ;

    //处理btnode节点
    nr->p_left = btnode ;
    btnode->p_parent = nr ;

    return TRUE ;
}

//---------------
bool_t
right_rotate( bintree_t *tree, bintree_node_t *btnode )
{
    bintree_node_t *np = btnode->p_parent, *nl = btnode->p_left  ;
    if( nl == NULL )                                    //左节点不存在,则无法右旋
      return FALSE ;

    //下面开始旋转
    //处理nl的parent
    nl->p_parent = np ;
    if( np != NULL )                                    //不是根节点
    {
        if( np->p_left == btnode )
            np->p_left = nl ;
        else
            np->p_right = nl ;
    }
    else
      tree->p_root = nl ;                                  //是根节点,重新定位新根节点

    //处理sl的右节点
    if( nl->p_right != NULL )
        nl->p_right->p_parent = btnode ;
    btnode->p_left = nl->p_right ;

    //处理s节点
    nl->p_right = btnode ;
    btnode->p_parent = nl ;

    return TRUE ;
}

