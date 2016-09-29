#include "../elementary_ext_priv.h"

typedef struct
{
   Efm_File *file;
   Evas_Object *icon;

   Eina_Bool preview;
   Ecore_Timer *t;
   Elm_File_MimeType_Cache *cache;

} Elm_File_Icon_Data;

#define PRIV_DATA  Elm_File_Icon_Data *pd = efl_data_scope_get(obj, ELM_FILE_ICON_CLASS);

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
   efl_event_callback_call(obj, ELM_FILE_ICON_EVENT_ITEM_HOVER, NULL);

   return EINA_FALSE;
}

EOLIAN static void
_elm_file_icon_efl_canvas_group_group_del(Eo *obj, Elm_File_Icon_Data *pd EINA_UNUSED)
{
  if (pd->t)
    ecore_timer_del(pd->t);
  pd->t = NULL;
  efl_canvas_group_del(efl_super(obj, ELM_FILE_ICON_CLASS));
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

   efl_event_callback_call(
    obj, ELM_FILE_ICON_EVENT_ITEM_DROP, ev);

   if (pd->t)
     ecore_timer_del(pd->t);
   pd->t = NULL;

   elm_icon_standard_set(pd->icon, "folder");
   return EINA_FALSE;
}

EOLIAN static void
_elm_file_icon_efl_canvas_group_group_add(Eo *obj, Elm_File_Icon_Data *pd EINA_UNUSED)
{
   efl_canvas_group_add(efl_super(obj, ELM_FILE_ICON_CLASS));

   if (!elm_layout_theme_set(obj, "file_icon", "base", elm_object_style_get(obj)))
     {
        CRI("Failed to set theme file\n");
     }
}

static void
mime_ready(Eo *obj EINA_UNUSED, Elm_File_Icon_Data *pd)
{
   const char *mime_type;

   mime_type = efm_file_mimetype_get(pd->file);

   if (!mime_type)
     return;

   if (efm_file_is_type(pd->file, EFM_FILE_TYPE_DIRECTORY))
     elm_file_mimetype_cache_mimetype_set(pd->cache, pd->icon, "folder");
   else
     elm_file_mimetype_cache_mimetype_set(pd->cache, pd->icon, mime_type);

   evas_object_show(pd->icon);
}

typedef enum {
  FILE_MODE_IMAGE, FILE_MODE_DESKTOP, FILE_MODE_TRIVIAL
} File_Mode;

static inline void
_file_set(Eo *obj, Elm_File_Icon_Data *pd)
{
   Eina_Bool dir;
   File_Mode filemode = FILE_MODE_TRIVIAL;
   const char *path, *mime_type, *filename, *fileextension;

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
        elm_object_part_text_set(obj, "public.text", filename);
     }
   else if (filemode == FILE_MODE_DESKTOP)
     {
        Efreet_Desktop *desktop;

        desktop = efreet_desktop_new(path);

        pd->icon = elm_icon_add(obj);
        elm_icon_standard_set(pd->icon, desktop->icon);
        elm_object_part_text_set(obj, "public.text", desktop->name);
     }
   else if (filemode == FILE_MODE_TRIVIAL)
     {
        pd->icon = elm_icon_add(obj);

        mime_ready(obj, pd);
        elm_object_part_text_set(obj, "public.text", filename);
     }

   // set the new conecnt
   _content_set(obj, pd->icon);

   // set the text of the filename
}

EOLIAN static Efl_Object*
_elm_file_icon_efl_object_finalize(Eo *obj, Elm_File_Icon_Data *pd)
{
   Eo *res;

   res = efl_finalize(efl_super(obj, ELM_FILE_ICON_CLASS));

   EINA_SAFETY_ON_NULL_RETURN_VAL(pd->cache, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pd->file, NULL);

   _file_set(obj, pd);

   return res;
}

static void
_elm_file_icon_efl_object_destructor(Eo *obj, Elm_File_Icon_Data *pd)
{
   efl_wref_del(pd->file, &pd->file);

   efl_destructor(efl_super(obj, ELM_FILE_ICON_CLASS));
}


EOLIAN static void
_elm_file_icon_file_set(Eo *obj EINA_UNUSED, Elm_File_Icon_Data *pd, Efm_File *file)
{
    efl_composite_detach(obj, pd->file);
    efl_wref_del(pd->file, &pd->file);
    pd->file = file;
    efl_wref_add(pd->file, &pd->file);

    if (efl_finalized_get(obj))
      _file_set(obj, pd);

    efl_composite_attach(obj, file);
}

EOLIAN static Efm_File *
_elm_file_icon_file_get(Eo *obj EINA_UNUSED, Elm_File_Icon_Data *pd)
{
   return pd->file;
}

EOLIAN static void
_elm_file_icon_cache_set(Eo *obj EINA_UNUSED, Elm_File_Icon_Data *pd, Elm_File_MimeType_Cache *cache)
{
   if (efl_finalized_get(obj)) return;

   pd->cache = cache;
}

EOLIAN static Elm_File_MimeType_Cache *
_elm_file_icon_cache_get(Eo *obj EINA_UNUSED, Elm_File_Icon_Data *pd)
{
   return pd->cache;
}

EOLIAN static void
_elm_file_icon_preview_set(Eo *obj EINA_UNUSED, Elm_File_Icon_Data *pd, Eina_Bool preview)
{
   if (efl_finalized_get(obj)) return;

   pd->preview = preview;
}

EOLIAN static Eina_Bool
_elm_file_icon_preview_get(Eo *obj EINA_UNUSED, Elm_File_Icon_Data *pd)
{
   return pd->preview;
}


#include "elm_file_icon.eo.x"
