#define EFL_BETA_API_SUPPORT
#define EFL_EO_API_SUPPORT

#include <Eo.h>
#include <Evas.h>
#include <Elementary.h>

#include <Efm.h>
#include "Elementary_Ext.h"
#include "elm_file_icon.eo.h"

typedef struct
{
   EFM_File *file;
   Evas_Object *icon;
   Evas_Object *label;

   Eina_Bool picmode;
   Ecore_Timer *t;
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

   if (!elm_layout_theme_set(obj, "file_icon", "base", "default"))
     {
        CRI("Failed to set theme file\n");
     }

   pd->label = elm_label_add(obj);
   elm_object_text_set(pd->label, "...");
   evas_object_show(pd->label);

   elm_object_part_content_set(obj, "text", pd->label);
}

static void
mime_type_resize(Eo *obj EINA_UNUSED, Elm_File_Icon_Data *pd, int w, int h)
{
   const char *theme, *file, *mime_type;;

   mime_type = efm_file_mimetype_get(pd->file);

   if (!efm_file_mimetype_get(pd->file))
     return;
   if (pd->picmode)
    return;

   eo_do(ELM_FILE_ICON_CLASS, theme = elm_obj_file_icon_util_icon_theme_get());

   file = efreet_mime_type_icon_get(mime_type, theme, (w > h) ? h : w);

   if (!file)
     ERR("Failed to fetch icon for mime type %s\n", mime_type);
   else
    elm_image_file_set(pd->icon, file, NULL);

   evas_object_show(pd->icon);
}

EOLIAN static const char *
_elm_file_icon_util_icon_theme_get(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED)
{
   const char *theme;

   theme = getenv("E_ICON_THEME");
   if (!theme)
     theme = "hicolor";

   return strdup(theme);
}


static void
mime_ready(Eo *obj, Elm_File_Icon_Data *pd)
{
  int w, h, x, y;

  evas_object_geometry_get(obj, &x, &y, &w, &h);

  mime_type_resize(obj, pd, w, h);
}

EOLIAN static void
_elm_file_icon_evas_object_smart_resize(Eo *obj, Elm_File_Icon_Data *pd, Evas_Coord w, Evas_Coord h)
{
   eo_do_super(obj, ELM_FILE_ICON_CLASS, evas_obj_smart_resize(w,h));
   mime_type_resize(obj, pd, w, h);
}

EOLIAN static void
_elm_file_icon_fm_monitor_file_set(Eo *obj, Elm_File_Icon_Data *pd, EFM_File *file)
{

   const char *path;

   elm_drop_target_del(obj, ELM_SEL_FORMAT_TARGETS, _enter_cb, obj,_leave_cb, NULL, NULL, NULL, _drop_cb, NULL);
   pd->file = file;
   path = efm_file_path_get(pd->file);

   if (efm_file_is_dir(pd->file))
     {
        //add dnd
        elm_drop_target_add(obj, ELM_SEL_FORMAT_TARGETS, _enter_cb, obj,_leave_cb, NULL, NULL, NULL, _drop_cb, NULL);
     }
   else
     {
        //check if this file can be loaded
        if (evas_object_image_extension_can_load_fast_get(path))
          pd->picmode = EINA_TRUE;
        else
          pd->picmode = EINA_FALSE;
      }

   //delete existing partwidgets
   if (pd->icon)
     evas_object_del(pd->icon);

   //create new display icons
   if (pd->picmode)
     {
        pd->icon = elm_thumb_add(obj);
        eo_do(pd->icon, efl_file_set(efm_file_path_get(file), NULL));
     }
   else
     pd->icon = elm_icon_add(obj);

   //set the new conecnt
   _content_set(obj, pd->icon);

   //if the mime type is allready set FIXME fix it
   if (!efm_file_mimetype_get(pd->file))
     {}//TODO listen for ready event - need to wait until efm is done with eo ...
   else
     mime_ready(obj, pd);

   //set the text of the filename
   elm_object_text_set(pd->label, efm_file_filename_get(file));
}

EOLIAN static EFM_File *
_elm_file_icon_fm_monitor_file_get(Eo *obj EINA_UNUSED, Elm_File_Icon_Data *pd)
{
   return pd->file;
}

EOLIAN static void
_elm_file_icon_fill_sample(Eo *obj EINA_UNUSED, Elm_File_Icon_Data *pd, const char **group, const char **file)
{
   const char *theme;

   eo_do(ELM_FILE_ICON_CLASS, theme = elm_obj_file_icon_util_icon_theme_get());

   *group = NULL;
   *file = efreet_mime_type_icon_get(efm_file_mimetype_get(pd->file), theme, 8);
}

#include "elm_file_icon.eo.x"
