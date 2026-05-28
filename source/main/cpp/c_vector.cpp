#include "ccore/c_target.h"
#include "ccore/c_arena.h"
#include "ccore/c_limits.h"
#include "ccore/c_memory.h"

#include "ccontainer/c_vector.h"

namespace ncore
{
    bool init_vector(vector_t* vector, u32 item_size, u32 num_items_reserved, u32 num_items_committed)
    {
        ASSERT(item_size > 0 && item_size <= 65536);  // item size should be reasonable
        ASSERT(num_items_reserved > 0);               // must reserve at least one item

        vector->m_count  = 0;
        vector->m_sizeof = item_size;
        vector->m_arena  = narena::new_arena((int_t)item_size * num_items_reserved, (int_t)item_size * num_items_committed);
        return vector->m_arena != nullptr;
    }

    void destroy_vector(vector_t* vector)
    {
        if (vector->m_arena != nullptr)
            narena::destroy(vector->m_arena);
    }

    bool vector_set_capacity(vector_t* vector, u32 new_capacity)
    {
        if (new_capacity == 0 || new_capacity > (D_U32_MAX / vector->m_sizeof))
            return false;  // new capacity is too large

        vector->m_capacity = new_capacity;
        return narena::commit(vector->m_arena, (int_t)new_capacity * vector->m_sizeof);
    }

    u32 vector_get_capacity(vector_t* vector) { return vector->m_capacity; }

    bool push_item(vector_t* vector, byte* item)
    {
        byte* dst = (byte*)narena::alloc(vector->m_arena, vector->m_sizeof);
        g_memcpy(dst, item, vector->m_sizeof);
        vector->m_count += 1;
        return true;
    }

    bool insert_item(vector_t* vector, u32 index, byte* item)
    {
        // insert does need to alloc a new slot
        narena::alloc(vector->m_arena, vector->m_sizeof);

        // if we are inserting at the end, just copy the item to the new slot
        if (index == vector->m_count)
        {
            byte* dst = (byte*)narena::base_ptr(vector->m_arena) + (index * vector->m_sizeof);
            g_memcpy(dst, item, vector->m_sizeof);
            vector->m_count += 1;
            return true;
        }

        // if we are inserting in the middle, we need to move the existing items after the index
        // to make room for the new item
        byte* dst       = (byte*)narena::base_ptr(vector->m_arena) + (index * vector->m_sizeof);
        byte* src       = dst;
        u32   move_size = (vector->m_count - index) * vector->m_sizeof;
        g_memmove(dst + vector->m_sizeof, src, move_size);
        g_memcpy(dst, item, vector->m_sizeof);
        vector->m_count += 1;
        return true;
    }

};  // namespace ncore
