#include <check.h>
#include "world.c"
#include "chunk_mesh.c"

START_TEST (test_world)
    {
        struct world *world = NULL;
        world_load_demo(&world, "empty");

        ck_assert_ptr_nonnull(world);

        struct block *b = world_get_block(world, (ivec3) {10, 10, 10});
        ck_assert_ptr_null(b);

        world_add_chunk(world, (ivec3) {0, 0, 0});
        b = world_get_block(world, (ivec3) {10, 10, 10});
        ck_assert_ptr_nonnull(b);

        ck_assert(b->type == BLOCK_AIR);
        demo_set_block_safely(world, (ivec3) {10, 10, 10}, BLOCK_GROUND);

        b = world_get_block(world, (ivec3) {10, 10, 10});
        ck_assert_ptr_nonnull(b);
        ck_assert(b->type == BLOCK_GROUND);

        struct chunk_iterator it;
        world_chunks_first(world, &it);
        ck_assert_ptr_nonnull(it.current);
        ck_assert(chunk_has_flag(it.current, CHUNK_FLAG_NEW));
        ck_assert(chunk_has_flag(it.current, CHUNK_FLAG_DIRTY));

        world_chunks_clear_dirty(world);
        ck_assert(!chunk_has_flag(it.current, CHUNK_FLAG_NEW));
        ck_assert(!chunk_has_flag(it.current, CHUNK_FLAG_DIRTY));

        world_chunks_next(world, &it);
        ck_assert_ptr_null(it.current);


        world_destroy(world);
    }
END_TEST

START_TEST(test_mesh)
    {
        ivec3 vec;
        expand_flat_index(0, vec);
        ck_assert(vec[0] == 0 && vec[1] == 0 && vec[2] == 0);

        expand_flat_index(4719, vec);
        ck_assert(vec[0] == 15 && vec[1] == 19 && vec[2] == 4);

        expand_flat_index(32625, vec);
        ck_assert(vec[0] == 17 && vec[1] == 27 && vec[2] == 31);
    }
END_TEST

Suite *str_suite(void) {
    Suite *suite = suite_create("world");
    TCase *tcase = tcase_create("world");
    tcase_add_test(tcase, test_world);
    suite_add_tcase(suite, tcase);

    tcase = tcase_create("mesh");
    tcase_add_test(tcase, test_mesh);
    suite_add_tcase(suite, tcase);
    return suite;
}

int main(int argc, char *argv[]) {
    int number_failed;
    Suite *suite = str_suite();
    SRunner *runner = srunner_create(suite);
    srunner_run_all(runner, CK_NORMAL);
    number_failed = srunner_ntests_failed(runner);
    srunner_free(runner);
    return number_failed;
}