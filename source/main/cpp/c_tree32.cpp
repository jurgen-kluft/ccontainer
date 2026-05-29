#include "ccore/c_target.h"
#include "ccore/c_arena.h"
#include "ccore/c_debug.h"

#include "ccontainer/c_tree32.h"

namespace ncore
{
    namespace ntree32
    {
        void tree_reset(tree_t* tree)
        {
            if (tree->m_nodes != nullptr)
                narena::reset(tree->m_nodes);
            tree->m_free_head = c_invalid_node;
            tree->m_count     = 0;
        }

        void tree_set_color(tree_t* tree, node_t node, u8 color)
        {
            ASSERT(node != c_invalid_index);
            nnode_t* nodes            = narena::base_ptr_as<nnode_t>(tree->m_nodes);
            nodes[node].m_child[LEFT] = (nodes[node].m_child[LEFT] & 0x7FFFFFFF) | (color << 31);
        }

        u8 tree_get_color(tree_t const* tree, node_t const node)
        {
            ASSERT(node != c_invalid_index);
            nnode_t* nodes = narena::base_ptr_as<nnode_t>(tree->m_nodes);
            return (nodes[node].m_child[LEFT] & 0x80000000) >> 31;
        }

        node_t tree_get_node(tree_t const* tree, node_t const node, s8 ne)
        {
            ASSERT(node != c_invalid_index);
            // Since we are using the highest bit to store the color of the node, we need to mask it out
            // but that also means that 0x7FFFFFFF is the highest value we can store in a node.
            // To return 0xFFFFFFFF when the node is 0x7FFFFFFF we can do the following bitwise operation:
            // - ((node + 1) & 0x80000000) | (node & 0x7FFFFFFF)
            nnode_t* nodes = narena::base_ptr_as<nnode_t>(tree->m_nodes);
            node_t   n     = nodes[node].m_child[ne] & 0x7FFFFFFF;
            return ((n + 1) & 0x80000000) | n;
        }

        void tree_set_node(tree_t* tree, node_t node, s8 ne, node_t set)
        {
            ASSERT(node != c_invalid_index);
            nnode_t* nodes          = narena::base_ptr_as<nnode_t>(tree->m_nodes);
            nodes[node].m_child[ne] = (nodes[node].m_child[ne] & 0x80000000) | set;
        }

        node_t tree_new_node(tree_t* tree)
        {
            nnode_t* nodes = narena::base_ptr_as<nnode_t>(tree->m_nodes);

            node_t node = tree->m_free_head;
            if (node != c_invalid_node)
            {
                tree->m_free_head = nodes[node].m_child[LEFT];
            }
            else
            {
                nnode_t* new_node = g_allocate<nnode_t>(tree->m_nodes);
                node              = (node_t)(new_node - nodes);
            }

            tree->m_count++;

            nodes[node].m_child[0] = c_invalid_node;
            nodes[node].m_child[1] = c_invalid_node;
            tree_set_color(tree, node, RED);
            return node;
        }

        void tree_del_node(tree_t* tree, node_t node)
        {
            ASSERT(node != c_invalid_index);
            nnode_t* nodes             = narena::base_ptr_as<nnode_t>(tree->m_nodes);
            nodes[node].m_child[LEFT]  = tree->m_free_head;
            nodes[node].m_child[RIGHT] = c_invalid_node;
            tree->m_free_head          = node;
            tree->m_count--;
        }

        static inline node_t rotate_single(tree_t* tree, node_t node, s32 dir)
        {
            node_t save = tree_get_node(tree, node, 1 - dir);
            tree_set_node(tree, node, 1 - dir, tree_get_node(tree, save, dir));
            tree_set_node(tree, save, dir, node);
            tree_set_color(tree, node, RED);
            tree_set_color(tree, save, BLACK);
            return save;
        }

        static inline node_t rotate_single_track_parent(tree_t* tree, node_t node, s32 dir, node_t fn, node_t& fp)
        {
            node_t save = tree_get_node(tree, node, 1 - dir);
            tree_set_node(tree, node, 1 - dir, tree_get_node(tree, save, dir));
            tree_set_node(tree, save, dir, node);
            tree_set_color(tree, node, RED);
            tree_set_color(tree, save, BLACK);

            if (fn == node)
                fp = save;
            else if (fn == tree_get_node(tree, node, 1 - dir))
                fp = node;

            return save;
        }

