#include <Elementary_Ext.h>


static void
on_done(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   // quit the mainloop (elm_run function will return)
   elm_exit();
}

static Eina_Bool
dir_cb(void *data EINA_UNUSED, const Eo_Event *event EINA_UNUSED)
{
   printf("directory changed to %s\n", (char*)event);
   return EINA_TRUE;
}

static Eina_Bool
item_cb(void *data EINA_UNUSED, const Eo_Event *event EINA_UNUSED)
{
   printf("item choosen %s\n", (char*)event);
   return EINA_TRUE;
}


static Eina_Bool
_menu_hook_start(void *data EINA_UNUSED, const Eo_Event *event EINA_UNUSED)
{
   elm_menu_item_add(event->info, NULL, NULL, "you can add menu entrys",
                     NULL, NULL);
   return EINA_TRUE;
}

static Eina_Bool
_menu_hook_end(void *data EINA_UNUSED, const Eo_Event *event EINA_UNUSED)
{
   elm_menu_item_add(event->info, NULL, NULL, "even on the end!!",
                     NULL, NULL);
   return EINA_TRUE;
}

int
elm_main(int argc EINA_UNUSED, char **argv EINA_UNUSED)
{
   Evas_Object *win, *ic, *bx;

   elm_need_ethumb();
   elm_need_efreet();

   elm_ext_init();

   win = elm_win_util_standard_add("efm", "efm - Jesus");
   evas_object_smart_callback_add(win, "delete,request", on_done, NULL);

   bx = elm_box_add(win);
   evas_object_size_hint_align_set(bx, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bx);

   ic = eo_add(ELM_FILE_SELECTOR_CLASS, win);
   efl_file_set(ic, "/home/marcel", NULL);
   eo_event_callback_add(ic, ELM_FILE_SELECTOR_EVENT_PATH_CHANGED,
                         dir_cb, NULL);
   eo_event_callback_add(ic, ELM_FILE_SELECTOR_EVENT_ITEM_CHOOSEN,
                         item_cb, NULL);
   eo_event_callback_add(ic, ELM_FILE_SELECTOR_EVENT_HOOK_MENU_SELECTOR_START,
                         _menu_hook_start, NULL);
   eo_event_callback_add(ic, ELM_FILE_SELECTOR_EVENT_HOOK_MENU_SELECTOR_END,
                         _menu_hook_end, NULL);
   evas_object_size_hint_align_set(ic, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(ic, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_pack_end(bx, ic);
   evas_object_show(ic);
#if 1
   ic = eo_add(ELM_FILE_DISPLAY_CLASS, win);

   elm_file_display_bookmarks_show_set(ic, EINA_FALSE);
   elm_file_display_filepreview_show_set(ic, EINA_FALSE);
   evas_object_size_hint_align_set(ic, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(ic, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_pack_end(bx, ic);
   evas_object_show(ic);
#endif
   elm_win_resize_object_add(win, bx);
   evas_object_resize(win, 200,200);
   evas_object_show(win);

   elm_run();
   return 0;
}
ELM_MAIN()
