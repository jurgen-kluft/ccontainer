#ifndef __CCONTAINER_VECTOR_H__
#define __CCONTAINER_VECTOR_H__
#include "ccore/c_target.h"
#ifdef USE_PRAGMA_ONCE
#    pragma once
#endif

namespace ncore
{
    struct arena_t;

    struct vector_t
    {
        u32      m_count;
        u32      m_sizeof;
        u32      m_capacity;
        arena_t* m_arena;
    };

    bool vector_setup(vector_t* vector, u32 item_size, u32 num_items_reserved, u32 num_items_committed = 0);
    void vector_destroy(vector_t* vector);

    inline u32 vector_get_size(vector_t* vector) { return vector->m_count; }
    void       vector_set_size(vector_t* vector, u32 new_size);

    bool       vector_ensure_capacity(vector_t* vector, u32 new_capacity);  // unit = number of items
    inline u32 vector_get_capacity(vector_t* vector) { return vector->m_capacity; }

    bool vector_is_empty(vector_t* vector) { return vector->m_count == 0; }
    void vector_clear(vector_t* vector) { vector->m_count = 0; }

    void vector_push(vector_t* vector, byte const* item);
    void vector_insert(vector_t* vector, u32 index, byte const* item);

    bool vector_set(vector_t* vector, u32 index, const byte* item);
    bool vector_pop(vector_t* vector, byte* out_item);
    void vector_remove(vector_t* vector, u32 index);
    void vector_remove_swap(vector_t* vector, u32 index);

    byte*       vector_begin_ptr(vector_t* vector);
    byte*       vector_end_ptr(vector_t* vector);
    byte const* vector_begin_ptr(vector_t const* vector);
    byte const* vector_end_ptr(vector_t const* vector);

    byte*       vector_item_ptr(vector_t* vector, u32 index);
    byte const* vector_item_ptr(vector_t const* vector, u32 index);
    byte*       vector_items_ptr(vector_t* vector);
    byte const* vector_items_ptr(vector_t const* vector);

    s32 vector_compare_items(vector_t const* vector, u32 lhs_index, u32 rhs_index);

};  // namespace ncore

#endif  // __CCONTAINER_VECTOR_H__
