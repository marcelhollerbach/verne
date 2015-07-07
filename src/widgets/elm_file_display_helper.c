#include "elm_file_display_priv.h"

Elm_File_MimeType_Cache *cache;

static Eina_Bool
_drop_cb(void *data, Eo *obj EINA_UNUSED, const Eo_Event_Description *desc EINA_UNUSED, void *event EINA_UNUSED)
{
 eo_do(data,
         eo_event_callback_call(ELM_FILE_DISPLAY_EVENT_DND_ITEM_DROPED, NULL));
   return EINA_FALSE;
}

static Eina_Bool
_hover_cb(void *data, Eo *obj EINA_UNUSED, const Eo_Event_Description *desc EINA_UNUSED, void *event EINA_UNUSED)
{
   eo_do(data,
         eo_event_callback_call(ELM_FILE_DISPLAY_EVENT_DND_ITEM_HOVER, NULL));
   return EINA_FALSE;
}

Evas_Object*
icon_create(Evas_Object *par, Efm_File *file)
{
   Evas_Object *ic, *widget;

   widget = elm_object_parent_widget_get(par);

   #if 1
      ic = eo_add(ELM_FILE_ICON_CLASS, par);
      eo_do(ic,
        elm_obj_file_icon_mimetype_cache_set(cache);
        elm_obj_file_icon_fm_monitor_file_set(file);
        eo_event_callback_add(ELM_FILE_ICON_EVENT_ITEM_DROP, _drop_cb, widget);
        eo_event_callback_add(ELM_FILE_ICON_EVENT_ITEM_HOVER, _hover_cb, widget)
      );
   #else
      const char *name;
      name = efm_file_path_get(file);
      ic = elm_label_add(par);
      elm_object_text_set(ic, name);
   #endif
   evas_object_show(ic);

   return ic;
}

static int
_alphabetic_sort(const char *n1, const char *n2)
{
   int c = 0;


   while(n1[c] != '\0' && n2[c] != '\0')
     {
        if (tolower(n1[c]) < tolower(n2[c]))
          return -1;
        else if (tolower(n1[c]) > tolower(n2[c]))
          return 1;
        c ++;
     }
  return 0;
}

static int
_file_name_sort(Efm_File *f1, Efm_File *f2)
{
   const char *n1;
   const char *n2;
   eo_do(f1, n1 = efm_file_filename_get());
   eo_do(f2, n2 = efm_file_filename_get());

   if (n1[0] == '.')
     n1 ++;
   if (n2[0] == '.')
     n2 ++;
   return _alphabetic_sort(n1, n2);
}

static int
sort_name_func(const void *data1, const void *data2)
{
   Efm_File *f1 = ((Efm_File*)data1), *f2 = ((Efm_File*)data2);
   if (config->sort.type == ELM_FILE_DISPLAY_SORT_TYPE_NAME)
     {
        return _file_name_sort(f1, f2);
     }
   else if (config->sort.type == ELM_FILE_DISPLAY_SORT_TYPE_SIZE)
     {
       Efm_File_Stat *st1, *st2;

        eo_do(f1, st1 = efm_file_stat_get());
        eo_do(f2, st2 = efm_file_stat_get());

        if (st1->size > st2->size)
          return 1;
        else
          return -1;
     }
   else if (config->sort.type == ELM_FILE_DISPLAY_SORT_TYPE_DATE)
     {
        Efm_File_Stat *st1, *st2;

        eo_do(f1, st1 = efm_file_stat_get());
        eo_do(f2, st2 = efm_file_stat_get());

        if (st1->size > st2->size)
          return 1;
        else
          return -1;
     }
   else //if (config->sort.type == SORT_TYPE_EXTENSION)
     {
        const char *ext1;
        const char *ext2;

        eo_do(f1, ext1 = efm_file_fileending_get());
        eo_do(f2, ext2 = efm_file_fileending_get());
        if (!ext1 && !ext2)
          return _file_name_sort(f1, f2);
        else if (!ext1 && ext2)
          return -1;
        else if (ext1 && !ext2)
          return 1;

        int sort = _alphabetic_sort(ext1, ext2);
        if (sort == 0)
          return _file_name_sort(f1, f2);
        else
          return sort;
     }
}

int
sort_func(const void *data1, const void *data2)
{
   Eina_Bool is;
   Efm_File *f1, *f2;
   int mul;

   f1 = elm_object_item_data_get(data1);
   f2 = elm_object_item_data_get(data2);

   if (config->sort.reverse)
     mul = -1;
   else
     mul = 1;

   if (eo_do_ret(f1, is, efm_file_is_type(EFM_FILE_TYPE_DIRECTORY)) &&
       eo_do_ret(f2, is, efm_file_is_type(EFM_FILE_TYPE_DIRECTORY)))
     {
       return sort_name_func(f1, f2) * mul;
     }
   else if (eo_do_ret(f1, is, efm_file_is_type(EFM_FILE_TYPE_DIRECTORY)) &&
            !eo_do_ret(f2, is, efm_file_is_type(EFM_FILE_TYPE_DIRECTORY)))
     {
        if (config->sort.folder_placement == ELM_FILE_DISPLAY_FOLDER_PLACEMENT_FIRST)
          return -1;
        else if (config->sort.folder_placement == ELM_FILE_DISPLAY_FOLDER_PLACEMENT_LAST)
          return 1;
        else
          return sort_name_func(f1, f2) * mul;
     }
   else if (!eo_do_ret(f1, is, efm_file_is_type(EFM_FILE_TYPE_DIRECTORY)) &&
            eo_do_ret(f2, is, efm_file_is_type(EFM_FILE_TYPE_DIRECTORY)))
     {
        if (config->sort.folder_placement == ELM_FILE_DISPLAY_FOLDER_PLACEMENT_FIRST)
          return 1;
        else if (config->sort.folder_placement == ELM_FILE_DISPLAY_FOLDER_PLACEMENT_LAST)
          return -1;
        else
          return sort_name_func(f1, f2)* mul;
     }
   else
     {
        return sort_name_func(f1, f2) * mul;
     }
}

Eina_Bool
_util_item_select_simple(void *data, Eo *obj EINA_UNUSED, const Eo_Event_Description *desc EINA_UNUSED, void *event)
{
   Efm_File *f = event;
   Elm_File_Display_Data *pd = eo_data_scope_get(data, ELM_FILE_DISPLAY_CLASS);

   filepreview_file_set(pd->preview, f);
   return EINA_TRUE;
}

Eina_Bool
_util_item_select_choosen(void *data, Eo *obj EINA_UNUSED, const Eo_Event_Description *desc EINA_UNUSED, void *event)
{
   Efm_File *f = event;
   Eina_Bool is;
   const char *path, *fileending, *filename;

   eo_do(f, path = efm_file_path_get();
            filename = efm_file_filename_get();
            fileending = efm_file_fileending_get()
         );

   eina_stringshare_ref(path);

   /*
    * if it is a standart directory
    * open it
    */
   if (eo_do_ret(f, is, efm_file_is_type(EFM_FILE_TYPE_DIRECTORY)))
     {
        /*call path changed */
        eo_do(data,
          efl_file_set(path, NULL);
          eo_event_callback_call(&_ELM_FILE_DISPLAY_EVENT_PATH_CHANGED, (void*)path);
        );
        goto end;
     }

   /*
    * if it is nothing of all choose
    * event should be called
    */
   eo_do(data,
         eo_event_callback_call(&_ELM_FILE_DISPLAY_EVENT_ITEM_CHOOSEN, f);
   );

end:
   eina_stringshare_del(path);
   return EINA_TRUE;
}