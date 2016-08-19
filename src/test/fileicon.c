#define EFL_BETA_API_SUPPORT
#define EFL_EO_API_SUPPORT

#include <Elementary.h>
#include "../lib/fm_monitor.h"
#include "../widgets/elm_file_icon.h"
#include "../lib/Common.h"

#include <Efl.h>

static void
on_done(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   // quit the mainloop (elm_run function will return)
   elm_exit();
}

EAPI_MAIN int
elm_main(int argc EINA_UNUSED, char **argv EINA_UNUSED)
{
   Evas_Object *win, *ic, *tb;

   elm_need_ethumb();
   elm_need_efreet();

   common_theme_init();

   win = elm_win_util_standard_add("efm", "efm - Verne");
   evas_object_smart_callback_add(win, "delete,request", on_done, NULL);

   tb = elm_table_add(win);
   evas_object_size_hint_align_set(tb, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(tb, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(tb);

   ic = efl_add(ELM_FILE_ICON_CLASS, win);
   // eo_do(ic, efl_file_set("/home/marcel//filewithareallyreallylooooooooooooooooooooooooooooooooooooooonnnnggggggname.txt", NULL));
   evas_object_size_hint_align_set(ic, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(ic, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_table_pack(tb, ic, 0, 0, 1, 1);
   evas_object_show(ic);

   ic = efl_add(ELM_FILE_ICON_CLASS, win);
   // eo_do(ic, efl_file_set("/home/marcel/picture/amsterdam/P8204468.JPG", NULL));
   evas_object_size_hint_align_set(ic, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(ic, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_table_pack(tb, ic, 1, 0, 1, 1);
   evas_object_show(ic);

   ic = efl_add(ELM_FILE_ICON_CLASS, win);
   // eo_do(ic, elm_obj_file_icon_extern_mime_handler_set(EINA_TRUE);
   //          efl_file_set("/etc/dhcpcd.conf", NULL));
   evas_object_size_hint_align_set(ic, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(ic, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_table_pack(tb, ic, 0, 1, 1, 1);
   // ecore_timer_add(2.0, func, ic);
   evas_object_show(ic);

   elm_win_resize_object_add(win, tb);
   evas_object_resize(win, 200,200);
   evas_object_show(win);

   elm_run();
   return 0;
}
ELM_MAIN()
