#define EFL_BETA_API_SUPPORT
#define EFL_EO_API_SUPPORT

#include <Eo.h>
#include <Evas.h>
#include <Elementary.h>

#include "../lib/Efm.h"
#include "elm_file_icon.eo.h"


#define CRI(...) printf(__VA_ARGS__)
#define ERR(...) printf(__VA_ARGS__)

typedef struct
{
   EFM_File *file;
   Evas_Object *icon;
   Evas_Object *label;

   Eina_Bool picmode;
} Elm_File_Icon_Data;

static void
_content_set(Evas_Object *obj, Evas_Object *c)
{
   Evas_Object *oo;

   oo = elm_object_part_content_unset(obj, "content");
   evas_object_hide(oo);

   elm_object_part_content_set(obj, "content", c);
   evas_object_show(c);
}

EOLIAN static void
_elm_file_icon_evas_object_smart_del(Eo *obj, Elm_File_Icon_Data *pd EINA_UNUSED)
{
  eo_do_super(obj, ELM_FILE_ICON_CLASS, evas_obj_smart_del());
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

   theme = getenv("E_ICON_THEME");
   if (!theme)
     theme = "hicolor";

   file = efreet_mime_type_icon_get(mime_type, theme, (w > h) ? h : w);

   if (!file)
     ERR("Failed to fetch icon for mime type %s\n", mime_type);
   else
    elm_image_file_set(pd->icon, file, NULL);

   evas_object_show(pd->icon);
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

   pd->file = file;
   path = efm_file_path_get(pd->file);

   if (!efm_file_is_dir(pd->file) && evas_object_image_extension_can_load_fast_get(path))
     pd->picmode = EINA_TRUE;
   else
     pd->picmode = EINA_FALSE;

   if (pd->icon)
     evas_object_del(pd->icon);

   if (pd->picmode)
     {
        pd->icon = elm_thumb_add(obj);
        eo_do(pd->icon, efl_file_set(efm_file_path_get(file), NULL));
     }
   else
     pd->icon = elm_icon_add(obj);

   _content_set(obj, pd->icon);

   if (efm_file_mimetype_get(pd->file))
     eo_do(obj, elm_obj_file_icon_mime_ready());

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

   theme = getenv("E_ICON_THEME");
   if (!theme)
     theme = "hicolor";
   *group = NULL;
   *file = efreet_mime_type_icon_get(efm_file_mimetype_get(pd->file), theme, 8);
}

EOLIAN static void
_elm_file_icon_mime_ready(Eo *obj, Elm_File_Icon_Data *pd)
{
  int w, h, x, y;

  evas_object_geometry_get(obj, &x, &y, &w, &h);

  mime_type_resize(obj, pd, w, h);
}

#include "elm_file_icon.eo.x"
