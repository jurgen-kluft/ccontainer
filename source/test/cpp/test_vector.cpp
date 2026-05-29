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
			vector_setup(&v, sizeof(u32), 128, 512);
		}

		UNITTEST_TEST(create_use_destroy)
		{
			vector_t darray;
			vector_setup(&darray, sizeof(u32), 512, 512);
			CHECK_EQUAL((u32)512, vector_get_capacity(&darray));
			for (s32 i = 0; i < 512; i++)
				push_item(&darray, (byte*)&i);
			CHECK_EQUAL((u32)512, vector_get_size(&darray));
		}

		UNITTEST_TEST(create_setcap_use_destroy)
		{
			vector_t darray;
			vector_setup(&darray, sizeof(u32), 512, 512);
			CHECK_EQUAL((u32)512, vector_get_capacity(&darray));

			vector_ensure_capacity(&darray, 1024);
			CHECK_EQUAL((u32)1024, vector_get_capacity(&darray));
			CHECK_EQUAL((u32)0, vector_get_size(&darray));

			for (s32 i = 0; i < 1024; i++)
				push_item(&darray, (byte*)&i);
			CHECK_EQUAL((u32)1024, vector_get_size(&darray));
		}
	}
}
UNITTEST_SUITE_END
