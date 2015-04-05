#define EFL_BETA_API_SUPPORT
#define EFL_EO_API_SUPPORT
#include <Elementary.h>
#include <Elementary_Ext.h>

static void
on_done(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   // quit the mainloop (elm_run function will return)
   elm_exit();
}

EAPI_MAIN int
elm_main(int argc EINA_UNUSED, char **argv EINA_UNUSED)
{
   Evas_Object *win, *tabbed_pane, *lb1, *content;

   elm_ext_init();

   win = elm_win_util_standard_add("efm", "tabbed pane  demo");
   evas_object_smart_callback_add(win, "delete,request", on_done, NULL);

   tabbed_pane = eo_add(ELM_TAB_PANE_CLASS, win);

   lb1 = elm_label_add(win);
   elm_object_text_set(lb1, ".........active");

   content = elm_image_add(win);
   elm_image_file_set(content, "/usr/local/share/elementary/images/icon_13.png", NULL);

   eo_do(tabbed_pane, elm_obj_tab_pane_item_add(lb1, content));

   lb1 = elm_label_add(win);
   elm_object_text_set(lb1, ".........active");

   content = elm_image_add(win);
   elm_image_file_set(content, "/usr/local/share/elementary/images/icon_13.png", NULL);

   eo_do(tabbed_pane, elm_obj_tab_pane_item_add(lb1, content));

   lb1 = elm_label_add(win);
   elm_object_text_set(lb1, ".........active");

   content = elm_image_add(win);
   elm_image_file_set(content, "/usr/local/share/elementary/images/icon_12.png", NULL);

   eo_do(tabbed_pane, elm_obj_tab_pane_item_add(lb1, content));

   lb1 = elm_label_add(win);
   elm_object_text_set(lb1, ".........active");

   content = elm_image_add(win);
   elm_image_file_set(content, "/usr/local/share/elementary/images/icon_11.png", NULL);

   eo_do(tabbed_pane, elm_obj_tab_pane_item_add(lb1, content));

   lb1 = elm_label_add(win);
   elm_object_text_set(lb1, ".........active");

   content = elm_image_add(win);
   elm_image_file_set(content, "/usr/local/share/elementary/images/icon_10.png", NULL);

   eo_do(tabbed_pane, elm_obj_tab_pane_item_add(lb1, content));

   lb1 = elm_label_add(win);
   elm_object_text_set(lb1, ".........active");

   content = elm_image_add(win);
   elm_image_file_set(content, "/usr/local/share/elementary/images/icon_14.png", NULL);

   eo_do(tabbed_pane, elm_obj_tab_pane_item_add(lb1, content));
   evas_object_size_hint_align_set(tabbed_pane, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(tabbed_pane, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(tabbed_pane);
   elm_win_resize_object_add(win, tabbed_pane);
   evas_object_resize(win, 200,200);
   evas_object_show(win);

   elm_run();
   return 0;
}
ELM_MAIN()
