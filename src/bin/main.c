#include "main.h"

Evas_Object *win;
Evas_Object *preview;
Evas_Object *selector;
Evas_Object *layout;

static void
on_done(void *data EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   evas_object_del(obj);
   // quit the mainloop (elm_run function will return)
   elm_exit();
}

void
printHelp()
{
   printf("usage: verne [Path] \n If path is not given $HOME will be taken, if this is not set / will be used.");
}

static void
_dir_changed(void *data EINA_UNUSED, const Efl_Event *event)
{
   Eina_Strbuf *buf;
   const char *filename;

   buf = eina_strbuf_new();
   filename = efm_file_filename_get(event->info);
   titlebar_path_set(efm_file_path_get(event->info));

   eina_strbuf_append_printf(buf, "elm - Verne | %s", filename);
   elm_win_title_set(win, eina_strbuf_string_get(buf));
   eina_strbuf_free(buf);
}

static void
ui_init()
{
   Evas_Object *icon, *seperator, *box;
   Evas_Image *img;

   win = elm_win_util_standard_add("Verne", "Verne - Fm");

   icon = elm_icon_add(win);
   elm_icon_standard_set(icon, "verne");
   evas_object_show(icon);
   img = elm_image_object_get(icon);
   elm_win_icon_object_set(win, img);

   evas_object_smart_callback_add(win, "delete,request", on_done, NULL);

   box = elm_box_add(win);
   evas_object_size_hint_align_set(box, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(box);


   layout = elm_layout_add(win);
   efl_file_set(layout, THEME_PATH"/default.edc.edj", "headbar");
   evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, 0.0);
   evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(box, layout);
   evas_object_show(layout);

   seperator = elm_separator_add(win);
   elm_separator_horizontal_set(seperator, EINA_TRUE);
   evas_object_size_hint_align_set(seperator, EVAS_HINT_FILL, 0.0);
   evas_object_size_hint_weight_set(seperator, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(box, seperator);
   evas_object_show(seperator);

   preview = efl_add(ELM_FILE_DISPLAY_CLASS, win);
   evas_object_size_hint_align_set(preview, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(preview, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_pack_end(box, preview);

   selector = elm_file_display_selector_get(preview);
   efl_event_callback_add(selector, ELM_FILE_SELECTOR_EVENT_PATH_CHANGED,
                                 _dir_changed, NULL);
   evas_object_show(preview);


   elm_object_focus_set(preview, EINA_TRUE);

   titlebar_init();

   shortcuts_init();

   elm_win_resize_object_add(win, box);
   evas_object_resize(win, 800,600);
   evas_object_show(win);
}

int
elm_main(int argc EINA_UNUSED, char **argv EINA_UNUSED)
{
   char *path = NULL;
   Efm_File *file;
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

   efm_init();
   file = efm_file_get(EFM_CLASS, path);

   if (!file)
     file = efm_file_get(EFM_CLASS, "/");
   if (!file)
     printf("WAD\n");

   // set app informations
   elm_app_name_set("Verne");
   elm_app_compile_bin_dir_set(PACKAGE_BIN_DIR);
   elm_app_compile_lib_dir_set(PACKAGE_LIB_DIR);
   elm_app_compile_data_dir_set(PACKAGE_DATA_DIR);
   elm_app_desktop_entry_set("verne.desktop");
   elm_app_info_set(elm_main, "verne", "");

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
   elm_file_selector_file_set(selector, file);
   titlebar_path_set(path);

   elm_run();

   clipboard_shutdown();
   config_shutdown();

   efm_shutdown();
   return 0;
}
ELM_MAIN()