        static inline node_t rotate_double(tree_t* tree, node_t node, s32 dir)
        {
            tree_set_node(tree, node, 1 - dir, rotate_single(tree, tree_get_node(tree, node, 1 - dir), 1 - dir));
            return rotate_single(tree, node, dir);
        }

        static inline node_t rotate_double_track_parent(tree_t* tree, node_t node, s32 dir, node_t fn, node_t& fp)
        {
            node_t child = rotate_single_track_parent(tree, tree_get_node(tree, node, 1 - dir), 1 - dir, fn, fp);
            tree_set_node(tree, node, 1 - dir, child);

            if (fn == child)  // never triggered
                fp = node;
            node_t save = rotate_single_track_parent(tree, node, dir, fn, fp);
            if (fn == node)
                fp = save;
            return save;
        }

        static inline bool is_red(tree_t const* tree, node_t n) { return n != c_invalid_node && tree_get_color(tree, n) == RED; }

        bool tree_insert(tree_t* tree, node_t& root, node_t temp, index_t key, compare_fn comparer, void const* user_data, node_t& inserted_or_found)
        {
            node_t inserted = c_invalid_node;
            node_t found    = c_invalid_node;
            if (root == c_invalid_node)
            {
                // We have an empty tree; attach the
                // new node directly to the root
                node_t new_node = tree_new_node(tree);
                root            = new_node;
                inserted        = new_node;
            }
            else
            {
                node_t head = temp;  // False tree root
                node_t g, t;         // Grandparent & parent
                node_t p, n;         // Iterator & parent
                s8     dir = 0, last = 0;

                // Set up our helpers
                t = head;
                tree_set_color(tree, t, BLACK);
                tree_set_node(tree, t, RIGHT, root);
                tree_set_node(tree, t, LEFT, c_invalid_node);

                g = p = c_invalid_node;
                n     = root;

                // Search down the tree for a place to insert
                for (;;)
                {
                    if (n == c_invalid_node)
                    {
                        // Insert a new node at the first null link
                        n = tree_new_node(tree);
                        tree_set_node(tree, p, dir, n);

                        if (is_red(tree, n) && is_red(tree, p))
                        {
                            // Hard red violation: rotations necessary
                            const s32 dir2 = (tree_get_node(tree, t, RIGHT) == g) ? 1 : 0;

                            if (n == tree_get_node(tree, p, last))
                                tree_set_node(tree, t, dir2, rotate_single(tree, g, 1 - last));
                            else
                                tree_set_node(tree, t, dir2, rotate_double(tree, g, 1 - last));
                        }

                        inserted = n;
                        break;
                    }
                    else if (is_red(tree, tree_get_node(tree, n, LEFT)) && is_red(tree, tree_get_node(tree, n, RIGHT)))
                    {
                        // Simple red violation: color flip
                        tree_set_color(tree, n, RED);
                        tree_set_color(tree, tree_get_node(tree, n, LEFT), BLACK);
                        tree_set_color(tree, tree_get_node(tree, n, RIGHT), BLACK);
                    }

                    if (is_red(tree, n) && is_red(tree, p))
                    {
                        // Hard red violation: rotations necessary
                        const s32 dir2 = (tree_get_node(tree, t, RIGHT) == g) ? 1 : 0;

                        if (n == tree_get_node(tree, p, last))
                            tree_set_node(tree, t, dir2, rotate_single(tree, g, 1 - last));
                        else
                            tree_set_node(tree, t, dir2, rotate_double(tree, g, 1 - last));
                    }

                    // Stop working if we inserted a node. This
                    // check also disallows duplicates in the tree
                    last = dir;
                    dir  = comparer(key, n, user_data);
                    if (dir == 0)
                    {
                        found = n;
                        break;
                    }
                    dir = ((dir + 1) >> 1);

                    // Move the helpers down
                    if (g != c_invalid_node)
                        t = g;

                    g = p;
                    p = n;
                    n = tree_get_node(tree, n, dir);
                }

                // Update the root (it may be different)
                // root = head->get_right(tree);
                root = tree_get_node(tree, head, RIGHT);
            }

            // Make the root black for simplified logic
            tree_set_color(tree, root, BLACK);

            inserted_or_found = (inserted == c_invalid_node) ? found : inserted;
            return inserted != c_invalid_node;
        }

