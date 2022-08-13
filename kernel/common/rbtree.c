#include "rbtree.h"
#include <mm/slab.h>
#include <common/errno.h>
#include <debug/bug.h>

#define smaller(root, a, b) (root->cmp(a, b) == -1)
#define equal(root, a, b) (root->cmp(a, b) == 0)
#define greater(root, a, b) (root->cmp(a, b) == 1)

#define rbt_r 0
#define rbt_b 1

static struct rbt_node_t *rbt_nil;
struct rbt_root_t *rbt_create_tree(struct rbt_node_t *node, int (*cmp)(void *a, void *b), int (*release)(void *value))
{
    if (node == NULL || cmp == NULL)
        return -EINVAL;
    struct rbt_root_t *root = (struct rbt_root_t *)kmalloc(sizeof(struct rbt_root_t), 0);
    memset((void *)root, 0, sizeof(struct rbt_root_t));
    root->rbt_node = node;
    root->release = release;

    root->rbt_node->parent = rbt_nil;
    root->rbt_node->left = rbt_nil;
    root->rbt_node->right = rbt_nil;
    root->rbt_node->color = rbt_b;

    return root;
}

struct rbt_node_t *rbt_create_node(void *value)
{
    struct rbt_node_t *node = (struct rbt_node_t *)kmalloc(sizeof(struct rbt_node_t), 0);

    rbt_nil = (struct rbt_node_t *)kmalloc(sizeof(struct rbt_node_t), 0);
    rbt_nil->left = rbt_nil;
    rbt_nil->right = rbt_nil;
    rbt_nil->color = rbt_b;
    FAIL_ON_TO(node == rbt_nil, nomen);
    memset((void *)node, 0, sizeof(struct rbt_node_t));

    node->left = rbt_nil;
    node->right = rbt_nil;
    node->color = rbt_r;
    node->parent = rbt_nil;

    return node;
nomen:;
    return -ENOMEM;
}

int rbt_query(struct rbt_root_t *root, void *value, uint64_t *ret_addr)
{
    struct rbt_node_t *this_node = root->rbt_node;
    struct rbt_node_t tmp_node = {0};
    tmp_node.value = value;

    if (ret_addr == rbt_nil)
        return -EINVAL;

    while (this_node != rbt_nil && !equal(root, this_node, &tmp_node))
    {
        if (smaller(root, &tmp_node, this_node))
            this_node = this_node->left;
        else
            this_node = this_node->right;
    }

    if (this_node != rbt_nil && equal(root, this_node, &tmp_node))
    {
        *ret_addr = (uint64_t)this_node;
        return 0;
    }
    else
    {
        *ret_addr = rbt_nil;
        return -1;
    }
}

/**
 * @brief 右旋
 *
 * @param root 树根 //在这里没有用上
 * @param node 待旋转的节点
 */
static struct rbt_node_t *left_rotate(struct rbt_root_t *root, struct rbt_node_t *node)
{
    struct rbt_node_t *tmp = node->right;
    node->right = tmp->left;
    if (tmp->left != rbt_nil)
        tmp->left->parent = node;
    tmp->parent = node->parent;
    if (node->parent == rbt_nil)
        root->rbt_node = tmp;
    else if (node == node->parent->left)
        node->parent->left = tmp;
    else
        node->parent->right = tmp;
    tmp->left = node;
    node->parent = tmp;
}

/**
 * @brief 左旋
 *
 * @param root 树根
 * @param node 待旋转的节点
 */
static void left_rotate(struct rbt_root_t *root, struct rbt_node_t *node)
{
    struct rbt_node_t *tmp = node->left;
    node->left = tmp->right;
    if (tmp->right != rbt_nil)
        tmp->right->parent = node;
    tmp->parent = node->parent;
    if (node->parent == rbt_nil)
        root->rbt_node = tmp;
    else if (node == node->parent->left)
        node->parent->left = tmp;
    else
        node->parent->right = tmp;
    tmp->right = node;
    node->parent = tmp;
}

int rbt_insert(struct rbt_root_t *root, void *value)
{
    if (root == rbt_nil)
        return -EINVAL;
    struct rbt_node_t *current_node = root->rbt_node;
    struct rbt_node_t *last_node = rbt_nil;
    struct rbt_node_t *insert_node = rbt_create_node(value);
    FAIL_ON_TO((uint64_t)insert_node == (uint64_t)(-ENOMEM), failed);

    while (current_node != rbt_nil)
    {
        last_node = current_node;
        if (smaller(root, insert_node, current_node))
            current_node = current_node->left;
        else
            current_node = current_node->right;
    }

    insert_node->parent = last_node;
    if (unlikely(last_node == rbt_nil))
        root->rbt_node = insert_node;
    else
    {
        if (smaller(root, insert_node, last_node))
            last_node->left = insert_node;
        else
            last_node->right = insert_node;
    }

    ++root->size;
    rbt_insert_fixup(root, insert_node);
    return 0;
failed:;
    return -ENOMEM;
}

