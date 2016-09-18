#define EFL_BETA_API_SUPPORT
#include <check.h>
#include <stdlib.h>
#include <stdio.h>
#include <Emous.h>
#include <Ecore.h>
#include "../lib/emous/Emous.h"
#include "emous_test.h"
//=============================================================================

static Eina_Bool mount_added;

static void
_mount_add(void *data EINA_UNUSED, const Efl_Event *event EINA_UNUSED)
{
  mount_added = EINA_TRUE;
}

START_TEST(emous_mount_point_listening)
{
   Emous_Manager *manager;

   efl_object_init();

   mount_added = EINA_FALSE;
   manager = emous_manager_object_get(EMOUS_MANAGER_CLASS);
   ck_assert_ptr_ne(manager, NULL);
   efl_event_callback_add(manager, EMOUS_MANAGER_EVENT_MOUNT_ADD, _mount_add, NULL);
   emous_init();

   ck_assert_int_eq(mount_added, EINA_TRUE);

   emous_shutdown();
}
END_TEST

//=============================================================================

START_TEST(normal_init)
{
   efl_object_init();

   ck_assert_int_eq(emous_init(), 1);
   emous_shutdown();
}
END_TEST

//==============================================

static int adddev = 0;
static int deldev = 0;

static void
_add_cb(void *data EINA_UNUSED, const Efl_Event *event EINA_UNUSED)
{
  adddev ++;
}
static void
_del_cb(void *data EINA_UNUSED, const Efl_Event *event EINA_UNUSED)
{
  deldev ++;
}

START_TEST(debug_devices_appear)
{
   Emous_Manager *manager;
   Eina_List *list;

   efl_object_init();
   emous_init();
   manager = emous_manager_object_get(EMOUS_MANAGER_CLASS);
   efl_event_callback_add(manager, EMOUS_MANAGER_EVENT_DEVICE_ADD, _add_cb, NULL);
   efl_event_callback_add(manager, EMOUS_MANAGER_EVENT_DEVICE_DEL, _del_cb, NULL);

   emous_debug_device_start();

   list = emous_manager_devices_get(EMOUS_MANAGER_CLASS);

   ck_assert_int_eq(eina_list_count(list), 1);

   ck_assert_int_eq(adddev, 2);
   ck_assert_int_eq(deldev, 1);

   emous_shutdown();
}
END_TEST

Suite * emous_suite(void)
{
    Suite *s;
    TCase *tc_core;

    s = suite_create("emous");

    /* Core test case */
    tc_core = tcase_create("Mount");

    tcase_set_timeout(tc_core, 7);
    tcase_add_test(tc_core, normal_init);
    tcase_add_test(tc_core, emous_mount_point_listening);
    tcase_add_test(tc_core, debug_devices_appear);

    suite_add_tcase(s, tc_core);

    return s;
}

int main(void)
{
    int number_failed;
    Suite *s;
    SRunner *sr;

    s = emous_suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
 }