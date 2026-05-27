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
        s32      m_count;
        s32      m_sizeof;
        arena_t* m_arena;
    };

    bool init_vector(vector_t& vector, s32 item_size, s32 num_items_reserved, s32 num_items_committed = 0);
    void destroy_vector(vector_t& vector);
    bool set_capacity(vector_t& vector, s32 new_capacity);
    s32  get_reserved(vector_t& vector, s32 item_size);

    bool push_item(vector_t& vector, byte* item);
    bool insert_item(vector_t& vector, s32 index, byte* item);

    bool set_item(vector_t& vector, u32 index, const byte* item);
    bool pop_item(vector_t& vector, byte* out_item);
    void remove_item(vector_t& vector, u32 index);
    void swap_remove(vector_t& vector, u32 index);

    byte*       get_item_ptr(vector_t& vector, u32 index);
    byte const* get_item_const_ptr(vector_t& vector, u32 index);
    byte*       get_items_ptr(vector_t& vector);
    byte const* get_items_const_ptr(vector_t& vector);

    s32 compare_items(vector_t& vector, u32 lhs_index, u32 rhs_index);

};  // namespace ncore

#endif  // __CCONTAINER_VECTOR_H__
