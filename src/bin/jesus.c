
#include "jesus.h"

static Evas_Object *entry;
static Evas_Object *display;

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
   const char *dir = event;

   elm_object_text_set(entry, NULL);
   elm_entry_entry_append(entry, dir);

   return EINA_FALSE;
}

void
change_path(const char *path)
{
   eo_do(display, efl_file_set(path, NULL));
}

EAPI_MAIN int
elm_main(int argc EINA_UNUSED, char **argv EINA_UNUSED)
{
   Evas_Object *win, *layout;
   const char *path = NULL;

   //check if someone gave us a path
   if (argc > 2)
     {
        printHelp();
     }
   else if (argc > 1)
     {
        path = argv[1];
     }

   //if not check if $HOME is set
   if (!path)
     {
        path = getenv("HOME");
     }

   //if everything fails we just take /
   if (!path)
     {
        path = strdup("/");
     }

   elm_ext_init();

   elm_need_ethumb();
   elm_need_efreet();

   win = elm_win_util_standard_add("efm", "efm - Jesus");
   evas_object_smart_callback_add(win, "delete,request", on_done, NULL);

   layout = elm_layout_add(win);
   eo_do(layout, efl_file_set(THEME_PATH"/efm.edc.edj", "headbar"));
   evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(layout);

   entry = titlebar_add(layout);
   elm_entry_entry_append(entry, strdup(path));
   elm_object_part_content_set(layout, "textbar", entry);
   evas_object_show(entry);

   display = eo_add(ELM_FILE_DISPLAY_CLASS, win);
   evas_object_size_hint_align_set(display, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(display, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_object_part_content_set(layout, "content", display);
   eo_do(display, efl_file_set(path, NULL);
                  eo_event_callback_add(ELM_FILE_DISPLAY_EVENT_PATH_CHANGED,
                                 _dir_changed, NULL););
   evas_object_show(display);

   elm_win_resize_object_add(win, layout);
   evas_object_resize(win, 200,200);
   evas_object_show(win);
   eo_unref(display);
   elm_run();
   return 0;
}
ELM_MAIN()