        bool rb_clear(tree_t* tree, node_t& root, node_t& removed_node)
        {
            removed_node = c_invalid_node;

            node_t node = root;
            if (node == c_invalid_node)
                return true;

            node_t todelete = node;

            if (tree_get_node(tree, node, LEFT) == c_invalid_node)
            {
                root = tree_get_node(tree, node, RIGHT);
            }
            else if (tree_get_node(tree, node, RIGHT) == c_invalid_node)
            {
                root = tree_get_node(tree, node, LEFT);
            }
            else
            {
                // We have left and right branches
                // Take right branch and place it
                // somewhere down the left branch
                node_t branch = tree_get_node(tree, node, RIGHT);
                tree_set_node(tree, node, RIGHT, c_invalid_node);

                // Find a node in the left branch that does not
                // have both a left and right branch and place
                // our branch there.
                node_t iter = tree_get_node(tree, node, LEFT);
                while (tree_get_node(tree, iter, LEFT) != c_invalid_node && tree_get_node(tree, iter, RIGHT) != c_invalid_node)
                {
                    iter = tree_get_node(tree, iter, LEFT);
                }
                if (tree_get_node(tree, iter, LEFT) == c_invalid_node)
                {
                    tree_set_node(tree, iter, LEFT, branch);
                }
                else if (tree_get_node(tree, iter, RIGHT) == c_invalid_node)
                {
                    tree_set_node(tree, iter, RIGHT, branch);
                }

                root = tree_get_node(tree, node, LEFT);
            }

            removed_node = todelete;
            return false;
        }

        bool tree_clear(tree_t* tree, node_t& root, node_t& n)
        {
            node_t node   = c_invalid_node;
            bool   result = rb_clear(tree, root, node);
            if (node != c_invalid_node)
            {
                n = node;
            }
            return result;
        }

        bool tree_find(tree_t const* tree, node_t root, index_t key, compare_fn comparer, void const* user_data, node_t& found)
        {
            node_t node = root;
            while (node != c_invalid_node)
            {
                const s8 c = comparer(key, node, user_data);
                if (c == 0)
                {
                    found = node;
                    return true;
                }
                node = tree_get_node(tree, node, (c + 1) >> 1);
            }
            found = c_invalid_node;
            return false;
        }

        // validate the tree (return violation description in 'result'), also returns black height
        static s32 rb_validate(tree_t const* tree, node_t root, const char*& result, compare_fn comparer, void const* user_data)
        {
            if (root == c_invalid_node)
            {
                return 1;
            }
            else
            {
                node_t ln = tree_get_node(tree, root, LEFT);
                node_t rn = tree_get_node(tree, root, RIGHT);

                // Consecutive red links
                if (is_red(tree, root))
                {
                    if (is_red(tree, ln) || is_red(tree, rn))
                    {
                        result = "Red violation";
                        return 0;
                    }
                }

                s8 lh = rb_validate(tree, ln, result, comparer, user_data);
                s8 rh = rb_validate(tree, rn, result, comparer, user_data);

                // Invalid binary search tree
                if ((ln != c_invalid_node && comparer(ln, root, user_data) >= 0) || (rn != c_invalid_node && comparer(rn, root, user_data) <= 0))
                {
                    result = "Binary tree violation";
                    return 0;
                }

                if (lh != 0 && rh != 0 && lh != rh)  // Black height mismatch
                {
                    result = "Black violation";
                    return 0;
                }

                if (lh != 0 && rh != 0)  // Only count black links
                {
                    return is_red(tree, root) ? lh : lh + 1;
                }
            }
            return 0;
        }

