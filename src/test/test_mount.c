#define EFL_EO_API_SUPPORT
#define EFL_BETA_API_SUPPORT

#include <check.h>
#include <stdlib.h>
#include <stdio.h>
#include <Emous.h>
#include <Ecore.h>
#include "../lib/emous/Emous_Test.h"
//=============================================================================

static Eina_Bool mount_call;
static Eina_Bool umount_call;
static Eina_Bool state_changed;

static Eina_Bool mm_add;
static Eina_Bool mm_del;
static Eina_Bool mm_req_a;
static Eina_Bool mm_req_e;

static Eina_Bool
_m_mount_add(void *data EINA_UNUSED, Eo *obj EINA_UNUSED, const Eo_Event_Description *desc EINA_UNUSED, void *event EINA_UNUSED)
{
   mm_add = EINA_TRUE;
   return EINA_TRUE;
}

static Eina_Bool
_m_mount_del(void *data EINA_UNUSED, Eo *obj EINA_UNUSED, const Eo_Event_Description *desc EINA_UNUSED, void *event EINA_UNUSED)
{
   mm_del = EINA_TRUE;
   return EINA_TRUE;
}

static Eina_Bool
_m_mount_req_a(void *data EINA_UNUSED, Eo *obj EINA_UNUSED, const Eo_Event_Description *desc EINA_UNUSED, void *event EINA_UNUSED)
{
   mm_req_a = EINA_TRUE;
   return EINA_TRUE;
}

static Eina_Bool
_m_mount_req_e(void *data EINA_UNUSED, Eo *obj EINA_UNUSED, const Eo_Event_Description *desc EINA_UNUSED, void *event EINA_UNUSED)
{
   mm_req_e = EINA_TRUE;
   return EINA_TRUE;
}

static Eina_Bool
_mount_req(void *data EINA_UNUSED, Eo *obj EINA_UNUSED, const Eo_Event_Description *desc EINA_UNUSED, void *event EINA_UNUSED)
{
   mount_call = EINA_TRUE;
      eo_do(obj, emous_device_state_set(DEVICE_STATE_MOUNTREQ);
                 emous_device_state_set(DEVICE_STATE_MOUNTED));
   return EINA_TRUE;
}

static Eina_Bool
_umount_req(void *data EINA_UNUSED, Eo *obj EINA_UNUSED, const Eo_Event_Description *desc EINA_UNUSED, void *event EINA_UNUSED)
{
   umount_call = EINA_TRUE;
   eo_do(obj, emous_device_state_set(DEVICE_STATE_UMOUNTREQ);
              emous_device_state_set(DEVICE_STATE_UMOUNTED));
   return EINA_TRUE;
}

static Eina_Bool
_state_changed(void *data EINA_UNUSED, Eo *obj EINA_UNUSED, const Eo_Event_Description *desc EINA_UNUSED, void *event EINA_UNUSED)
{
   state_changed = EINA_TRUE;

   return EINA_TRUE;
}

