#include "ccontainer/c_vector.h"

#include "cunittest/cunittest.h"

using namespace ncore;

UNITTEST_SUITE_BEGIN(vector_t)
{
    UNITTEST_FIXTURE(main)
    {
        UNITTEST_FIXTURE_SETUP() {}
        UNITTEST_FIXTURE_TEARDOWN() {}

        //UNITTEST_ALLOCATOR;

		UNITTEST_TEST(create_destroy)
		{
			vector_t v;
			CHECK_TRUE(vector_setup(&v, sizeof(u32), 128, 32));
			CHECK_EQUAL((u32)0, vector_get_size(&v));
			CHECK_EQUAL((u32)128, vector_get_capacity(&v));
			CHECK_EQUAL((byte*)nullptr, vector_items_ptr(&v));
			CHECK_EQUAL((byte const*)nullptr, vector_items_ptr((vector_t const*)&v));
			CHECK_EQUAL((byte*)nullptr, vector_item_ptr(&v, 0));
			CHECK_EQUAL((byte const*)nullptr, vector_item_ptr((vector_t const*)&v, 0));
			vector_destroy(&v);
		}

		UNITTEST_TEST(item_access_and_mutation)
		{
			vector_t vector;
			CHECK_TRUE(vector_setup(&vector, sizeof(u32), 8, 8));

			u32 values[] = {10, 20, 30};
			for (u32 i = 0; i < 3; ++i)
				vector_push(&vector, (byte*)&values[i]);

			CHECK_EQUAL((u32)3, vector_get_size(&vector));
			CHECK_NOT_EQUAL((byte*)nullptr, vector_items_ptr(&vector));
			CHECK_NOT_EQUAL((byte const*)nullptr, vector_items_ptr((vector_t const*)&vector));
			CHECK_EQUAL(values[0], *(u32*)vector_item_ptr(&vector, 0));
			CHECK_EQUAL(values[1], *(u32 const*)vector_item_ptr((vector_t const*)&vector, 1));
			CHECK_EQUAL((byte*)nullptr, vector_item_ptr(&vector, 3));

			u32 updated = 40;
			CHECK_TRUE(vector_set(&vector, 1, (byte const*)&updated));
			CHECK_FALSE(vector_set(&vector, 4, (byte const*)&updated));
			CHECK_EQUAL(updated, *(u32*)vector_item_ptr(&vector, 1));
			CHECK_NOT_EQUAL(0, vector_compare_items(&vector, 0, 1));
			CHECK_EQUAL(0, vector_compare_items(&vector, 1, 1));
			CHECK_EQUAL(0, vector_compare_items(&vector, 1, 9));

			u32 inserted = 25;
			vector_insert(&vector, 1, (byte*)&inserted);
			CHECK_EQUAL((u32)4, vector_get_size(&vector));
			CHECK_EQUAL((u32)10, *(u32*)vector_item_ptr(&vector, 0));
			CHECK_EQUAL((u32)25, *(u32*)vector_item_ptr(&vector, 1));
			CHECK_EQUAL((u32)40, *(u32*)vector_item_ptr(&vector, 2));

			u32 appended = 50;
			vector_insert(&vector, vector_get_size(&vector), (byte*)&appended);
			CHECK_EQUAL((u32)5, vector_get_size(&vector));
			CHECK_EQUAL((u32)50, *(u32*)vector_item_ptr(&vector, 4));

			u32 popped = 0;
			CHECK_TRUE(vector_pop(&vector, (byte*)&popped));
			CHECK_EQUAL((u32)50, popped);
			CHECK_EQUAL((u32)4, vector_get_size(&vector));

			vector_remove(&vector, 1);
			CHECK_EQUAL((u32)3, vector_get_size(&vector));
			CHECK_EQUAL((u32)40, *(u32*)vector_item_ptr(&vector, 1));

			vector_remove_swap(&vector, 0);
			CHECK_EQUAL((u32)2, vector_get_size(&vector));
			CHECK_EQUAL((u32)30, *(u32*)vector_item_ptr(&vector, 0));
			CHECK_EQUAL((u32)40, *(u32*)vector_item_ptr(&vector, 1));

			vector_set_size(&vector, 1);
			CHECK_EQUAL((u32)1, vector_get_size(&vector));
			vector_set_size(&vector, 99);
			CHECK_EQUAL((u32)1, vector_get_size(&vector));

			vector_destroy(&vector);
		}

		UNITTEST_TEST(capacity_and_empty_boundaries)
		{
			vector_t vector;
			CHECK_TRUE(vector_setup(&vector, sizeof(u32), 4, 0));
			CHECK_TRUE(vector_ensure_capacity(&vector, 6));
			CHECK_EQUAL((u32)6, vector_get_capacity(&vector));
			CHECK_FALSE(vector_ensure_capacity(&vector, 0));

			u32 value = 7;
			vector_push(&vector, (byte*)&value);
			CHECK_EQUAL((u32)1, vector_get_size(&vector));

			vector_remove(&vector, 8);
			vector_remove_swap(&vector, 8);
			CHECK_EQUAL((u32)1, vector_get_size(&vector));

			u32 popped = 0;
			CHECK_TRUE(vector_pop(&vector, (byte*)&popped));
			CHECK_EQUAL(value, popped);
			CHECK_FALSE(vector_pop(&vector, (byte*)&popped));
			CHECK_EQUAL((byte*)nullptr, vector_items_ptr(&vector));

			vector_destroy(&vector);
		}

		
	}
}
UNITTEST_SUITE_END