        bool tree_remove(tree_t* tree, node_t& root, node_t temp, index_t key, compare_fn comparer, void const* user_data, node_t& out_removed)
        {
            if (root == c_invalid_node)
                return false;

            node_t head = temp;            // False tree root
            node_t fn   = c_invalid_node;  // Found node
            node_t fp   = c_invalid_node;  // Found node parent
            s8     dir  = 1;

            // Set up our helpers
            node_t g = c_invalid_node;
            node_t p = c_invalid_node;

            node_t n = head;
            tree_set_color(tree, n, BLACK);  // Color it black
            tree_set_node(tree, n, RIGHT, root);
            tree_set_node(tree, n, LEFT, c_invalid_node);

            // Search and push a red node down
            // to fix red violations as we go
            while (tree_get_node(tree, n, dir) != c_invalid_node)
            {
                const s8 last = dir;

                // Move the helpers down
                g   = p;
                p   = n;
                n   = tree_get_node(tree, n, dir);
                dir = comparer(key, n, user_data);

                // Save the node with matching data and keep
                // going; we'll do removal tasks at the end
                if (dir == 0)
                {
                    fn = n;
                    fp = p;
                }
                dir = ((dir + 1) >> 1);

                /* Push the red node down with rotations and color flips */
                if (!is_red(tree, n) && !is_red(tree, tree_get_node(tree, n, dir)))
                {
                    if (is_red(tree, tree_get_node(tree, n, 1 - dir)))
                    {
                        node_t r = rotate_single_track_parent(tree, n, dir, fn, fp);
                        tree_set_node(tree, p, last, r);
                        if (fn == r)  // never triggered
                            fp = p;
                        p = r;
                    }
                    else if (!is_red(tree, tree_get_node(tree, n, 1 - dir)))
                    {
                        node_t s = tree_get_node(tree, p, 1 - last);
                        if (s != c_invalid_node)
                        {
                            if (!is_red(tree, tree_get_node(tree, s, 1 - last)) && !is_red(tree, tree_get_node(tree, s, last)))
                            {
                                // Color flip
                                tree_set_color(tree, p, BLACK);
                                tree_set_color(tree, s, RED);
                                tree_set_color(tree, n, RED);
                            }
                            else
                            {
                                // const s32 dir2 = g->get_right(tree) == p ? 1 : 0;
                                const s32 dir2 = tree_get_node(tree, g, RIGHT) == p ? 1 : 0;
                                if (is_red(tree, tree_get_node(tree, s, last)))
                                {
                                    node_t r = rotate_double_track_parent(tree, p, last, fn, fp);
                                    // g->set_child(tree, dir2, r);
                                    tree_set_node(tree, g, dir2, r);
                                    if (fn == r)  // never triggered
                                        fp = g;
                                }
                                else if (is_red(tree, tree_get_node(tree, s, 1 - last)))
                                {
                                    node_t r = rotate_single_track_parent(tree, p, last, fn, fp);
                                    // g->set_child(tree, dir2, r);
                                    tree_set_node(tree, g, dir2, r);
                                    if (fn == r)  // never triggered
                                        fp = g;
                                }

                                // Ensure correct coloring
                                tree_set_color(tree, n, RED);
                                tree_set_color(tree, tree_get_node(tree, g, dir2), RED);

                                tree_set_color(tree, tree_get_node(tree, tree_get_node(tree, g, dir2), LEFT), BLACK);
                                tree_set_color(tree, tree_get_node(tree, tree_get_node(tree, g, dir2), RIGHT), BLACK);
                            }
                        }
                    }
                }
            }

            // Update the root (it may be different)
            root = tree_get_node(tree, head, RIGHT);

            // Replace and remove the saved node
            if (fn != c_invalid_node)
            {
                ASSERT(tree_get_node(tree, fp, RIGHT) == fn || tree_get_node(tree, fp, LEFT) == fn);
                ASSERT(tree_get_node(tree, p, RIGHT) == n || tree_get_node(tree, p, LEFT) == n);

                node_t child1 = tree_get_node(tree, n, tree_get_node(tree, n, LEFT) == c_invalid_node);
                tree_set_node(tree, p, tree_get_node(tree, p, RIGHT) == n, child1);

                if (fn != n)
                {
                    ASSERT(fp != p);

                    // swap 'n' and 'fn', we want to remove the node that was holding 'item'
                    tree_set_node(tree, fp, tree_get_node(tree, fp, RIGHT) == fn, n);
                    tree_set_node(tree, n, LEFT, tree_get_node(tree, fn, LEFT));
                    tree_set_node(tree, n, RIGHT, tree_get_node(tree, fn, RIGHT));
                    tree_set_color(tree, n, tree_get_color(tree, fn));
                    if (root == fn)
                        root = n;
                }
                else
                {
                    if (root == fn)
                        root = c_invalid_node;
                }

                // tree->v_del_node(fn); // User must delete the node
                out_removed = fn;
            }

            // Make the root black for simplified logic
            if (root != c_invalid_node)
                tree_set_color(tree, root, BLACK);

            return true;
        }

        bool tree_validate(tree_t const* tree, node_t root, const char*& error_str, compare_fn comparer, void const* user_data)
        {
            rb_validate(tree, root, error_str, comparer, user_data);
            return error_str == nullptr;
        }

        void tree_iterate(iterator_t* iter, tree_t* tree, node_t root)
        {
            iterator_setup(iter, tree, root);
            iter->m_it    = c_invalid_node;
            iter->m_stack = -1;
        }

        void iterator_setup(iterator_t* iter, tree_t* tree, node_t root)
        {
            iter->m_tree  = tree;
            iter->m_root  = root;
            iter->m_it    = c_invalid_node;
            iter->m_stack = -1;
        }

