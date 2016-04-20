#include "../elementary_ext_priv.h"

typedef struct
{
   Efm_File *file;
   Evas_Object *icon;

   Eina_Bool preview;
   Ecore_Timer *t;
   Elm_File_MimeType_Cache *cache;

   Eina_Bool rename_mode;
} Elm_File_Icon_Data;

#define PRIV_DATA  Elm_File_Icon_Data *pd = eo_data_scope_get(obj, ELM_FILE_ICON_CLASS);

static void
_content_set(Evas_Object *obj, Evas_Object *c)
{
   Evas_Object *oo;

   oo = elm_object_part_content_unset(obj, "content");
   evas_object_hide(oo);

   elm_object_part_content_set(obj, "content", c);
   evas_object_show(c);
}

static Eina_Bool
_long_cb(void *data)
{
   Evas_Object *obj = data;
   PRIV_DATA

   ecore_timer_del(pd->t);
   pd->t = NULL;
   eo_event_callback_call(obj, ELM_FILE_ICON_EVENT_ITEM_HOVER, NULL);

   return EINA_FALSE;
}

EOLIAN static void
_elm_file_icon_evas_object_smart_del(Eo *obj, Elm_File_Icon_Data *pd EINA_UNUSED)
{
  if (pd->t)
    ecore_timer_del(pd->t);
  pd->t = NULL;
  evas_obj_smart_del(eo_super(obj, ELM_FILE_ICON_CLASS));
}

static void
_enter_cb(void *data EINA_UNUSED, Evas_Object *obj)
{
   PRIV_DATA

   pd->t = ecore_timer_add(1.0, _long_cb, obj);
   elm_icon_standard_set(pd->icon, "folder-drag-accept");
}

static void
_leave_cb(void *data EINA_UNUSED, Evas_Object *obj)
{
   PRIV_DATA

   if (pd->t)
     ecore_timer_del(pd->t);
   pd->t = NULL;
   elm_icon_standard_set(pd->icon, "folder");
}

static Eina_Bool
_drop_cb(void *data EINA_UNUSED, Evas_Object *obj, Elm_Selection_Data *ev)
{
   PRIV_DATA

   eo_event_callback_call(obj, ELM_FILE_ICON_EVENT_ITEM_DROP, ev);

   if (pd->t)
     ecore_timer_del(pd->t);
   pd->t = NULL;

   elm_icon_standard_set(pd->icon, "folder");
   return EINA_FALSE;
}

EOLIAN static void
_elm_file_icon_evas_object_smart_add(Eo *obj, Elm_File_Icon_Data *pd)
{
   evas_obj_smart_add(eo_super(obj, ELM_FILE_ICON_CLASS));

   if (!elm_layout_theme_set(obj, "file_icon", "base", elm_object_style_get(obj)))
     {
        CRI("Failed to set theme file\n");
     }
}

static Eina_Bool
_key_down_cb(void *data, const Eo_Event *event)
{
   Evas_Event_Key_Down *ev;

   ev = event->info;

   if (!strcmp(ev->key, "Escape"))
     elm_obj_file_icon_rename_set(data, EINA_FALSE, EINA_FALSE);
   else if (!strcmp(ev->key, "Return"))
     elm_obj_file_icon_rename_set(data, EINA_FALSE, EINA_TRUE);

   return EO_CALLBACK_CONTINUE;
}

