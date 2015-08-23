#define EFL_BETA_API_SUPPORT
#define EFL_EO_API_SUPPORT

#include <Eo.h>
#include <Evas.h>
#include <Elementary.h>
#include <Efm.h>

#include "../elementary_ext_priv.h"

typedef struct
{
   Efm_File *file;
   Evas_Object *icon;
   Evas_Object *label;
   Evas_Object *entry;

   Eina_Bool picmode;
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
   eo_do(obj, eo_event_callback_call(ELM_FILE_ICON_EVENT_ITEM_HOVER, NULL));
   pd->t = NULL;

   return EINA_FALSE;
}

EOLIAN static void
_elm_file_icon_evas_object_smart_del(Eo *obj, Elm_File_Icon_Data *pd EINA_UNUSED)
{
  if (pd->t)
    ecore_timer_del(pd->t);
  pd->t = NULL;
  eo_do_super(obj, ELM_FILE_ICON_CLASS, evas_obj_smart_del());
}

static void
_enter_cb(void *data EINA_UNUSED, Evas_Object *obj)
{
   PRIV_DATA

   pd->t = ecore_timer_add(1.0, _long_cb, obj);
   elm_layout_signal_emit(obj, "file_icon,mode,drop", "elm");
}

static void
_leave_cb(void *data EINA_UNUSED, Evas_Object *obj)
{
   PRIV_DATA

   if (pd->t)
     ecore_timer_del(pd->t);
   pd->t = NULL;
   elm_layout_signal_emit(obj, "file_icon,mode,display", "elm");
}

static Eina_Bool
_drop_cb(void *data EINA_UNUSED, Evas_Object *obj, Elm_Selection_Data *ev)
{
   eo_do(obj, eo_event_callback_call(ELM_FILE_ICON_EVENT_ITEM_DROP, ev));
   elm_layout_signal_emit(obj, "file_icon,mode,display", "elm");
   return EINA_FALSE;
}

EOLIAN static void
_elm_file_icon_evas_object_smart_add(Eo *obj, Elm_File_Icon_Data *pd)
{
   eo_do_super(obj, ELM_FILE_ICON_CLASS, evas_obj_smart_add());

   if (!elm_layout_theme_set(obj, "file_icon", "base", elm_object_style_get(obj)))
     {
        CRI("Failed to set theme file\n");
     }

   pd->label = elm_label_add(obj);
   elm_object_text_set(pd->label, "...");
   evas_object_show(pd->label);
   elm_object_part_content_set(obj, "text", pd->label);
}

