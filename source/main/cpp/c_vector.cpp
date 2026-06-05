#include "ccore/c_target.h"
#include "ccore/c_arena.h"
#include "ccore/c_limits.h"
#include "ccore/c_memory.h"

#include "ccontainer/c_vector.h"

namespace ncore
{
    bool vector_setup(vector_t* vector, u32 item_size, u32 num_items_reserved, u32 num_items_committed)
    {
        ASSERT(item_size > 0 && item_size <= 65536);  // item size should be reasonable
        ASSERT(num_items_reserved > 0);               // must reserve at least one item

        vector->m_count    = 0;
        vector->m_sizeof   = item_size;
        vector->m_capacity = num_items_reserved;
        vector->m_arena    = narena::new_arena((int_t)item_size * num_items_reserved, (int_t)item_size * num_items_committed);
        return vector->m_arena != nullptr;
    }

    void vector_destroy(vector_t* vector)
    {
        if (vector->m_arena != nullptr)
            narena::destroy(vector->m_arena);
    }

    void vector_set_size(vector_t* vector, u32 new_size)
    {
        if (new_size > vector->m_capacity)
            return;  // new size is too large
        vector->m_count = new_size;
    }

    bool vector_ensure_capacity(vector_t* vector, u32 new_capacity)
    {
        if (new_capacity == 0 || new_capacity > narena::reserved_size(vector->m_arena) / vector->m_sizeof)
            return false;  // new capacity is too large

        vector->m_capacity = new_capacity;
        return narena::commit(vector->m_arena, (int_t)new_capacity * vector->m_sizeof);
    }

    void push_item(vector_t* vector, byte const* item)
    {
        byte* dst = (byte*)narena::alloc(vector->m_arena, vector->m_sizeof);
        g_memcpy(dst, item, vector->m_sizeof);
        vector->m_count += 1;
    }

    void insert_item(vector_t* vector, u32 index, byte const* item)
    {
        // insert does need to alloc a new slot
        narena::alloc(vector->m_arena, vector->m_sizeof);

        // if we are inserting at the end, just copy the item to the new slot
        if (index == vector->m_count)
        {
            byte* dst = (byte*)narena::base_ptr(vector->m_arena) + (index * vector->m_sizeof);
            g_memcpy(dst, item, vector->m_sizeof);
            vector->m_count += 1;
            return;
        }

        // if we are inserting in the middle, we need to move the existing items after the index
        // to make room for the new item
        byte* dst       = (byte*)narena::base_ptr(vector->m_arena) + (index * vector->m_sizeof);
        byte* src       = dst;
        u32   move_size = (vector->m_count - index) * vector->m_sizeof;
        g_memmove(dst + vector->m_sizeof, src, move_size);
        g_memcpy(dst, item, vector->m_sizeof);
        vector->m_count += 1;
    }

    bool set_item(vector_t* vector, u32 index, const byte* item)
    {
        if (index >= vector->m_count)
            return false;  // index out of bounds

        byte* dst = (byte*)narena::base_ptr(vector->m_arena) + (index * vector->m_sizeof);
        g_memcpy(dst, item, vector->m_sizeof);
        return true;
    }

    bool pop_item(vector_t* vector, byte* out_item)
    {
        if (vector->m_count == 0)
            return false;  // no items to pop

        vector->m_count -= 1;
        byte* dst = (byte*)narena::base_ptr(vector->m_arena) + (vector->m_count * vector->m_sizeof);
        g_memcpy(out_item, dst, vector->m_sizeof);
        return true;
    }

    void remove_item(vector_t* vector, u32 index)
    {
        if (index >= vector->m_count)
            return;  // index out of bounds

        // if we are removing the last item, just decrease the count
        if (index == vector->m_count - 1)
        {
            vector->m_count -= 1;
            return;
        }

        // if we are removing an item in the middle, we need to move the existing items after the index
        // to fill the gap left by the removed item
        byte* dst       = (byte*)narena::base_ptr(vector->m_arena) + (index * vector->m_sizeof);
        byte* src       = dst + vector->m_sizeof;
        u32   move_size = (vector->m_count - index - 1) * vector->m_sizeof;
        g_memmove(dst, src, move_size);
        vector->m_count -= 1;
    }

    void remove_item_swap(vector_t* vector, u32 index)
    {
        if (index >= vector->m_count)
            return;  // index out of bounds

        // if we are removing the last item, just decrease the count
        if (index == vector->m_count - 1)
        {
            vector->m_count -= 1;
            return;
        }

        // if we are removing an item in the middle, we can swap it with the last item and then decrease the count
        byte* dst = (byte*)narena::base_ptr(vector->m_arena) + (index * vector->m_sizeof);
        byte* src = (byte*)narena::base_ptr(vector->m_arena) + ((vector->m_count - 1) * vector->m_sizeof);
        g_memcpy(dst, src, vector->m_sizeof);
        vector->m_count -= 1;
    }

    byte* get_item_ptr(vector_t* vector, u32 index)
    {
        if (index >= vector->m_count)
            return nullptr;
        return (byte*)narena::base_ptr(vector->m_arena) + (index * vector->m_sizeof);
    }

    byte const* get_item_ptr(vector_t const* vector, u32 index)
    {
        if (index >= vector->m_count)
            return nullptr;
        return (byte const*)narena::base_ptr(vector->m_arena) + (index * vector->m_sizeof);
    }

    byte* get_items_ptr(vector_t* vector)
    {
        if (vector->m_count == 0)
            return nullptr;
        return (byte*)narena::base_ptr(vector->m_arena);
    }

    byte const* get_items_ptr(vector_t const* vector)
    {
        if (vector->m_count == 0)
            return nullptr;
        return (byte const*)narena::base_ptr(vector->m_arena);
    }

    s32 compare_items(vector_t const* vector, u32 lhs_index, u32 rhs_index)
    {
        if (lhs_index >= vector->m_count || rhs_index >= vector->m_count)
            return 0;
        byte const* lhs = (byte const*)narena::base_ptr(vector->m_arena) + (lhs_index * vector->m_sizeof);
        byte const* rhs = (byte const*)narena::base_ptr(vector->m_arena) + (rhs_index * vector->m_sizeof);
        return g_memcmp(lhs, rhs, vector->m_sizeof);
    }

};  // namespace ncore
