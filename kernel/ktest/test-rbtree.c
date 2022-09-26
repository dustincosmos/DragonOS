#include "ktest.h"
#include <ktest/ktest_utils.h>

#include <common/unistd.h>
#include <common/kprint.h>
#include <common/rbtree.h>
#include <common/errno.h>

#include <mm/slab.h>

struct test_value_t
{
    uint64_t tv;
};
static int compare(void *a, void *b)
{
    if (((struct test_value_t *)a)->tv > ((struct test_value_t *)b)->tv)
        return 1;
    else if (((struct test_value_t *)a)->tv == ((struct test_value_t *)b)->tv)
        return 0;
    else
        return -1;
}

static int release(void *value)
{
    // kdebug("release");
}

/**
 * @brief 测试创建红黑树
 *
 * @return int
 */
static long ktest_rbtree_case1(uint64_t arg0, uint64_t arg1)
{
    //======测试初始化
    int flag=rbt_CreateNil();

    // ========== 测试创建树
    struct test_value_t *tv1 = (struct test_value_t *)kmalloc(sizeof(struct test_value_t), 0);
    struct test_value_t *tv2 = (struct test_value_t *)kmalloc(sizeof(struct test_value_t), 0);
    struct test_value_t *tv3 = (struct test_value_t *)kmalloc(sizeof(struct test_value_t), 0);
    struct test_value_t *tv4 = (struct test_value_t *)kmalloc(sizeof(struct test_value_t), 0);
    struct test_value_t *tv5 = (struct test_value_t *)kmalloc(sizeof(struct test_value_t), 0);
    struct test_value_t *tv6 = (struct test_value_t *)kmalloc(sizeof(struct test_value_t), 0);
    struct test_value_t *tv7 = (struct test_value_t *)kmalloc(sizeof(struct test_value_t), 0);
    struct test_value_t *tv8 = (struct test_value_t *)kmalloc(sizeof(struct test_value_t), 0);
    tv1->tv = 20;
    tv2->tv = 10;
    tv3->tv = 30;
    tv4->tv = 25;
    tv5->tv = 35;
    tv6->tv = 33;
    tv7->tv = 40;
    tv8->tv = 38;

    //测试创建树
    struct rbt_node_t *node = rbt_create_node(tv1);
    assert(tv1->tv==20);
    assert(node != NULL);
    struct rbt_root_t *r = rbt_create_tree(node, compare, release);
    assert(r->cmp == compare);

    //测试创建节点
    int j1 = rbt_insert(r, tv2);
    assert(r->rbt_node->left->value==tv2);
    assert(r->rbt_node->left->color==red);
    int j2 = rbt_insert(r, tv3);
    assert(r->rbt_node->right->value==tv3);
    assert(r->rbt_node->right->color==red);
    
    int j3 = rbt_insert(r, tv4);
    int j4 = rbt_insert(r, tv5);
    int j5 = rbt_insert(r, tv6);
    int j6 = rbt_insert(r, tv7);
    int j7 = rbt_insert(r, tv8);

    //测试删除节点
    rbt_delete(r, tv5);
    rbt_delete(r, tv8);
    return 0;
}

static ktest_case_table kt_bitree_func_table[] = {
    ktest_rbtree_case1,
};

uint64_t ktest_test_rbtree(uint64_t arg)
{
    kTEST("Testing rbtree...");
    for (int i = 0; i < sizeof(kt_bitree_func_table) / sizeof(ktest_case_table); ++i)
    {
        kTEST("Testing case %d", i);
        kt_bitree_func_table[i](0, 0);
    }
    kTEST("rbtree Test done.");
    return 0;
}