static Eina_Bool
_key_down_cb(void *data, Eo *obj EINA_UNUSED, const Eo_Event_Description2 *desc EINA_UNUSED, void *event EINA_UNUSED)
{
   Evas_Event_Key_Down *ev;

   ev = event;

   if (!strcmp(ev->key, "Escape"))
     eo_do(data, elm_obj_file_icon_rename_set(EINA_FALSE, EINA_FALSE));
   else if (!strcmp(ev->key, "Return"))
     eo_do(data, elm_obj_file_icon_rename_set(EINA_FALSE, EINA_TRUE));

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

        eo_do(pd->file, filename = efm_file_filename_get());

        pd->entry = elm_entry_add(obj);
        eo_do(pd->entry, eo_event_callback_add(EVAS_OBJECT_EVENT_KEY_DOWN, _key_down_cb, obj));
        evas_object_propagate_events_set(pd->entry, EINA_FALSE);
        elm_entry_scrollable_set(pd->entry, EINA_TRUE);
        elm_scroller_policy_set(pd->entry, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
        elm_entry_single_line_set(pd->entry, EINA_TRUE);
        elm_entry_editable_set(pd->entry, EINA_TRUE);
        elm_entry_entry_set(pd->entry, filename);
        eo_do(obj, eo_event_callback_call(ELM_FILE_ICON_EVENT_RENAME_START, NULL));
        elm_object_part_content_unset(obj, "text");
        elm_object_part_content_set(obj, "text", pd->entry);
        evas_object_hide(pd->label);
     }
   else
     {
        const char *nname;

        nname = elm_entry_entry_get(pd->entry);
        if (take)
          eo_do(obj, eo_event_callback_call(ELM_FILE_ICON_EVENT_RENAME_DONE, (char*)nname));
        evas_object_del(pd->entry);
        pd->entry = NULL;

        elm_object_part_content_unset(obj, "text");
        elm_object_part_content_set(obj, "text", pd->label);
        evas_object_show(pd->label);
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
   Eina_Bool dir;

   if (!pd->file)
     return;

   eo_do(pd->file, mime_type = efm_file_mimetype_get());

   if (!mime_type)
     return;

   if (pd->picmode)
    return;

   if (!pd->cache)
     {
        ERR("A cache needs to be set at first");
        return;
     }
   if (eo_do_ret(pd->file, dir, efm_file_is_type(EFM_FILE_TYPE_DIRECTORY)))
     elm_icon_standard_set(pd->icon, "folder");
   else
     {
        eo_do(pd->cache, file = elm_file_mimetype_cache_mimetype_get(mime_type));
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
   eo_do_super(obj, ELM_FILE_ICON_CLASS, evas_obj_smart_resize(w,h));
}

static Eina_Bool
_mime_ready(void *data EINA_UNUSED, Eo *obj EINA_UNUSED, const Eo_Event_Description *desc EINA_UNUSED, void *event EINA_UNUSED)
{
   Eo *icon = data;
   Elm_File_Icon_Data *pd;

   pd = eo_data_scope_get(icon, ELM_FILE_ICON_CLASS);
   mime_ready(icon, pd);
   return EINA_TRUE;
}

static inline void
_file_set(Eo *obj, Elm_File_Icon_Data *pd, Efm_File *file)
{
   Eina_Bool dir;
   const char *path, *mime_type, *filename;

   if (pd->file)
     {
        eo_do(pd->file, eo_wref_del(&pd->file);
                        eo_event_callback_del(EFM_FILE_EVENT_FSQUERY_DONE, _mime_ready, obj));
     }
   eo_do(file, eo_wref_add(&pd->file));

   elm_drop_target_del(obj, ELM_SEL_FORMAT_TARGETS, _enter_cb, obj,_leave_cb, NULL, NULL, NULL, _drop_cb, NULL);
   eo_do(pd->file, path = efm_file_path_get();
                   dir = efm_file_is_type(EFM_FILE_TYPE_DIRECTORY);
                   mime_type = efm_file_mimetype_get();
                   filename = efm_file_filename_get());

   if (dir)
     {
        // add dnd
        elm_drop_target_add(obj, ELM_SEL_FORMAT_TARGETS, _enter_cb, obj,_leave_cb, NULL, NULL, NULL, _drop_cb, NULL);
     }
   else
     {
        //check if this file can be loaded
        //if (evas_object_image_extension_can_load_fast_get(path))
        //  pd->picmode = EINA_TRUE;
        //else
          pd->picmode = EINA_FALSE;
      }

   // delete existing partwidgets
   if (pd->icon)
     evas_object_del(pd->icon);

   // create new display icons
   if (pd->picmode)
     {
        pd->icon = elm_thumb_add(obj);
        eo_do(pd->icon, efl_file_set(path, NULL));
     }
   else
     {
        pd->icon = elm_icon_add(obj);
        elm_icon_order_lookup_set(pd->icon, ELM_ICON_LOOKUP_FDO);
     }

   // set the new conecnt
   _content_set(obj, pd->icon);

   // if the mime type is allready set FIXME fix it

    if (!mime_type)
      eo_do(pd->file, eo_event_callback_add(EFM_FILE_EVENT_FSQUERY_DONE,
                      _mime_ready, obj));
    else
      mime_ready(obj, pd);
   // set the text of the filename
   elm_object_text_set(pd->label, filename);
   elm_object_tooltip_text_set(pd->label, filename);
}

EOLIAN static Efm_File *
_elm_file_icon_file_get(Eo *obj EINA_UNUSED, Elm_File_Icon_Data *pd)
{
   return pd->file;
}
EOLIAN static void
_elm_file_icon_install(Eo *obj, Elm_File_Icon_Data *pd, Elm_File_MimeType_Cache *cache, Efm_File *file)
{
   pd->cache = cache;
   _file_set(obj, pd, file);
}
EOLIAN static Eo_Base*
_elm_file_icon_eo_base_finalize(Eo *obj, Elm_File_Icon_Data *pd)
{
   Eo *res;

   eo_do_super(obj, ELM_FILE_ICON_CLASS, res = eo_finalize());

   EINA_SAFETY_ON_NULL_RETURN_VAL(pd->cache, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pd->file, NULL);

   return res;
}

static void
_elm_file_icon_eo_base_destructor(Eo *obj, Elm_File_Icon_Data *pd)
{
   if (pd->file)
     eo_do(pd->file, eo_event_callback_del(EFM_FILE_EVENT_FSQUERY_DONE, _mime_ready, obj));
   eo_do(pd->file, eo_wref_del(&pd->file));
   eo_do_super(obj, ELM_FILE_ICON_CLASS, eo_destructor());
}

#include "elm_file_icon.eo.x"
