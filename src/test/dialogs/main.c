#define EFL_BETA_API_SUPPORT
#define EFL_EO_API_SUPPORT

#include <Elementary_Ext.h>

#if 0
static void
_complex(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{

}
#endif

static void
_apply(void *data EINA_UNUSED, const Efl_Event *event EINA_UNUSED)
{
   printf("Apply APPLY\n");
}

static void
_cancel(void *data EINA_UNUSED, const Efl_Event *event EINA_UNUSED)
{
   printf("Cancel CANCEL\n");
}

static void
_simple(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win;

   win = efl_add(ELM_DIALOG_DECISION_CLASS, NULL);
   efl_event_callback_add(win, ELM_DIALOG_DECISION_EVENT_CANCEL, _cancel, NULL);
   efl_event_callback_add(win, ELM_DIALOG_DECISION_EVENT_APPLY, _apply, NULL);
   elm_win_title_set(win, "bla");
   elm_dialog_icon_set(win, "dialog-error");
   elm_object_text_set(win, "<p align=center>And we have a sample text with error sign<br>->Testing<-</align>");
   evas_object_show(win);
}

int
elm_main(int argc EINA_UNUSED, char **argv EINA_UNUSED)
{
   Evas_Object *win, *box, *simple, *o;
   elm_ext_init();

   win = elm_win_util_standard_add("Dialog test", "Dialog test");

   o = box = elm_box_add(win);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, o);
   evas_object_show(o);
#if 0
   o = complex = elm_button_add(o);
   evas_object_smart_callback_add(o, "clicked", _complex, NULL);
   elm_object_text_set(o, "Complex");
   elm_box_pack_end(box, o);
   evas_object_show(o);
#endif
   o = simple = elm_button_add(o);
   evas_object_smart_callback_add(o, "clicked", _simple, NULL);
   elm_object_text_set(o, "Simple");
   elm_box_pack_end(box, o);
   evas_object_show(o);

   evas_object_show(win);

   elm_run();

   return 0;
}
ELM_MAIN()