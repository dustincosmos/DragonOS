#pragma once
#include <common/glib.h>

enum rbt_Colortype { red, black } ;

struct rbt_node_t
{
    struct rbt_node_t *left;
    struct rbt_node_t *right;
    struct rbt_node_t *parent;
    void* value;
    enum rbt_Colortype color;
} __attribute__((aligned(sizeof(long))));

struct rbt_root_t
{
    struct rbt_node_t *rbt_node;
    int32_t size;
    int (*cmp)(void* a,void* b);
    int (*release)(void* value);
};


struct rbt_root_t *rbt_create_tree(struct rbt_node_t *node, int (*cmp)(void *a, void *b), int (*release)(void *value));

struct rbt_node_t *rbt_create_node(struct rbt_node_t *left, struct rbt_node_t *right, struct rbt_node_t *parent,enum rbt_Colortype color ,void *value);

int rbt_insert(struct rbt_root_t *root,void* value);

int rbt_query(struct rbt_root_t *root,void *value,uint64_t *ret_addr);

int rbt_delete(struct rbt_root_t *root,void* value);
