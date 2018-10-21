#include <check.h>
#include "world.c"

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
        world_set_block(world, (ivec3) {10, 10, 10}, BLOCK_GROUND);

        b = world_get_block(world, (ivec3) {10, 10, 10});
        ck_assert_ptr_nonnull(b);
        ck_assert(b->type == BLOCK_GROUND);

        world_destroy(world);
    }
END_TEST

Suite *str_suite(void) {
    Suite *suite = suite_create("world");
    TCase *tcase = tcase_create("case");
    tcase_add_test(tcase, test_world);
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