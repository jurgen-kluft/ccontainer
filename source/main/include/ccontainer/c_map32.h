#ifndef __CCONTAINER_MAP32_H__
#define __CCONTAINER_MAP32_H__
#include "ccore/c_target.h"
#ifdef USE_PRAGMA_ONCE
#    pragma once
#endif

#include "ccontainer/c_tree32.h"
#include "ccontainer/c_vector.h"

namespace ncore
{
    // -----------------------------------------------------------------------------------
    // -----------------------------------------------------------------------------------
    // Note of caution:
    // The map and set provided here assume that a key and value are very simple POD types.
    // There is no handling for 'construction' or 'destruction', furthermore there is also
    // NO hashing done on the key.
    // The index of a node that is managed by the tree is the same index at which you can
    // find the key/value, so here there is no indirection between the index at which you
    // can find the key/value and the node index.
    // -----------------------------------------------------------------------------------

    typedef s8 (*map32_compare_t)(u32 _key, u32 _item, void const* user_data);

    struct map32_t
    {
        ntree32::tree_t m_tree;
        ntree32::node_t m_root;
        map32_compare_t m_comparer;
        vector_t        m_keys;
        vector_t        m_values;
    };

    void map32_setup(map32_t* c, u32 capacity, u32 sizeof_key, u32 sizeof_value);
    void map32_setup(map32_t* c, u32 capacity, u32 sizeof_key, u32 sizeof_value, map32_compare_t comparer);
    void map32_teardown(map32_t* c);
    bool map32_insert(map32_t* c, byte const* key, byte const* value);
    bool map32_remove(map32_t* c, byte const* key);
    bool map32_find(map32_t* c, byte const* key, byte* value);

    template <typename K, typename V>
    void map32_setup(map32_t* c, u32 capacity)
    { map32_setup(c, capacity, sizeof(K), sizeof(V)); }

    template <typename K, typename V>
    bool map32_insert(map32_t* c, K const& key, V const& value)
    {
        ASSERT(sizeof(K) == c->m_keys.m_sizeof);
        ASSERT(sizeof(V) == c->m_values.m_sizeof);
        return map32_insert(c, (byte const*)&key, (byte const*)&value);
    }

    template <typename K>
    bool map32_remove(map32_t* c, K const& key)
    {
        ASSERT(sizeof(K) == c->m_keys.m_sizeof);
        return map32_remove(c, (byte const*)&key);
    }

    template <typename K, typename V>
    bool map32_find(map32_t* c, K const& key, V& value)
    {
        ASSERT(sizeof(K) == c->m_keys.m_sizeof);
        ASSERT(sizeof(V) == c->m_values.m_sizeof);
        return map32_find(c, (byte const*)&key, (byte*)&value);
    }

};  // namespace ncore

#endif  // __CCONTAINER_MAP_H__
