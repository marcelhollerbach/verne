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

int
sort_func(const void *data1, const void *data2)
{
    //XXX remove
    return -1;
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
   char buf[PATH_MAX];
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