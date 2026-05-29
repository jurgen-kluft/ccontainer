#include "ccore/c_target.h"
#include "ccore/c_arena.h"
#include "ccore/c_debug.h"
#include "ccore/c_memory.h"

#include "ccontainer/c_tree32.h"
#include "ccontainer/c_vector.h"
#include "ccontainer/c_map32.h"

namespace ncore
{
    static s8 s_compare(u32 _key, u32 _item, void const* user_data)
    {
        map32_t const* data = (map32_t const*)user_data;
        byte const*    key  = get_item_const_ptr(&data->m_keys, _key);
        byte const*    item = get_item_const_ptr(&data->m_keys, _item);
        return g_memcmp(key, item, data->m_keys.m_sizeof);
    }

    void map32_setup(map32_t* map, u32 capacity, u32 sizeof_key, u32 sizeof_value, map32_compare_t comparer)
    {
        map->m_root     = ntree32::c_invalid_node;
        map->m_comparer = comparer;
        vector_setup(&map->m_keys, sizeof_key, capacity, 0);
        vector_setup(&map->m_values, sizeof_value, capacity, 0);
        ntree32::tree_setup(&map->m_tree, capacity);
    }

    void map32_setup(map32_t* map, u32 capacity, u32 sizeof_key, u32 sizeof_value)
    {
        // default comparer is memcmp on the key
        map32_setup(map, capacity, sizeof_key, sizeof_value, s_compare);
    }

    void map32_teardown(map32_t* map)
    {
        ntree32::tree_teardown(&map->m_tree);
        vector_destroy(&map->m_keys);
        vector_destroy(&map->m_values);
    }

    bool map32_insert(map32_t* map, byte const* key, byte const* value)
    {
        // Ensure we have enough capacity for find and temp
        const u32 capacity = ntree32::tree_get_used_capacity(&map->m_tree);
        vector_ensure_capacity(&map->m_keys, capacity + 3);
        vector_ensure_capacity(&map->m_values, capacity + 3);
        ntree32::tree_ensure_capacity(&map->m_tree, capacity + 3);

        vector_set_size(&map->m_keys, capacity + 2);
        vector_set_size(&map->m_values, capacity + 2);

        ntree32::index_t find_slot = capacity;
        ntree32::node_t  temp_slot = capacity + 1;

        byte* keys = get_item_ptr(&map->m_keys, capacity);
        g_memcpy(keys, key, map->m_keys.m_sizeof);

        ntree32::node_t inserted;
        if (ntree32::tree_insert(&map->m_tree, map->m_root, temp_slot, find_slot, map->m_comparer, map, inserted))
        {
            byte* key_inserted   = get_item_ptr(&map->m_keys, inserted);
            byte* value_inserted = get_item_ptr(&map->m_values, inserted);
            g_memcpy(key_inserted, key, map->m_keys.m_sizeof);
            g_memcpy(value_inserted, value, map->m_values.m_sizeof);
            return true;
        }
        return false;
    }

    bool map32_remove(map32_t* map, byte const* key)
    {
        if (map->m_root == ntree32::c_invalid_node)
            return false;

        const u32 capacity = ntree32::tree_get_capacity(&map->m_tree);

        byte* keys = get_item_ptr(&map->m_keys, capacity);
        g_memcpy(keys, key, map->m_keys.m_sizeof);

        ntree32::index_t find_slot = capacity;
        ntree32::node_t  temp_slot = capacity + 1;

        ntree32::node_t removed;
        if (ntree32::tree_remove(&map->m_tree, map->m_root, temp_slot, find_slot, map->m_comparer, map, removed))
        {
            if (removed != ntree32::c_invalid_node)
            {
                ntree32::tree_del_node(&map->m_tree, removed);
                return true;
            }
        }

        return false;
    }

    bool map32_find(map32_t* map, byte const* key, byte* value)
    {
        if (map->m_root == ntree32::c_invalid_node)
            return false;

        const u32 capacity = ntree32::tree_get_capacity(&map->m_tree);

        byte* keys = get_item_ptr(&map->m_keys, capacity);
        g_memcpy(keys, key, map->m_keys.m_sizeof);

        ntree32::index_t find_slot = capacity;

        ntree32::node_t found;
        if (ntree32::tree_find(&map->m_tree, map->m_root, find_slot, map->m_comparer, map, found))
        {
            if (found != ntree32::c_invalid_node)
            {
                byte* value_found = get_item_ptr(&map->m_values, found);
                g_memcpy(value, value_found, map->m_values.m_sizeof);
                return true;
            }
        }
        return false;
    }

}  // namespace ncore
