#define EFL_EO_API_SUPPORT
#include <check.h>
#include <stdlib.h>
#include <stdio.h>
#include <Emous.h>
#include <Ecore.h>
#include "../lib/emous/Emous.h"
#include "emous_test.h"
//=============================================================================

static Eina_Bool mount_added;

static Eina_Bool
_mount_add(void *data EINA_UNUSED, const Eo_Event *event EINA_UNUSED)
{
  mount_added = EINA_TRUE;
  return EINA_TRUE;
}

START_TEST(emous_mount_point_listening)
{
   Emous_Manager *manager;

   eo_init();

   emous_init_lib(EMOUS_CLASS);
   mount_added = EINA_FALSE;
   manager = emous_manager_object_get(EMOUS_MANAGER_CLASS);
   eo_event_callback_add(manager, EMOUS_MANAGER_EVENT_MOUNT_ADD, _mount_add, NULL);
   emous_init(EMOUS_CLASS);

   ck_assert_int_eq(mount_added, EINA_TRUE);

   emous_shutdown(EMOUS_CLASS);
   emous_shutdown_lib(EMOUS_CLASS);
}
END_TEST

//=============================================================================

START_TEST(normal_init)
{
   eo_init();

   ck_assert_int_eq(emous_init(EMOUS_CLASS), 1);
   emous_shutdown(EMOUS_CLASS);
}
END_TEST

//==============================================

static int adddev = 0;
static int deldev = 0;

static Eina_Bool
_add_cb(void *data EINA_UNUSED, const Eo_Event *event EINA_UNUSED)
{
  adddev ++;
  return EINA_TRUE;
}
static Eina_Bool
_del_cb(void *data EINA_UNUSED, const Eo_Event *event EINA_UNUSED)
{
  deldev ++;
  return EINA_TRUE;
}

START_TEST(debug_devices_appear)
{
   Emous_Manager *manager;
   Eina_List *list;

   eo_init();
   emous_init_lib(EMOUS_CLASS);
   manager = emous_manager_object_get(EMOUS_MANAGER_CLASS);
                  eo_event_callback_add(manager, EMOUS_MANAGER_EVENT_DEVICE_ADD, _add_cb, NULL);
                  eo_event_callback_add(manager, EMOUS_MANAGER_EVENT_DEVICE_DEL, _del_cb, NULL);

   emous_debug_device_start();

   list = emous_manager_devices_get(EMOUS_MANAGER_CLASS);

   ck_assert_int_eq(eina_list_count(list), 1);

   ck_assert_int_eq(adddev, 2);
   ck_assert_int_eq(deldev, 1);

   emous_shutdown_lib(EMOUS_CLASS);
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