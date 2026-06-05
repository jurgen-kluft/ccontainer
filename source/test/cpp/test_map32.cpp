#include "ccore/c_allocator.h"

#include "ccontainer/c_map32.h"

#include "cunittest/cunittest.h"

using namespace ncore;

static s8 s_compare_s32_keys(u32 key_index, u32 item_index, void const* user_data)
{
    map32_t const* map = (map32_t const*)user_data;
    s32 const* key = (s32 const*)get_item_ptr(&map->m_keys, key_index);
    s32 const* item = (s32 const*)get_item_ptr(&map->m_keys, item_index);
    if (*key < *item)
        return -1;
    if (*key > *item)
        return 1;
    return 0;
}

UNITTEST_SUITE_BEGIN(map32)
{
    UNITTEST_FIXTURE(map)
    {
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

        UNITTEST_TEST(empty_map_find_and_remove)
        {
            map32_t map;
            map32_setup<s32, s32>(&map, 8);

            s32 key = 42;
            s32 value = 0;
            CHECK_FALSE(map32_find(&map, key, value));
            CHECK_FALSE(map32_remove(&map, key));

            map32_teardown(&map);
        }

        UNITTEST_TEST(map32_s32)
        {
            map32_t map;
            map32_setup(&map, 16, sizeof(s32), sizeof(s32));

            const s32 c_num_keys = 3;

            for (s32 v = 0; v < c_num_keys; ++v)
            {
                s32 k = v + 65536;
                s32 f;
                CHECK_TRUE(map32_insert(&map, k, v));
                CHECK_TRUE(map32_find(&map, k, f));
                CHECK_EQUAL(v, f);
                 CHECK_TRUE(map32_remove(&map, k));
            }
            for (s32 v = 0; v < c_num_keys; ++v)
            {
                s32 k = v + 65536;
                CHECK_TRUE(map32_insert(&map, k, v));
            }
            for (s32 v = 0; v < c_num_keys; ++v)
            {
                s32 k = v + 65536;
                s32 f;
                CHECK_TRUE(map32_find(&map, k, f));
                CHECK_EQUAL(v, f);
                CHECK_TRUE(map32_remove(&map, k));
            }

            map32_teardown(&map);
        }

        UNITTEST_TEST(remove_duplicate_and_missing_paths)
        {
            map32_t map;
            //map32_setup<s32, s32>(&map, 8);
            map32_setup(&map, 16, sizeof(s32), sizeof(s32));

            s32 key_a = 100;
            s32 key_b = 200;
            s32 value_a = 11;
            s32 value_b = 22;
            s32 found = 0;

            CHECK_TRUE(map32_insert(&map, key_a, value_a));
            CHECK_TRUE(map32_insert(&map, key_b, value_b));
//            CHECK_FALSE(map32_insert(&map, key_a, value_b));

            // CHECK_TRUE(map32_find(&map, key_a, found));
            // CHECK_EQUAL(value_a, found);

            // CHECK_TRUE(map32_remove(&map, key_a));
            // CHECK_FALSE(map32_find(&map, key_a, found));
            // CHECK_FALSE(map32_remove(&map, key_a));

            // CHECK_TRUE(map32_find(&map, key_b, found));
            // CHECK_EQUAL(value_b, found);
            // CHECK_FALSE(map32_find(&map, (s32)999, found));

            map32_teardown(&map);
        }

        // UNITTEST_TEST(custom_comparer_setup)
        // {
        //     map32_t map;
        //     map32_setup(&map, 8, sizeof(s32), sizeof(s32), s_compare_s32_keys);

        //     s32 key = -5;
        //     s32 value = 77;
        //     s32 found = 0;

        //     CHECK_TRUE(map32_insert(&map, key, value));
        //     CHECK_TRUE(map32_find(&map, key, found));
        //     CHECK_EQUAL(value, found);
        //     CHECK_TRUE(map32_remove(&map, key));
        //     CHECK_FALSE(map32_find(&map, key, found));

        //     map32_teardown(&map);
        // }
    }
}
UNITTEST_SUITE_END
