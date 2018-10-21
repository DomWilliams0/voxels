#include <check.h>
#include "world.h"

START_TEST (test_nop)
    {
        struct world *world;
        world_load_demo(&world, "");
        world_destroy(world);
    }
END_TEST

Suite *str_suite(void) {
    Suite *suite = suite_create("world");
    TCase *tcase = tcase_create("case");
    tcase_add_test(tcase, test_nop);
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