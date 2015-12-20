#define EFL_BETA_API_SUPPORT
#define EFL_EO_API_SUPPORT

#include <Elementary.h>
#include <Efm.h>
#include <check.h>

#define TEST_FILE "/tmp/bla.txt"
#define TEST_FILE_ITER "/tmp/bla%d.txt"
#define TEST_FILE_ITER_MAX 100

#define TEST_DIRECTORY "/tmp/test"
#define TEST_DIRECTORY_FILES_MAX 100
#define TEST_DIRECTORY_FILES "/tmp/test/test_file%d.txt"

START_TEST(efm_file_invalid_name)
{
   Efm_File *file;
   const char *filename = "I-Am-Invalid";

   eo_init();
   eo_do(EFM_CLASS, efm_init());

   eo_do(EFM_CLASS, file = efm_file_get(filename));

   ck_assert_ptr_ne(file, NULL);

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

   system("touch "TEST_FILE);

   eo_init();
   eo_do(EFM_CLASS, efm_init());

   done = EINA_FALSE;

   eo_do(EFM_CLASS, file = efm_file_get(filename));

   ck_assert_ptr_ne(file, NULL);

   eo_do(file, eo_event_callback_add(EFM_FILE_EVENT_FSQUERY_DONE, _done_cb, NULL));

   ecore_main_loop_begin();

   ck_assert_int_eq(done, 1);

   eo_do(EFM_CLASS, efm_shutdown());
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
   eo_do(EFM_CLASS, efm_init());

   done = EINA_FALSE;
   for (i = 0; i < TEST_FILE_ITER_MAX; i++)
     {
        eo_do(EFM_CLASS, file = efm_file_get(filename));

        ck_assert_ptr_ne(file, NULL);

        eo_do(file, eo_event_callback_add(EFM_FILE_EVENT_FSQUERY_DONE, _done2_cb, NULL));
     }
   ecore_main_loop_begin();

   ck_assert_int_eq(filecounter, TEST_FILE_ITER_MAX);

   efm_shutdown();
   ecore_shutdown();
   eina_shutdown();
}
END_TEST

Eina_Bool error;
int files;

static Eina_Bool
_error(void *data EINA_UNUSED, Eo *obj EINA_UNUSED, const Eo_Event_Description *desc EINA_UNUSED,  void *event EINA_UNUSED)
{
   error = EINA_TRUE;
   ecore_mainloop_quit();
   return EINA_TRUE;
}

#if 0
static Eina_Bool
_del(void *data, Eo *obj, const Eo_Event_Description *desc,  void *event)
{
   return EINA_TRUE;
}
#endif

static Eina_Bool
_add(void *data EINA_UNUSED, Eo *obj EINA_UNUSED, const Eo_Event_Description *desc EINA_UNUSED,  void *event EINA_UNUSED)
{
   files ++;
   if (files >= TEST_DIRECTORY_FILES_MAX)
     ecore_main_loop_quit();
   return EINA_TRUE;
}

START_TEST(efm_monitor_test)
{
   Efm_Monitor *mon;
   int i;

   system("mkdir -p "TEST_DIRECTORY);

   for(i = 0; i < TEST_DIRECTORY_FILES_MAX; i++)
     {
        char buf[PATH_MAX];
        snprintf(buf, sizeof(buf), "touch "TEST_DIRECTORY_FILES, i);
        system(buf);
     }

   error = EINA_FALSE;
   files = 0;

   eo_init();
   eo_do(EFM_CLASS, efm_init());

   mon = eo_add(EFM_MONITOR_CLASS, NULL, efm_monitor_install(TEST_DIRECTORY, NULL));
   eo_do (mon,
//      eo_event_callback_add(EFM_MONITOR_EVENT_FILE_DEL, _del, NULL);
      eo_event_callback_add(EFM_MONITOR_EVENT_FILE_ADD, _add, NULL);
      eo_event_callback_add(EFM_MONITOR_EVENT_ERROR, _error, NULL);
   );

   ecore_main_loop_begin();

   ck_assert_int_eq(error, 0);
   ck_assert_int_eq(files, TEST_DIRECTORY_FILES_MAX);
   eo_do(EFM_CLASS, efm_shutdown());
}
END_TEST


START_TEST(efm_archive_test)
{
   Efm_File *archive;

   eo_init();
   eo_do(EFM_CLASS, efm_init());

   eo_do(EFM_CLASS, archive = efm_archive_get("/home/marcel/git/efm/src/test/archiv.tar", "zip-test/dir1/bla1"));

   ck_assert_ptr_ne(archive, NULL);

   eo_do(EFM_CLASS, efm_shutdown());
}
END_TEST


Suite * efm_suite(void)
{
    Suite *s;
    TCase *tc_core;

    s = suite_create("efm_monitor");

    /* Core test case */
    tc_core = tcase_create("efm_file");

    tcase_set_timeout(tc_core, 7);
    tcase_add_test(tc_core, efm_file_invalid_name);
    tcase_add_test(tc_core, efm_valid_file);
    tcase_add_test(tc_core, efm_stresstest);
    tcase_add_test(tc_core, efm_archive_test);
    tcase_add_test(tc_core, efm_monitor_test);

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

    srunner_run_all(sr, CK_NORMAL);
    srunner_set_fork_status(sr, CK_NOFORK);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
 }