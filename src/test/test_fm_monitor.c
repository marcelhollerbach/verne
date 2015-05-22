#define EFL_BETA_API_SUPPORT
#define EFL_EO_API_SUPPORT

#include <Elementary.h>
#include <Efm.h>
#include <check.h>

#define TEST_FILE "/tmp/bla.txt"
#define TEST_FILE_ITER "/tmp/bla%d.txt"
#define TEST_FILE_ITER_MAX 100

START_TEST(efm_file_invalid_name)
{
   Efm_File *file;
   Eina_Bool res;
   const char *filename = "I-Am-Invalid";

   efm_init();

   file = eo_add(EFM_FILE_CLASS, NULL, res = efm_file_obj_generate(filename));

   ck_assert_int_eq(res, 0);

   efm_shutdown();

}
END_TEST

Eina_Bool done;

static Eina_Bool
_done_cb(void *data EINA_UNUSED, Eo *obj EINA_UNUSED, const Eo_Event_Description *desc EINA_UNUSED, void *event EINA_UNUSED)
{
   done = EINA_TRUE;
   ecore_main_loop_quit();
   printf("DONE");
   return EINA_TRUE;
}

START_TEST(efm_valid_file)
{
   Efm_File *file;
   const char *filename = TEST_FILE;
   Eina_Bool res;

   system("touch "TEST_FILE);

   eina_init();
   ecore_init();
   efm_init();

   done = EINA_FALSE;

   file = eo_add(EFM_FILE_CLASS, NULL, res = efm_file_obj_generate(filename));

   ck_assert_int_eq(res, 1);

   eo_do(file, eo_event_callback_add(EFM_FILE_EVENT_FSQUERY_DONE, _done_cb, NULL));

   ecore_main_loop_begin();

   ck_assert_int_eq(done, 1);

   efm_shutdown();
   ecore_shutdown();
   eina_shutdown();
}
END_TEST

int filecounter = 0;

static Eina_Bool
_done2_cb(void *data EINA_UNUSED, Eo *obj EINA_UNUSED, const Eo_Event_Description *desc EINA_UNUSED, void *event EINA_UNUSED)
{
   filecounter ++;
   if (filecounter >= TEST_FILE_ITER_MAX)
     ecore_main_loop_quit();
   return EINA_TRUE;
}

START_TEST(efm_stresstest)
{
   Efm_File *file;
   const char *filename = TEST_FILE;
   Eina_Bool res;
   int i;

   for(i = 0; i < TEST_FILE_ITER_MAX; i++)
     {
        char buf[PATH_MAX];
        snprintf(buf, sizeof(buf), "touch "TEST_FILE_ITER, i);
        printf("Running %s - %s", buf, TEST_FILE_ITER);
        system(buf);
     }


   eina_init();
   ecore_init();
   efm_init();

   done = EINA_FALSE;
   for (i = 0; i < TEST_FILE_ITER_MAX; i++)
     {
        file = eo_add(EFM_FILE_CLASS, NULL, res = efm_file_obj_generate(filename));

        ck_assert_int_eq(res, 1);

        eo_do(file, eo_event_callback_add(EFM_FILE_EVENT_FSQUERY_DONE, _done2_cb, NULL));
     }
   ecore_main_loop_begin();

   ck_assert_int_eq(filecounter, TEST_FILE_ITER_MAX);

   efm_shutdown();
   ecore_shutdown();
   eina_shutdown();
}
END_TEST
Suite * efm_suite(void)
{
    Suite *s;
    TCase *tc_core;

    s = suite_create("efm_monitor");

    /* Core test case */
    tc_core = tcase_create("fm_monitor");

    tcase_set_timeout(tc_core, 7);
    tcase_add_test(tc_core, efm_file_invalid_name);
    tcase_add_test(tc_core, efm_valid_file);
    tcase_add_test(tc_core, efm_stresstest);

    suite_add_tcase(s, tc_core);

    return s;
}

int main(void)
{
    int number_failed;
    Suite *s;
    SRunner *sr;

    s = efm_suite();
    sr = srunner_create(s);
    //srunner_set_fork_status(sr, CK_NOFORK);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
 }