int rbt_delete(struct rbt_root_t *root, void *value)
{
    struct rbt_node_t *bedelete_node = root->rbt_node;
    struct rbt_node_t tmp_node = {0};
    tmp_node.value = value;

    while (bedelete_node != rbt_nil && !equal(root, bedelete_node, &tmp_node))
    {
        if (smaller(root, &tmp_node, bedelete_node))
            bedelete_node = bedelete_node->left;
        else
            bedelete_node = bedelete_node->right;
    }
    struct rbt_node_t *replace_node = bedelete_node;
    int replace_node_origincolor = replace_node->color;
    struct rbt_node_t *tmp = rbt_nil;
    if (bedelete_node->left == rbt_nil)
    {
        tmp = bedelete_node->right;
        rbt_conversion(root, bedelete_node, bedelete_node->right);
    }
    else if (bedelete_node->right == rbt_nil)
    {
        tmp = bedelete_node->left;
        rbt_conversion(root, bedelete_node, bedelete_node->left);
    }
    else
    {
        replace_node = rbt_findmin(bedelete_node->right);
        replace_node_origincolor = replace_node->color;
        tmp = replace_node->right;
        if (replace_node->parent == bedelete_node)
            tmp->parent = replace_node;
        else
        {
            rbt_conversion(root, replace_node, replace_node->right);
            replace_node->right = bedelete_node->right;
            replace_node->right->parent = replace_node;
        }
        rbt_conversion(root, bedelete_node, replace_node);
        replace_node->left = bedelete_node->left;
        replace_node->left->parent = replace_node;
        replace_node->color = bedelete_node->color;
    }

    if (replace_node_origincolor == rbt_b)
        rbt_delete_fixup(root, tmp);
}

/**
 * @brief 进行插入后的颜色修复
 * 
 * @param root 
 * @param node 待插入节点
 */
static void rbt_insert_fixup(struct rbt_root_t *root, struct rbt_node_t *node)
{
    while (node->parent->color == rbt_r)
    {
        if (node->parent == node->parent->parent->left)
        {
            struct rbt_node_t *node_uncle = node->parent->parent->right;
            if (node_uncle->color == rbt_r)
            {
                node->parent->color = rbt_b;
                node_uncle->color = rbt_b;
                node->parent->parent->color = rbt_r;
                node = node->parent->parent;
            }
            else if (node == node->parent->right)
            {
                node = node->parent;
                left_rotate(root,node);
            }
            node->parent->color = rbt_b;
            node->parent->parent->color = rbt_r;
            right_rotate(root,node->parent->parent);
        }
        else
        {
            struct rbt_node_t *node_uncle = node->parent->parent->left;
            if (node_uncle->color == rbt_r)
            {
                node->parent->color = rbt_b;
                node_uncle->color = rbt_b;
                node->parent->parent->color = rbt_r;
                node = node->parent->parent;
            }
            else if (node = node->parent->right)
            {
                node = node->parent;
                left_rotate(root,node);
            }
            node->parent->color = rbt_b;
            node->parent->parent->color = rbt_r;
            right_rotate(root,node->parent->parent);
        }
        root->rbt_node->color = rbt_b;
    }
}

/**
 * @brief 进行删除后的颜色修复
 * 
 * @param r 
 * @param node 删除节点的替换节点
 */
static void rbt_delete_fixup(struct rbt_root_t *r, struct rbt_node_t *node)
{
    struct rbt_node_t *tmp = rbt_nil;
    while (node != r->rbt_node && node->color == rbt_b)
    {
        if (node == node->parent->left)
        {
            tmp = node->parent->left;
            if (tmp->color == rbt_r)
            {
                tmp->color = rbt_b;
                node->parent->color = rbt_r;
                left_rotate(r, node->parent);
                tmp = node->parent->left;
            }
            if (tmp->left->color == rbt_b && tmp->right->color == rbt_b)
            {
                tmp->color == rbt_r;
                node = node->parent;
            }
            else if (tmp->right->color == rbt_b)
            {
                tmp->left->color = rbt_b;
                tmp->color = rbt_r;
                right_rotate(r, tmp);
                tmp = node->parent->right;

                tmp->color = node->parent->color;
                node->parent->color = rbt_b;
                tmp->right->color = rbt_b;
                left_rotate(r, node->parent);
                node = r->rbt_node;
            }
        }
        else
        {
            tmp = node->parent->right;
            if (tmp->color == rbt_r)
            {
                tmp->color = rbt_b;
                node->parent->color = rbt_r;
                right_rotate(r, node->parent);
                tmp = node->parent->right;
            }
            if (tmp->left->color == rbt_b && tmp->right->color == rbt_b)
            {
                tmp->color == rbt_r;
                node = node->parent;
            }
            else if (tmp->right->color == rbt_b)
            {
                tmp->left->color = rbt_b;
                tmp->color = rbt_r;
                right_rotate(r, tmp);
                tmp = node->parent->right;

                tmp->color = node->parent->color;
                node->parent->color = rbt_b;
                tmp->right->color = rbt_b;
                left_rotate(r, node->parent);
                node = r->rbt_node;
            }
        }
    }
}

/**
 * @brief 把b节点替换到a节点
 * 
 * @param r 
 * @param a 
 * @param b 
 */
static void rbt_conversion(struct rbt_root_t *r, struct rbt_node_t *a, struct rbt_node_t *b)
{
    if (a->parent == rbt_nil)
        r->rbt_node = b;
    else if (a = a->parent->left)
        a->parent->left = b;
    else
        a->parent->right = b;
    b->parent = a->parent;
    kfree(a);
}

/**
 * @brief 查找树的最小值
 * 
 * @param node 待查的根节点
 * @return struct rbt_node_t* 
 */
static struct rbt_node_t *rbt_findmin(struct rbt_node_t *node)
{
    struct rbt_node_t *tmp = node;
    if (node == rbt_nil)
        return rbt_nil;
    while (tmp != rbt_nil)
        tmp = tmp->left;
    return tmp;
}