        bool iterator_traverse(iterator_t* iter, s32 d, node_t& out_node)
        {
            if (iter->m_it == c_invalid_node)
            {
                iter->m_it = iter->m_root;
            }
            else
            {
                iter->m_it = tree_get_node(iter->m_tree, iter->m_it, d);
            }

            if (iter->m_it != c_invalid_node)
            {
                out_node = iter->m_it;
                return true;
            }
            return false;
        }

        bool iterator_preorder(iterator_t* iter, s32 dir, node_t& out_node)
        {
            if (iter->m_stack == -1)
            {
                iter->m_stack = 0;
                if (iter->m_root != c_invalid_node)
                {
                    iter->m_stack_array[iter->m_stack++] = iter->m_root;
                }
            }

            if (iter->m_stack == 0)
            {
                out_node = c_invalid_node;
                return false;
            }

            iter->m_it = iter->m_stack_array[--iter->m_stack];

            node_t child1 = tree_get_node(iter->m_tree, iter->m_it, 1 - dir);
            if (child1 != c_invalid_node)
                iter->m_stack_array[iter->m_stack++] = child1;

            node_t child2 = tree_get_node(iter->m_tree, iter->m_it, dir);
            if (child2 != c_invalid_node)
                iter->m_stack_array[iter->m_stack++] = child2;

            out_node = iter->m_it;
            return true;
        }

        bool iterator_sortorder(iterator_t* iter, s32 dir, node_t& out_node)
        {
            if (iter->m_stack == -1)
            {
                iter->m_stack = 0;
                iter->m_it    = iter->m_root;
            }

            while (iter->m_it != c_invalid_node)
            {
                iter->m_stack_array[iter->m_stack++] = iter->m_it;
                iter->m_it                           = tree_get_node(iter->m_tree, iter->m_it, dir);
            }

            if (iter->m_stack == 0)
                return false;

            iter->m_it = iter->m_stack_array[--iter->m_stack];
            out_node   = iter->m_it;
            iter->m_it = tree_get_node(iter->m_tree, iter->m_it, 1 - dir);
            return true;
        }

        bool iterator_postorder(iterator_t* iter, s32 dir, node_t& out_node)
        {
            if (iter->m_stack == -1)
            {
                iter->m_stack = 0;
                iter->m_it    = iter->m_root;
                if (iter->m_it != c_invalid_node)
                {
                    iter->m_stack_array[iter->m_stack++] = iter->m_it;
                }
            }

            if (iter->m_stack == 0)
                return false;

            while (true)
            {
                node_t const node   = iter->m_stack_array[iter->m_stack - 1];
                node_t const child1 = tree_get_node(iter->m_tree, node, 1 - dir);
                node_t const child2 = tree_get_node(iter->m_tree, node, dir);
                if ((child1 == iter->m_it || child2 == iter->m_it) || (child1 == c_invalid_node && child2 == c_invalid_node))
                {
                    iter->m_it = node;
                    out_node   = iter->m_it;
                    iter->m_stack--;
                    return true;
                }
                else
                {
                    if (child1 != c_invalid_node)
                        iter->m_stack_array[iter->m_stack++] = child1;
                    if (child2 != c_invalid_node)
                        iter->m_stack_array[iter->m_stack++] = child2;
                }
            }
            return true;
        }

        void tree_setup(tree_t* tree, u32 max_nodes)
        {
            tree->m_count     = 0;
            tree->m_free_head = c_invalid_index;
            tree->m_nodes     = narena::new_arena(max_nodes * sizeof(nnode_t), 0);
        }

        void tree_teardown(tree_t* tree)
        {
            tree->m_count     = 0;
            tree->m_free_head = c_invalid_index;
            narena::destroy(tree->m_nodes);
        }

        u32 tree_get_used_capacity(tree_t const* tree)
        {
            return narena::current_pos(tree->m_nodes) / sizeof(nnode_t);
        }

        u32 tree_get_capacity(tree_t const* tree) { return narena::committed_size(tree->m_nodes) / sizeof(nnode_t); }

        void tree_ensure_capacity(tree_t* tree, u32 capacity)
        {
            if ((capacity * sizeof(nnode_t)) > narena::committed_size(tree->m_nodes))
            {
                narena::commit(tree->m_nodes, capacity * sizeof(nnode_t));
            }
        }

    }  // namespace ntree32
}  // namespace ncore
