#include "main.h"

Evas_Object *win;
Evas_Object *preview;
Evas_Object *selector;
Evas_Object *layout;

int
elm_main(int argc EINA_UNUSED, char **argv EINA_UNUSED)
{
   /*char *path = NULL;
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

   fs_operations_shutdown();

   clipboard_shutdown();
   config_shutdown();

   efm_shutdown();
   return 0;*/
   {
         Evas_Object *win;

         elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

         win = elm_win_util_standard_add("Main", "Hello, World!");
         elm_win_autodel_set(win, EINA_TRUE);
         //win 400x400
         evas_object_resize(win, 400, 400);

         /*basic tutorial code*/

         evas_object_show(win);

         elm_run();
         return 0;
   }
}
ELM_MAIN()
