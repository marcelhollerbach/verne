#include "jesus.h"

Evas_Object *win;
Evas_Object *preview;
Evas_Object *layout;

static void
on_done(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   // quit the mainloop (elm_run function will return)
   elm_exit();
}

void
printHelp()
{
   printf("usage: jesus [Path] \n If path is not given $HOME will be taken, if this is not set / will be used.");
}

static Eina_Bool
_dir_changed(void *data EINA_UNUSED, Eo *obj EINA_UNUSED, const Eo_Event_Description *desc EINA_UNUSED, void *event)
{
   titlebar_path_set(event);

   return EINA_FALSE;
}

static void
ui_init()
{
   Evas_Object *icon;

   win = elm_win_util_standard_add("efm", "efm - Jesus");

   icon = elm_icon_add(win);
   elm_icon_standard_set(icon, "system-file-manager");
   evas_object_show(icon);
   elm_win_icon_object_set(win, icon);

   evas_object_smart_callback_add(win, "delete,request", on_done, NULL);

   layout = elm_layout_add(win);
   eo_do(layout, efl_file_set(THEME_PATH"/efm.edc.edj", "headbar"));
   evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(layout);

   preview = eo_add(ELM_FILE_DISPLAY_CLASS, win);
   evas_object_size_hint_align_set(preview, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(preview, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_object_part_content_set(layout, "jesus.content", preview);
   eo_do(preview, eo_event_callback_add(ELM_FILE_DISPLAY_EVENT_PATH_CHANGED_USER,
                                 _dir_changed, NULL););
   evas_object_show(preview);

   elm_object_focus_set(preview, EINA_TRUE);

   titlebar_init();

   shortcuts_init();

   elm_win_resize_object_add(win, layout);
   evas_object_resize(win, 800,600);
   evas_object_show(win);
}

EAPI_MAIN int
elm_main(int argc EINA_UNUSED, char **argv EINA_UNUSED)
{
   char *path = NULL;

   // check if someone gave us a path
   if (argc > 2)
     {
        printHelp();
     }
   else if (argc > 1)
     {
        path = argv[1];
     }

   // if not check if $HOME is set
   if (!path)
     {
        path = getenv("HOME");
     }

   // if everything fails we just take /
   if (!path)
     {
        path = "/";
     }

   // set app informations
   elm_app_name_set("Jesus");
   elm_app_compile_bin_dir_set(PACKAGE_BIN_DIR);
   elm_app_compile_lib_dir_set(PACKAGE_LIB_DIR);
   elm_app_compile_data_dir_set(PACKAGE_DATA_DIR);
   elm_app_desktop_entry_set("efm.desktop");
   elm_app_info_set(elm_main, "jesus", "");

   // init external elementary stuff
   elm_ext_init();

   // init config
   config_init();

   // init clipboard
   clipboard_init();

   // we need ethumb and efreet
   elm_need_ethumb();
   elm_need_efreet();

   // init ui and stuff
   ui_init();

   // init the hooks
   hooks_init();

   // init history
   history_init();

   fs_operations_init();

   // set the correct path
   eo_do(preview, efl_file_set(path, NULL));
   titlebar_path_set(path);

   elm_run();

   clipboard_shutdown();
   config_shutdown();

   return 0;
}
ELM_MAIN()
