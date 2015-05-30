#include "elm_file_display_priv.h"


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
      eo_do(ic, elm_obj_file_icon_fm_monitor_file_set(file);
                eo_event_callback_add(ELM_FILE_ICON_EVENT_ITEM_DROP, _drop_cb, widget);
                eo_event_callback_add(ELM_FILE_ICON_EVENT_ITEM_HOVER, _hover_cb, widget));
   #else
      ic = elm_label_add(par);
      elm_object_text_set(ic, file);
   #endif
   evas_object_show(ic);

   return ic;
}

static int
sort_name_func(const void *data1, const void *data2)
{
   Efm_File *f1 = ((Efm_File*)data1), *f2 = ((Efm_File*)data2);
   if (config->sort.type == SORT_TYPE_NAME)
     {
        const char *n1 = efm_file_filename_get(f1);
        const char *n2 = efm_file_filename_get(f2);
        int c = 0;
#if 0
        if (n1[0] == '.')
          n1 ++;
        if (n2[0] == '.')
          n2 ++;
#endif
        while(n1[c] != '\0' && n2[c] != '\0')
          {
             if (n1[c] < n2[c])
               return -1;
             else if (n1[c] > n2[c])
               return 1;
             c ++;
          }
        return 0;
     }
   else if (config->sort.type == SORT_TYPE_SIZE)
     {
       Efm_File_Stat *st1, *st2;

        st1 = efm_file_stat_get(f1);
        st2 = efm_file_stat_get(f2);

        if (st1->mtime > st2->mtime)
          return 1;
        else
          return -1;
     }
   else if (config->sort.type == SORT_TYPE_DATE)
     {
        Efm_File_Stat *st1, *st2;

        st1 = efm_file_stat_get(f1);
        st2 = efm_file_stat_get(f2);

        if (st1->size > st2->size)
          return 1;
        else
          return -1;
     }
   else //if (config->sort.type == SORT_TYPE_EXTENSION)
     {
        return 1;
        //FIXME BLUARB
     }
}

int
sort_func(const void *data1, const void *data2)
{
   Efm_File *f1, *f2;
   int mul;

   f1 = elm_object_item_data_get(data1);
   f2 = elm_object_item_data_get(data2);

   if (config->sort.reverse)
     mul = -1;
   else
     mul = 1;

   if (efm_file_is_type(f1, EFM_FILE_TYPE_DIRECTORY) && efm_file_is_type(f2, EFM_FILE_TYPE_DIRECTORY))
     {
       return sort_name_func(f1, f2) * mul;
     }
   else if (efm_file_is_type(f1, EFM_FILE_TYPE_DIRECTORY) && !efm_file_is_type(f2, EFM_FILE_TYPE_DIRECTORY))
     {
        if (config->sort.folder_placement == FOLDER_FIRST)
          return -1;
        else if (config->sort.folder_placement == FOLDER_LAST)
          return 1;
        else
          return sort_name_func(f1, f2) * mul;
     }
   else if (!efm_file_is_type(f1, EFM_FILE_TYPE_DIRECTORY) && efm_file_is_type(f2, EFM_FILE_TYPE_DIRECTORY))
     {
        if (config->sort.folder_placement == FOLDER_FIRST)
          return 1;
        else if (config->sort.folder_placement == FOLDER_LAST)
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
   char buf[PATH_MAX];
   const char *path, *fileending, *filename;

   path = efm_file_path_get(f);
   fileending = efm_file_fileending_get(f);
   filename = efm_file_filename_get(f);

   eina_stringshare_ref(path);

   /*
    * if it is a standart directory
    * open it
    */
   if (efm_file_is_type(f, EFM_FILE_TYPE_DIRECTORY))
     {
        /*call path changed */
        eo_do(data,
          efl_file_set(path, NULL);
          eo_event_callback_call(&_ELM_FILE_DISPLAY_EVENT_PATH_CHANGED, (void*)path);
        );
        goto end;
     }

   /*
    * if it is a archiv
    * extract it and change the directory
    */
   if (efm_archive_file_supported(fileending))
     {
        /* gen dir */
        snprintf(buf, sizeof(buf), "/tmp/%s", filename);
        if (!ecore_file_exists(buf))
          /* extract the file */
          efm_archive_file_extract(path, buf);
        /* set path to the archive */
        eo_do(data,
          efl_file_set(buf, NULL);
          eo_event_callback_call(&_ELM_FILE_DISPLAY_EVENT_PATH_CHANGED, (void*)buf);
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