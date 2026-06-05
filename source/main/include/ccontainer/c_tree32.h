#ifndef __CCONTAINER_TREE_INDICES_H__
#define __CCONTAINER_TREE_INDICES_H__
#include "ccore/c_target.h"
#ifdef USE_PRAGMA_ONCE
#    pragma once
#endif

namespace ncore
{
    struct arena_t;

    // binary balanced search tree implemented using the red-black tree algorithm
    namespace ntree32
    {
        typedef u32 index_t;
        typedef u32 node_t;

        enum echild_t
        {
            LEFT  = 0,
            RIGHT = 1,
        };

        enum ecolor_t
        {
            RED   = LEFT,
            BLACK = RIGHT
        };

        const u32    c_invalid_index = 0xFFFFFFFF;
        const node_t c_invalid_node  = 0xFFFFFFFF;

        // a and b are the indices of nodes/items to compare, user_data is a pointer to the data that is passed to the tree
        typedef s8 (*compare_fn)(u32 a, u32 b, void const* user_data);

        struct tree_t
        {
            arena_t* m_nodes;
            u32      m_count;
            u32      m_free_head;
        };

        struct nnode_t
        {
            node_t m_child[2];
        };

        void      tree_reset(tree_t* tree);
        inline s8 tree_getdir(s8 compare) { return (compare + 1) >> 1; }
        void      tree_set_color(tree_t* tree, node_t node, u8 color);
        u8        tree_get_color(tree_t const* tree, node_t const node);
        node_t    tree_get_node(tree_t const* tree, node_t const node, s8 ne);
        void      tree_set_node(tree_t* tree, node_t node, s8 ne, node_t set);
        node_t    tree_new_node(tree_t* tree);
        void      tree_del_node(tree_t* tree, node_t node);

        struct iterator_t
        {
            tree_t* m_tree;
            node_t  m_root;
            node_t  m_it;
            node_t  m_stack_array[32];
            s32     m_stack;
        };

        void iterator_setup(iterator_t* iter, tree_t* tree, node_t root);
        bool iterator_traverse(iterator_t* iter, s32 d, node_t& node);
        bool iterator_preorder(iterator_t* iter, s32 d, node_t& node);
        bool iterator_sortorder(iterator_t* iter, s32 d, node_t& node);
        bool iterator_postorder(iterator_t* iter, s32 d, node_t& node);

        void tree_setup(tree_t* tree, u32 max_nodes);
        void tree_teardown(tree_t* tree);

        u32  tree_get_capacity(tree_t const* tree);
        void tree_set_capacity(tree_t* tree, u32 capacity);
        u32  tree_get_used_capacity(tree_t const* tree);

        bool tree_clear(tree_t* tree, node_t& root, node_t& n);  // Repeatedly call 'clear' until true is returned
        bool tree_find(tree_t const* tree, node_t root, index_t key, compare_fn comparer, void const* user_data, node_t& found);
        bool tree_insert(tree_t* tree, node_t& root, node_t temp, index_t key, compare_fn comparer, void const* user_data, node_t& inserted_or_found);
        bool tree_remove(tree_t* tree, node_t& root, node_t temp, index_t key, compare_fn comparer, void const* user_data, node_t& removed);
        bool tree_validate(tree_t const* tree, node_t root, const char*& error_str, compare_fn comparer, void const* user_data);
        void tree_iterate(iterator_t* iter, tree_t* tree, node_t root);

    }  // namespace ntree32

}  // namespace ncore

#endif