EOLIAN static void
_elm_file_icon_rename_set(Eo *obj, Elm_File_Icon_Data *pd, Eina_Bool mode, Eina_Bool take)
{
   if (mode == pd->rename_mode)
     return;

   pd->rename_mode = mode;

   if (mode)
     {
        const char *filename;
        Evas_Object *entry;

        elm_layout_signal_emit(obj, "public,rename,on", "elm");

        filename = efm_file_filename_get(pd->file);

        entry = elm_entry_add(obj);
        eo_event_callback_add(entry, EVAS_OBJECT_EVENT_KEY_DOWN, _key_down_cb, obj);
        evas_object_propagate_events_set(entry, EINA_FALSE);
        elm_entry_scrollable_set(entry, EINA_TRUE);
        elm_scroller_policy_set(entry, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
        elm_entry_single_line_set(entry, EINA_TRUE);
        elm_entry_editable_set(entry, EINA_TRUE);
        elm_entry_entry_set(entry, filename);
        eo_event_callback_call(obj, ELM_FILE_ICON_EVENT_RENAME_START, NULL);
        elm_object_part_content_set(obj, "public.rename", entry);
        evas_object_show(entry);
        elm_object_focus_set(entry, EINA_TRUE);
     }
   else
     {
        const char *filename;
        Evas_Object *entry;
        const char *nname;

        elm_layout_signal_emit(obj, "public,rename,off", "elm");

        filename = efm_file_filename_get(pd->file);

        entry = elm_object_part_content_unset(obj, "public.rename");
        nname = elm_object_text_get(entry);

        if (take)
          eo_event_callback_call(obj, ELM_FILE_ICON_EVENT_RENAME_DONE, (char*)nname);
        elm_object_part_text_set(obj, "public.text", filename);
        evas_object_del(entry);
     }
}

EOLIAN static Eina_Bool
_elm_file_icon_rename_get(Eo *obj EINA_UNUSED, Elm_File_Icon_Data *pd)
{
  return pd->rename_mode;
}

EOLIAN static const char *
_elm_file_icon_util_icon_theme_get(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED)
{
   const char *theme;

   theme = getenv("E_ICON_THEME");

   if (!theme)
     theme = "hicolor";

   if (!efreet_icon_theme_find(theme))
     ERR("Failed to find usefull theme");

   return theme;
}

static void
mime_ready(Eo *obj EINA_UNUSED, Elm_File_Icon_Data *pd)
{
   const char *file, *mime_type;;

   mime_type = efm_file_mimetype_get(pd->file);

   if (!mime_type)
     return;

   if (efm_file_is_type(pd->file, EFM_FILE_TYPE_DIRECTORY))
     elm_icon_standard_set(pd->icon, "folder");
   else
     {
        file = elm_file_mimetype_cache_mimetype_get(pd->cache, mime_type);
        if (!file)
          INF("Failed to fetch icon for mime type %s\n", mime_type);
        else
          elm_image_file_set(pd->icon, file, NULL);
     }

   evas_object_show(pd->icon);
}

EOLIAN static void
_elm_file_icon_evas_object_smart_resize(Eo *obj, Elm_File_Icon_Data *pd EINA_UNUSED, Evas_Coord w, Evas_Coord h)
{
   evas_obj_smart_resize(eo_super(obj, ELM_FILE_ICON_CLASS), w, h);
}

static Eina_Bool
_mime_ready(void *data, const Eo_Event *event EINA_UNUSED)
{
   Eo *icon = data;
   Elm_File_Icon_Data *pd;

   pd = eo_data_scope_get(icon, ELM_FILE_ICON_CLASS);
   mime_ready(icon, pd);
   return EINA_TRUE;
}

typedef enum {
  FILE_MODE_IMAGE, FILE_MODE_DESKTOP, FILE_MODE_TRIVIAL
} File_Mode;

static inline void
_file_set(Eo *obj, Elm_File_Icon_Data *pd, Efm_File *file)
{
   Eina_Bool dir;
   File_Mode filemode = FILE_MODE_TRIVIAL;
   const char *path, *mime_type, *filename, *fileextension;

   eo_wref_add(file, &pd->file);
   path = efm_file_path_get(pd->file);
   dir = efm_file_is_type(pd->file, EFM_FILE_TYPE_DIRECTORY);
   mime_type = efm_file_mimetype_get(pd->file);
   filename = efm_file_filename_get(pd->file);
   fileextension = efm_file_fileending_get(pd->file);

   if (dir)
     {
        // add dnd
        elm_drop_target_add(obj, ELM_SEL_FORMAT_TARGETS, _enter_cb, obj,_leave_cb, NULL, NULL, NULL, _drop_cb, NULL);
     }

   if (!dir && pd->preview)
     {
        //check if this file can be loaded
        if (evas_object_image_extension_can_load_fast_get(path))
          filemode = FILE_MODE_IMAGE;
        else if (fileextension && !strcmp(fileextension, "desktop"))
          filemode = FILE_MODE_DESKTOP;
        else
          filemode = FILE_MODE_TRIVIAL;
      }

   // create new display icons
   if (filemode == FILE_MODE_IMAGE)
     {
        pd->icon = elm_thumb_add(obj);
        efl_file_set(pd->icon, path, NULL);
     }
   else if (filemode == FILE_MODE_DESKTOP)
     {
        Efreet_Desktop *desktop;

        desktop = efreet_desktop_new(path);

        pd->icon = elm_icon_add(obj);
        elm_icon_order_lookup_set(pd->icon, ELM_ICON_LOOKUP_FDO_THEME);
        elm_icon_standard_set(pd->icon, desktop->icon);
     }
   else if (filemode == FILE_MODE_TRIVIAL)
     {
        pd->icon = elm_icon_add(obj);
        elm_icon_order_lookup_set(pd->icon, ELM_ICON_LOOKUP_FDO_THEME);

        if (!mime_type)
          eo_event_callback_add(pd->file, EFM_FILE_EVENT_FSQUERY_DONE,
                          _mime_ready, obj);
        else
          mime_ready(obj, pd);
     }

   // set the new conecnt
   _content_set(obj, pd->icon);

   // set the text of the filename
   elm_object_part_text_set(obj, "public.text", filename);
}

EOLIAN static Efm_File *
_elm_file_icon_file_get(Eo *obj EINA_UNUSED, Elm_File_Icon_Data *pd)
{
   return pd->file;
}
EOLIAN static void
_elm_file_icon_install(Eo *obj, Elm_File_Icon_Data *pd, Elm_File_MimeType_Cache *cache, Efm_File *file, Eina_Bool preview)
{
   EINA_SAFETY_ON_NULL_RETURN(cache);
   EINA_SAFETY_ON_NULL_RETURN(file);

   pd->cache = cache;
   pd->preview = preview;
   _file_set(obj, pd, file);
}
EOLIAN static Eo_Base*
_elm_file_icon_eo_base_finalize(Eo *obj, Elm_File_Icon_Data *pd)
{
   Eo *res;

   res = eo_finalize(eo_super(obj, ELM_FILE_ICON_CLASS));

   EINA_SAFETY_ON_NULL_RETURN_VAL(pd->cache, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pd->file, NULL);

   return res;
}

static void
_elm_file_icon_eo_base_destructor(Eo *obj, Elm_File_Icon_Data *pd)
{
   eo_event_callback_del(pd->file, EFM_FILE_EVENT_FSQUERY_DONE, _mime_ready, obj);
   eo_wref_del(pd->file, &pd->file);

   eo_destructor(eo_super(obj, ELM_FILE_ICON_CLASS));
}

#include "elm_file_icon.eo.x"