START_TEST(test_something)
{
   Emous_Manager *m;
   Emous_Device_Type *c;
   Emous_Device *d;

   ck_assert_int_eq(emous_test_init(), 1);

   eo_do(EMOUS_MANAGER_CLASS, m = emous_manager_object_get());
   ck_assert_ptr_ne(m, NULL);

   eo_do(m,
            eo_event_callback_add(EMOUS_MANAGER_EVENT_MOUNT_ADD, _m_mount_add, NULL);
            eo_event_callback_add(EMOUS_MANAGER_EVENT_MOUNT_DEL, _m_mount_del, NULL);
            eo_event_callback_add(EMOUS_MANAGER_EVENT_MOUNT_REQUEST_ADD, _m_mount_req_a, NULL);
            eo_event_callback_add(EMOUS_MANAGER_EVENT_MOUNT_REQUEST_END, _m_mount_req_e, NULL);
            );


   eo_do(EMOUS_MANAGER_CLASS, c = emous_manager_device_type_add("test_class"));
   ck_assert_ptr_ne(c,NULL);

   eo_do(c, d = emous_device_type_device_add("test_device"));
   ck_assert_ptr_ne(d, NULL);

   eo_do(d, emous_device_populate();
            eo_event_callback_add(EMOUS_DEVICE_EVENT_MOUNT_REQUEST, _mount_req, NULL);
            eo_event_callback_add(EMOUS_DEVICE_EVENT_UMOUNT_REQUEST, _umount_req, NULL);
            eo_event_callback_add(EMOUS_DEVICE_EVENT_STATE_CHANGED, _state_changed, NULL));

   mount_call = EINA_FALSE;
   umount_call = EINA_FALSE;
   state_changed = EINA_FALSE;
   mm_del = EINA_FALSE;
   mm_add = EINA_FALSE;
   mm_req_e = EINA_FALSE;
   mm_req_a = EINA_FALSE;

   eo_do(d, emous_device_mount());

   ck_assert(mount_call == EINA_TRUE);
   ck_assert(umount_call == EINA_FALSE);

   ck_assert(mm_req_a == EINA_TRUE);
   ck_assert(mm_add == EINA_TRUE);
   ck_assert(mm_req_e == EINA_TRUE);

   ck_assert(mm_del == EINA_FALSE);

   mount_call = EINA_FALSE;
   mm_del = EINA_FALSE;
   mm_add = EINA_FALSE;
   mm_req_e = EINA_FALSE;
   mm_req_a = EINA_FALSE;

   eo_do(d, emous_device_umount());
   ck_assert(umount_call == EINA_TRUE);
   ck_assert(mount_call == EINA_FALSE);

   ck_assert(mm_req_e == EINA_TRUE);
   ck_assert(mm_req_a == EINA_TRUE);
   ck_assert(mm_del == EINA_TRUE);

   ck_assert(mm_add == EINA_FALSE);

   ck_assert(state_changed == EINA_TRUE);

   emous_shutdown();
}
END_TEST

//=============================================================================

static Eina_Bool checked;

static Eina_Bool
_check(void *data EINA_UNUSED, Eo *obj EINA_UNUSED, const Eo_Event_Description *desc EINA_UNUSED, void *event EINA_UNUSED)
{
   checked = EINA_TRUE;

   return EINA_TRUE;
}

START_TEST(file_backend)
{
   Emous_Device_Type *c;

   ck_assert_int_eq(emous_test_init(), 1);

   checked = EINA_FALSE;
   eo_do(EMOUS_MANAGER_CLASS, c = emous_manager_device_type_add("file"));
   eo_do(c, emous_device_type_keyword_add("ext4");
            eo_event_callback_add(EMOUS_DEVICE_TYPE_EVENT_MOUNTPOINT_CHECK_ADD, _check, NULL));
   emous_test_init2();
   ck_assert(checked == EINA_TRUE);

   emous_shutdown();
}
END_TEST

START_TEST(normal_init)
{
   ck_assert_int_eq(emous_init(), 1);
   emous_shutdown();
}
END_TEST

//==============================================

static Eina_Bool device_add;

static Eina_Bool
_add(void *data EINA_UNUSED, Eo *obj EINA_UNUSED, const Eo_Event_Description *desc EINA_UNUSED, void *event EINA_UNUSED)
{
   Emous_Device *d = event;
   printf("appeared %p\n", d);
   device_add = EINA_TRUE;
   return EINA_TRUE;
}

static Eina_Bool
_stop(void *data EINA_UNUSED)
{
   ecore_main_loop_quit();
   return EINA_FALSE;
}

START_TEST(mainloop_fireup_test)
{
   Emous_Manager *m;
   device_add = EINA_FALSE;
   ecore_init();
   ck_assert_int_eq(emous_init(), 1);

   eo_do(EMOUS_MANAGER_CLASS, m = emous_manager_object_get());

   eo_do(m, eo_event_callback_add(EMOUS_MANAGER_EVENT_DEVICE_ADD, _add, NULL));

   ecore_timer_add(5, _stop, NULL);

   ecore_main_loop_begin();
   ck_assert(device_add == EINA_TRUE);
   ecore_shutdown();
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
    tcase_add_test(tc_core, test_something);
    tcase_add_test(tc_core, file_backend);
    tcase_add_test(tc_core, normal_init);
    tcase_add_test(tc_core, mainloop_fireup_test);

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