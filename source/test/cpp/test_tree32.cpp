#include "ccore/c_target.h"
#include "ccore/c_allocator.h"
#include "ccore/c_random.h"

#include "ccontainer/c_tree32.h"

#include "cunittest/cunittest.h"

#define TEST_ITERATOR

using namespace ncore;
using namespace ncore::ntree32;

UNITTEST_SUITE_BEGIN(tree32)
{
    UNITTEST_FIXTURE(tree2)
    {
        // UNITTEST_ALLOCATOR;

        const s32  c_find_slot = 62;
        const s32  c_temp_slot = 63;
        const s32  c_num_keys  = 62;
        static s32 s_find[c_num_keys + 2];
        static s32 s_keys[c_num_keys + 2];  // +2 for 'find' and 'temp' slots

        static void s_init_keys()
        {
            for (s32 i = 0; i < c_num_keys; ++i)
            {
                s_find[i] = 33 + i * 33;
                s_keys[i] = -1;
            }
        }

        static s8 s_compare_key_and_node(u32 _key, u32 _item, void const *user_data)
        {
            s32 const *keys = (s32 const *)user_data;
            s32        key  = keys[_key];
            s32        item = keys[_item];
            if (key < item)
                return -1;
            else if (key > item)
                return 1;
            else
                return 0;
        }

        static s8 s_compare_node_and_node(u32 _nodeA, u32 _nodeB, void const *user_data)
        {
            s32 const *keys = (s32 const *)user_data;
            s32        key  = keys[_nodeA];
            s32        item = keys[_nodeB];
            if (key < item)
                return -1;
            else if (key > item)
                return 1;
            else
                return 0;
        }

        UNITTEST_FIXTURE_SETUP() {}
        UNITTEST_FIXTURE_TEARDOWN() {}

        const u32 c_max_nodes = 1022;

        UNITTEST_TEST(tree_node)
        {
            ntree32::tree_t tree;
            ntree32::tree_setup(&tree, c_max_nodes);
            s_init_keys();

            // ntree32::index_t const key = 0;

            ntree32::node_t node  = tree_new_node(&tree);
            ntree32::node_t left  = tree_new_node(&tree);
            ntree32::node_t right = tree_new_node(&tree);

            ntree32::node_t left_ptr = left;
            tree_set_node(&tree, node, ntree32::LEFT, left_ptr);

            CHECK_EQUAL(left_ptr, tree_get_node(&tree, node, ntree32::LEFT));

            ntree32::node_t right_ptr = right;
            tree_set_node(&tree, node, ntree32::RIGHT, right_ptr);
            CHECK_EQUAL(right_ptr, tree_get_node(&tree, node, ntree32::RIGHT));

            CHECK_EQUAL(true, tree_get_color(&tree, node) == ntree32::RED);

            tree_set_color(&tree, node, ntree32::BLACK);
            CHECK_EQUAL(false, tree_get_color(&tree, node) == ntree32::RED);
            tree_set_color(&tree, node, ntree32::RED);
            CHECK_EQUAL(true, tree_get_color(&tree, node) == ntree32::RED);
            tree_set_color(&tree, node, ntree32::BLACK);
            CHECK_EQUAL(false, tree_get_color(&tree, node) == ntree32::RED);
            CHECK_EQUAL(true, tree_get_color(&tree, node) == ntree32::BLACK);

            left_ptr  = left;
            right_ptr = right;
            tree_set_node(&tree, node, ntree32::LEFT, left_ptr);
            tree_set_node(&tree, node, ntree32::RIGHT, right_ptr);
            CHECK_EQUAL(left_ptr, tree_get_node(&tree, node, ntree32::LEFT));
            CHECK_EQUAL(right_ptr, tree_get_node(&tree, node, ntree32::RIGHT));
            CHECK_EQUAL(true, tree_get_color(&tree, node) == ntree32::BLACK);

            tree_set_node(&tree, node, ntree32::LEFT, left_ptr);
            tree_set_node(&tree, node, ntree32::RIGHT, right_ptr);

            CHECK_EQUAL(left_ptr, tree_get_node(&tree, node, ntree32::LEFT));
            CHECK_EQUAL(right_ptr, tree_get_node(&tree, node, ntree32::RIGHT));

            CHECK_EQUAL(true, tree_get_color(&tree, node) == ntree32::BLACK);

            tree_del_node(&tree, node);
            tree_del_node(&tree, left);
            tree_del_node(&tree, right);

            ntree32::tree_teardown(&tree);
        }

        UNITTEST_TEST(insert_remove)
        {
            ntree32::node_t root = ntree32::c_invalid_node;
            ntree32::tree_t tree;
            ntree32::tree_setup(&tree, c_max_nodes);
            s_init_keys();

            for (s32 i = 0; i < c_num_keys; ++i)
            {
                s_keys[c_find_slot] = s_find[i];
                ntree32::node_t inserted;
                CHECK_TRUE(ntree32::tree_insert(&tree, root, c_temp_slot, c_find_slot, s_compare_key_and_node, &s_keys, inserted));
                s_keys[inserted] = s_find[i];

                const char *result = nullptr;
                CHECK_TRUE(ntree32::tree_validate(&tree, root, result, s_compare_key_and_node, &s_keys));
                ntree32::node_t removed;
                CHECK_TRUE(ntree32::tree_remove(&tree, root, c_temp_slot, c_find_slot, s_compare_key_and_node, &s_keys, removed));
                CHECK_NOT_EQUAL(ntree32::c_invalid_node, removed);
                tree_del_node(&tree, removed);
            }

            ntree32::node_t node;
            while (!ntree32::tree_clear(&tree, root, node)) {}

            ntree32::tree_teardown(&tree);
        }

        UNITTEST_TEST(void_tree_iterate_preorder)
        {
            ntree32::node_t root = ntree32::c_invalid_node;
            ntree32::tree_t tree;
            ntree32::tree_setup(&tree, c_max_nodes);

            s_init_keys();

            for (s32 i = 0; i < 9; ++i)
            {
                s_keys[c_find_slot] = s_find[i];
                ntree32::node_t inserted;
                CHECK_TRUE(ntree32::tree_insert(&tree, root, c_temp_slot, c_find_slot, s_compare_key_and_node, &s_keys, inserted));
                s_keys[inserted] = s_find[i];
            }

#ifdef TEST_ITERATOR
            ntree32::iterator_t iterator;
            ntree32::tree_iterate(&iterator, &tree, root);

            s32             round      = 0;
            s32             preorder[] = {3, 1, 0, 2, 5, 4, 7, 6, 8};
            ntree32::node_t node;
            while (ntree32::iterator_preorder(&iterator, ntree32::LEFT, node))
            {
                CHECK_EQUAL(s_find[preorder[round++]], s_keys[node]);
            }
            CHECK_EQUAL(9, round);
#endif
            while (!ntree32::tree_clear(&tree, root, node)) {}

            ntree32::tree_teardown(&tree);
        }

        UNITTEST_TEST(void_tree_iterate_sortorder)
        {
            ntree32::node_t root = ntree32::c_invalid_node;
            ntree32::tree_t tree;
            ntree32::tree_setup(&tree, c_max_nodes);
            s_init_keys();

            for (s32 i = 0; i < c_num_keys; ++i)
            {
                s_keys[c_find_slot] = s_find[i];
                ntree32::node_t inserted;
                CHECK_TRUE(ntree32::tree_insert(&tree, root, c_temp_slot, c_find_slot, s_compare_key_and_node, &s_keys, inserted));
                s_keys[inserted] = s_find[i];
            }

#ifdef TEST_ITERATOR
            ntree32::iterator_t iterator;
            ntree32::tree_iterate(&iterator, &tree, root);

            s32             sortorder = 0;
            ntree32::node_t node;
            while (ntree32::iterator_sortorder(&iterator, ntree32::LEFT, node))
            {
                CHECK_EQUAL(s_find[sortorder++], s_keys[node]);
            }
            CHECK_EQUAL(c_num_keys, sortorder);
#endif

            while (!ntree32::tree_clear(&tree, root, node)) {}

            ntree32::tree_teardown(&tree);
        }

        UNITTEST_TEST(void_tree_iterate_sortorder_backwards)
        {
            ntree32::node_t root = ntree32::c_invalid_node;
            ntree32::tree_t tree;
            ntree32::tree_setup(&tree, c_max_nodes);
            s_init_keys();

            for (s32 i = 0; i < c_num_keys; ++i)
            {
                s_keys[c_find_slot] = s_find[i];
                ntree32::node_t inserted;
                CHECK_TRUE(ntree32::tree_insert(&tree, root, c_temp_slot, c_find_slot, s_compare_key_and_node, &s_keys, inserted));
                s_keys[inserted] = s_find[i];
            }

#ifdef TEST_ITERATOR
            ntree32::iterator_t iterator;
            ntree32::tree_iterate(&iterator, &tree, root);

            s32             sortorder = c_num_keys;
            ntree32::node_t node;
            while (ntree32::iterator_sortorder(&iterator, ntree32::RIGHT, node))
            {
                sortorder--;
                CHECK_EQUAL(s_find[sortorder], s_keys[node]);
            }
            CHECK_EQUAL(0, sortorder);
#endif

            while (!ntree32::tree_clear(&tree, root, node)) {}

            ntree32::tree_teardown(&tree);
        }

        UNITTEST_TEST(void_tree_iterate_postorder)
        {
            ntree32::node_t root = ntree32::c_invalid_node;
            ntree32::tree_t tree;
            ntree32::tree_setup(&tree, c_max_nodes);
            s_init_keys();

            for (s32 i = 0; i < 9; ++i)
            {
                s_keys[c_find_slot] = s_find[i];
                ntree32::node_t inserted;
                CHECK_TRUE(ntree32::tree_insert(&tree, root, c_temp_slot, c_find_slot, s_compare_key_and_node, &s_keys, inserted));
                s_keys[inserted] = s_find[i];
            }

#ifdef TEST_ITERATOR
            ntree32::iterator_t iterator;
            ntree32::tree_iterate(&iterator, &tree, root);

            s32       round       = 0;
            s32 const postorder[] = {0, 2, 1, 4, 6, 8, 7, 5, 3};

            ntree32::node_t node;
            while (ntree32::iterator_postorder(&iterator, ntree32::LEFT, node))
            {
                CHECK_EQUAL(s_find[postorder[round++]], s_keys[node]);
            }
            CHECK_EQUAL(9, round);
#endif

            while (!ntree32::tree_clear(&tree, root, node)) {}

            ntree32::tree_teardown(&tree);
        }

        UNITTEST_TEST(void_tree_search)
        {
            ntree32::node_t root = ntree32::c_invalid_node;
            ntree32::tree_t tree;
            ntree32::tree_setup(&tree, c_max_nodes);
            s_init_keys();

            for (s32 i = 0; i < c_num_keys; ++i)
            {
                s_keys[c_find_slot] = s_find[i];
                ntree32::node_t inserted;
                CHECK_TRUE(ntree32::tree_insert(&tree, root, c_temp_slot, c_find_slot, s_compare_key_and_node, &s_keys, inserted));
                s_keys[inserted] = s_find[i];
            }

            ntree32::iterator_t iterator;
            ntree32::tree_iterate(&iterator, &tree, root);

            s32             dir = ntree32::LEFT;
            ntree32::node_t node;
            s32             find = (c_num_keys / 2);  // find the key at s_keys[find]
            while (ntree32::iterator_traverse(&iterator, dir, node))
            {
                s8 const c = s_compare_key_and_node(find, node, &s_keys);
                if (c == 0)
                    break;
                dir = ntree32::tree_getdir(c);
            }
            CHECK_EQUAL(0, s_compare_key_and_node(find, node, &s_keys));

            while (!ntree32::tree_clear(&tree, root, node)) {}

            ntree32::tree_teardown(&tree);
        }

        UNITTEST_TEST(s32_tree)
        {
            ntree32::node_t root = ntree32::c_invalid_node;
            ntree32::tree_t tree;
            ntree32::tree_setup(&tree, c_max_nodes);

            s_init_keys();

            for (s32 i = 0; i < c_num_keys; ++i)
            {
                s_keys[c_find_slot] = s_find[i];
                ntree32::node_t inserted;
                CHECK_TRUE(ntree32::tree_insert(&tree, root, c_temp_slot, c_find_slot, s_compare_key_and_node, &s_keys, inserted));
                s_keys[inserted] = s_find[i];
            }

            for (s32 i = 0; i < c_num_keys; ++i)
            {
                ntree32::node_t node = ntree32::c_invalid_node;
                s_keys[c_find_slot]  = s_keys[i];
                CHECK_EQUAL(true, ntree32::tree_find(&tree, root, c_find_slot, s_compare_key_and_node, &s_keys, node));
                CHECK_NOT_EQUAL(ntree32::c_invalid_node, node);
                CHECK_EQUAL((ntree32::node_t)i, node);
            }

            ntree32::node_t node = ntree32::c_invalid_node;
            s_keys[c_find_slot]  = 11111;
            CHECK_FALSE(ntree32::tree_find(&tree, root, c_find_slot, s_compare_key_and_node, &s_keys, node));

            while (!ntree32::tree_clear(&tree, root, node)) {}

            ntree32::tree_teardown(&tree);
        }
    }
}
UNITTEST_SUITE_END
