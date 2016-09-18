#define EFL_BETA_API_SUPPORT
#define EFL_EO_API_SUPPORT

#include <Elementary.h>
#include <Efm.h>
#include <check.h>

#define TEST_FILE_NAME "efm_test_file_XXXXXX.txt"

#define TEST_FILE_ITER "efm_stresstest_%d_XXXXXX.txt"
#define TEST_FILE_ITER_MAX 100

#define TEST_DIRECTORY "efm_test_monitor_XXXXXX"
#define TEST_DIRECTORY_FILES_MAX 100
#define TEST_DIRECTORY_FILES "%s/test_file_%d_XXXXXX.txt"

#define ARCHIVE_FILE_NUMBER 5

START_TEST(efm_file_invalid_name)
{
   Efm_File *file;
   const char *filename = "/I-Am-Invalid";

   efl_object_init();
   efm_init();

   file = efm_file_get(EFM_CLASS, filename);

   ck_assert_ptr_eq(file, NULL);

   efm_shutdown();

}
END_TEST

Eina_Bool done;

static void
_done_cb(void *data EINA_UNUSED, const Efl_Event *event EINA_UNUSED)
{
   done = EINA_TRUE;
   ecore_main_loop_quit();
}

START_TEST(efm_valid_file)
{
   Efm_File *file;
   Eina_Tmpstr *test_dir;

   eina_init();
   ecore_init();

   eina_file_mkstemp(TEST_FILE_NAME, &test_dir);

   printf("Test file in %s\n", test_dir);

   efl_object_init();
   efm_init();

   done = EINA_FALSE;

   file = efm_file_get(EFM_CLASS, test_dir);

   ck_assert_ptr_ne(file, NULL);

   efl_event_callback_add(file, EFM_FILE_EVENT_FSQUERY_DONE, _done_cb, NULL);

   ecore_main_loop_begin();

   ck_assert_int_eq(done, 1);

   efm_shutdown();

   ecore_file_remove(test_dir);

   ecore_shutdown();
   eina_shutdown();
}
END_TEST

int filecounter = 0;

static void
_done2_cb(void *data EINA_UNUSED, const Efl_Event *event EINA_UNUSED)
{
   filecounter ++;
   if (filecounter >= TEST_FILE_ITER_MAX)
     ecore_main_loop_quit();
}

START_TEST(efm_stresstest)
{
   Efm_File *file;
   Eina_Tmpstr *tmptrs[TEST_FILE_ITER_MAX];
   int i;

   eina_init();
   ecore_init();

   for(i = 0; i < TEST_FILE_ITER_MAX; i++)
     {
        char buf[PATH_MAX];
        snprintf(buf, sizeof(buf), TEST_FILE_ITER, i);
        eina_file_mkstemp(buf, &tmptrs[i]);
     }

   efm_init();

   done = EINA_FALSE;
   for (i = 0; i < TEST_FILE_ITER_MAX; i++)
     {
        file = efm_file_get(EFM_CLASS, tmptrs[i]);

        ck_assert_ptr_ne(file, NULL);

        efl_event_callback_add(file, EFM_FILE_EVENT_FSQUERY_DONE, _done2_cb, NULL);
     }
   ecore_main_loop_begin();

   ck_assert_int_eq(filecounter, TEST_FILE_ITER_MAX);

   efm_shutdown();

   for(i = 0; i < TEST_FILE_ITER_MAX; i++)
     {
        ecore_file_remove(tmptrs[i]);
     }

   ecore_shutdown();
   eina_shutdown();
}
END_TEST

Eina_Bool error;
int files;

static void
_error(void *data EINA_UNUSED, const Efl_Event *event EINA_UNUSED)
{
   error = EINA_TRUE;
   ecore_main_loop_quit();
}

#if 0
static Eina_Bool
_del(void *data, Eo *obj, const Efl_Event_Description *desc,  void *event)
{
   return EINA_TRUE;
}
#endif

static void
_add(void *data EINA_UNUSED, const Efl_Event *event EINA_UNUSED)
{
   files ++;
   if (files >= TEST_DIRECTORY_FILES_MAX)
     ecore_main_loop_quit();
}

START_TEST(efm_monitor_test)
{
   Efm_Monitor *mon;
   Efm_File *f;
   int i;
   Eina_Tmpstr *dir;

   eina_init();
   ecore_init();

   eina_file_mkdtemp(TEST_DIRECTORY, &dir);

   for(i = 0; i < TEST_DIRECTORY_FILES_MAX; i++)
     {
        char buf[PATH_MAX];
        snprintf(buf, sizeof(buf), TEST_DIRECTORY_FILES, dir, i);
        eina_file_mkstemp(buf, NULL);
     }

   error = EINA_FALSE;
   files = 0;

   efl_object_init();
   efm_init();

   f = efm_file_get(EFM_CLASS, dir);

   mon = efm_file_monitor(f, NULL);

//      efl_event_callback_add(EFM_MONITOR_EVENT_FILE_DEL, _del, NULL);
   efl_event_callback_add(mon, EFM_MONITOR_EVENT_FILE_ADD, _add, NULL);
   efl_event_callback_add(mon, EFM_MONITOR_EVENT_ERROR, _error, NULL);


   ecore_main_loop_begin();

   ck_assert_int_eq(error, 0);
   ck_assert_int_eq(files, TEST_DIRECTORY_FILES_MAX);

   ecore_file_remove(dir);

   efm_shutdown();
   ecore_shutdown();
   eina_shutdown();
}
END_TEST

int mon_files;

static void
_error_mon(void *data EINA_UNUSED, const Efl_Event *event EINA_UNUSED)
{
   mon_files = -1;
   ecore_main_loop_quit();
}

#if 0
static Eina_Bool
_del(void *data, Eo *obj, const Efl_Event_Description *desc,  void *event)
{
   return EINA_TRUE;
}
#endif

static void
_add_mon(void *data EINA_UNUSED, const Efl_Event *event EINA_UNUSED)
{
   mon_files ++;
   if (mon_files >= ARCHIVE_FILE_NUMBER)
     ecore_main_loop_quit();
}

START_TEST(efm_archive_monitor_test)
{
   Efm_Monitor *archive;
   Efm_File *f;

   efl_object_init();
   efm_init();
   f = efm_archive_get(EFM_CLASS, TEST_RESSOURCES"/src/test/archiv.tar", "zip-test/");

   archive = efm_file_monitor(f, NULL);
   ck_assert_ptr_ne(archive, NULL);
   efl_event_callback_add(archive, EFM_MONITOR_EVENT_FILE_ADD, _add_mon, NULL);
   efl_event_callback_add(archive, EFM_MONITOR_EVENT_ERROR, _error_mon, NULL);
   ecore_main_loop_begin();
   ck_assert_int_eq(mon_files, ARCHIVE_FILE_NUMBER);

   efm_shutdown();
}
END_TEST

START_TEST(efm_archive_test)
{
   Efm_File *archive;

   efl_object_init();
   efm_init();

   archive = efm_archive_get(EFM_CLASS, TEST_RESSOURCES"/src/test/archiv.tar", "zip-test/dir1/bla1");

   ck_assert_ptr_ne(archive, NULL);

   efm_shutdown();
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
    tcase_add_test(tc_core, efm_archive_monitor_test);

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
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
 }