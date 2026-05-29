#include "ccore/c_allocator.h"

#include "ccontainer/c_map32.h"

#include "cunittest/cunittest.h"

using namespace ncore;

UNITTEST_SUITE_BEGIN(map32)
{
    UNITTEST_FIXTURE(map)
    {
        // UNITTEST_ALLOCATOR;

        UNITTEST_FIXTURE_SETUP() {}
        UNITTEST_FIXTURE_TEARDOWN() {}

        UNITTEST_TEST(setup_teardown)
        {
            map32_t map;
            map32_setup(&map, 16, sizeof(s32), sizeof(s32));

            map32_teardown(&map);
        }

        UNITTEST_TEST(single_insert)
        {
            map32_t map;
            map32_setup(&map, 16, sizeof(s32), sizeof(s32));

            s32 k = 65536;
            s32 v = 12345;
            CHECK_TRUE(map32_insert(&map, k, v));

            map32_teardown(&map);
        }


        // UNITTEST_TEST(map32_s32)
        // {
        //     map32_t map;
        //     map32_setup(&map, 16, sizeof(s32), sizeof(s32));

        //     const s32 c_num_keys = 3;

        //     for (s32 v = 0; v < c_num_keys; ++v)
        //     {
        //         s32 k = v + 65536;
        //         s32 f;
        //         CHECK_TRUE(map32_insert(&map, k, v));
        //         CHECK_TRUE(map32_find(&map, k, f));
        //         CHECK_EQUAL(v, f);
        //         CHECK_TRUE(map32_remove(&map, k));
        //     }
        //     for (s32 v = 0; v < c_num_keys; ++v)
        //     {
        //         s32 k = v + 65536;
        //         CHECK_TRUE(map32_insert(&map, k, v));
        //     }
        //     for (s32 v = 0; v < c_num_keys; ++v)
        //     {
        //         s32 k = v + 65536;
        //         s32 f;
        //         CHECK_TRUE(map32_find(&map, k, f));
        //         CHECK_EQUAL(v, f);
        //         CHECK_TRUE(map32_remove(&map, k));
        //     }

        //     map32_teardown(&map);
        // }
    }
}
UNITTEST_SUITE_END
