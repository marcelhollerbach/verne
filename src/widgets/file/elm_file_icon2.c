#define EFL_BETA_API_SUPPORT
#define EFL_EO_API_SUPPORT

#include <Eo.h>
#include "../lib/fm_monitor.h"
#include "elm_file_icon.eo.h"
#include <Evas.h>
#include <Elementary.h>
#include <Eio.h>


typedef struct
{
   Evas_Object *icon, *entry, *thumb, *spinner;

   Eina_Bool editable; //<  file should be editable ?
   Eina_Bool write; //< file is writable ?

   const char *name; //< the file name
   const char *path; //< the complemte path
   const char *mime_icon_file; //< mime type of the file

   Eina_Bool extern_mime_handler; //< a extern hook will set the mime type
   Eina_Bool picmode; //< we are in picmode, so no mimetype is needed

   Eina_Bool rename_in_progress; //< true if a rename is done, the next monitor error should be catched
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

static void
_icon_refresh(Eo *obj, Elm_File_Icon_Data *pd)
{
  int x, y, w, h, min, i, c;
  int sizes[] = {8, 16, 22, 24, 32, 48, 128, 256, 512, 0};
  char *theme;
  const char *file;

  if (pd->picmode) return;


  if (w < h)
    c = w;
  else
    c = h;

  min = c-sizes[0];

  for (i = 1; sizes[i]; i++)
    {
       int val = abs(c - sizes[i]);
       if (val < min)
         min = val;
       else
         {
           i = i -1;
           break;
         }
    }
  /* check if the val got statically smaller
     this is the case when we should get the biggest size */

  if (sizes[i] == 0)
    i = i -1;

  theme = getenv("E_ICON_THEME");
  if (!theme)
    theme = "hicolor";

  file = efreet_mime_type_icon_get(pd->mime_icon_file, theme, sizes[i]);

  if (!file){
    ERR("Failed to fetch icon for mime type %s\n", pd->mime_icon_file);
  }else
    elm_image_file_set(pd->icon, file, NULL);

  ERR("USED MIME TYPE %s\n", pd->mime_icon_file);

  _content_set(obj, pd->icon);
}

EOLIAN static void
_elm_file_icon_evas_object_smart_resize(Eo *obj, Elm_File_Icon_Data *pd, Evas_Coord w, Evas_Coord h)
{
   eo_do_super(obj, ELM_FILE_ICON_CLASS, evas_obj_smart_resize(w, h));

   /* check if the file was valid */
   if (!pd->mime_icon_file) return;

   _icon_refresh(obj, pd);
}

EOLIAN static Eina_Bool
_elm_file_icon_efl_file_file_set(Eo *obj, Elm_File_Icon_Data *pd, const char *file, const char *key EINA_UNUSED)
{
   const char *filename;

   /* call new file event */

   /* set the easy things */
   if (!pd->path)
     pd->path = eina_stringshare_add(file);
   else
     eina_stringshare_replace(&(pd->path), file);

   //FIXME ecore_file_exists is slow
   if (!file || !ecore_file_exists(pd->path))
     {
        evas_object_hide(pd->entry);
        evas_object_hide(pd->icon);
        if (file)
          ERR("File does not exists %s\n", pd->path);
        return EINA_FALSE;
     }
  /* update icon bit */
  filename = ecore_file_file_get(pd->path);
  if (!pd->name)
    pd->name = eina_stringshare_add(filename);
  else
    eina_stringshare_replace(&(pd->name), filename);

  /* check if it is readable */
  //FIXME should we do that here ?
  if (access(file, W_OK) == 0)
    pd->write = EINA_TRUE;
  else
    pd->write = EINA_FALSE;

  /* check the mimetype */
  //check directly with the file extension
  if (evas_object_image_extension_can_load_fast_get(pd->path))
    {
      pd->picmode = EINA_TRUE;
      elm_thumb_file_set(pd->thumb, file, NULL);
      _content_set(obj, pd->thumb);
    }
  else
    {
      pd->picmode = EINA_FALSE;

      if (!pd->extern_mime_handler)
        {
           const char *mime;
           if (ecore_file_is_dir(pd->path))
             mime = "folder";
           else // this is slow, you can do it better with passing a mimetype
             mime = efreet_mime_fallback_type_get(pd->path);
           eo_do(obj, elm_obj_file_icon_mime_set(mime));
        }
      else
        {
           _content_set(obj, pd->spinner);
        }
    }

  /* update text file */
  char buf[PATH_MAX];
  snprintf(buf, sizeof(buf), "<align=0.5>%s", pd->name);
  elm_entry_entry_set(pd->entry, buf);
  return EINA_TRUE;
}

EOLIAN static void
_elm_file_icon_efl_file_file_get(Eo *obj EINA_UNUSED, Elm_File_Icon_Data *pd, const char **file, const char **key)
{
   *file = pd->path;
   *key = NULL;
}

EOLIAN static void
_elm_file_icon_evas_object_smart_del(Eo *obj, Elm_File_Icon_Data *pd EINA_UNUSED)
{
   eo_do_super(obj, ELM_FILE_ICON_CLASS, evas_obj_smart_del());
   //eio_monitor_del(pd->monitor);
}

static void
_cb_double_click(void *data, Evas_Object *obj EINA_UNUSED, void *event EINA_UNUSED)
{
  Eo *o = data;
  Eina_Bool editmode;

  eo_do(o, editmode = elm_obj_file_icon_edit_get());
  eo_do(o, elm_obj_file_icon_edit_set(!editmode));
}

EOLIAN static void
_elm_file_icon_fill_sample(Eo *obj EINA_UNUSED, Elm_File_Icon_Data *pd, const char **group, const char **file)
{
   const char *theme;

   if (pd->picmode)
     {
        *group = "";
        *file = "/usr/local/share/elementary/images/icon_13.png";
     }
   else
     {
        theme = getenv("E_ICON_THEME");
        if (!theme)
          theme = "hicolor";
        *group = NULL;
        *file = efreet_mime_type_icon_get(pd->mime_icon_file, theme, 8);
     }
}

EOLIAN static void
_elm_file_icon_evas_object_smart_add(Eo *obj, Elm_File_Icon_Data *pd)
{
   eo_do_super(obj, ELM_FILE_ICON_CLASS, evas_obj_smart_add());
   //elm_widget_sub_object_parent_add(obj);
   //elm_widget_can_focus_set(obj, EINA_TRUE);
   //evas_object_size_hint_min_set(obj, 60, 60);
   //standart settings;

   if (!elm_layout_theme_set(obj, "file_icon", "base", "default"))
     {
        CRI("Failed to set theme file");
     }

   pd->icon = elm_icon_add(obj);
   //elm_widget_sub_object_add(obj, pd->icon);

   pd->thumb = elm_thumb_add(obj);
   //elm_widget_sub_object_add(obj, pd->thumb);

   pd->spinner = elm_progressbar_add(obj);
   //elm_widget_sub_object_add(obj, pd->spinner);
   elm_object_style_set(pd->spinner, "wheel");
   elm_progressbar_pulse_set(pd->spinner, EINA_TRUE);
   elm_progressbar_pulse(pd->spinner, EINA_TRUE);
   evas_object_size_hint_weight_set(pd->spinner, 0.5, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(pd->spinner, 0.0, EVAS_HINT_FILL);

   pd->entry = elm_entry_add(obj);
   //elm_widget_sub_object_add(obj, pd->entry);
   elm_entry_editable_set(pd->entry, EINA_FALSE);
   elm_entry_single_line_set(pd->entry, EINA_TRUE);
   elm_entry_line_wrap_set(pd->entry, ELM_WRAP_MIXED);
   evas_object_smart_callback_add(pd->entry, "clicked,double", _cb_double_click, obj);
   evas_object_size_hint_weight_set(pd->entry, 0.5, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(pd->entry, 0.0, EVAS_HINT_FILL);
   evas_object_show(pd->entry);

   elm_object_part_content_set(obj, "text", pd->entry);
}

EOLIAN static void
_elm_file_icon_extern_mime_handler_set(Eo *obj EINA_UNUSED, Elm_File_Icon_Data *pd, Eina_Bool edit)
{
   pd->extern_mime_handler = edit;
}

EOLIAN static Eina_Bool
_elm_file_icon_extern_mime_handler_get(Eo *obj EINA_UNUSED, Elm_File_Icon_Data *pd)
{
  return pd->extern_mime_handler;
}

EOLIAN static void
_elm_file_icon_mime_set(Eo *obj, Elm_File_Icon_Data *pd, const char *mime)
{
   pd->mime_icon_file = eina_stringshare_add(mime);
   _icon_refresh(obj, pd);
}

EOLIAN static const char*
_elm_file_icon_mime_get(Eo *obj EINA_UNUSED, Elm_File_Icon_Data *pd)
{
  return pd->mime_icon_file;
}


EOLIAN static void
_elm_file_icon_editable_set(Eo *obj EINA_UNUSED, Elm_File_Icon_Data *pd, Eina_Bool editable)
{
  if (pd->write)
    pd->editable = editable;
  else
    ERR("editable cannot be set on a not writable file");
}

EOLIAN static Eina_Bool
_elm_file_icon_editable_get(Eo *obj EINA_UNUSED, Elm_File_Icon_Data *pd)
{
  return pd->editable;
}

EOLIAN static void
_elm_file_icon_edit_set(Eo *obj, Elm_File_Icon_Data *pd, Eina_Bool edit)
{
  if (!pd->editable || !pd->write)
    return;
  if (!edit)
    {
      /* check if the value has changed */
      const char *val;

      val = elm_entry_entry_get(pd->entry);
      if (!!strcmp(pd->name, val))
        {
           const char *olddir, *dir, *newdir;
           char buf[PATH_MAX];

           olddir = pd->path;
           //get the pure dir
           dir = ecore_file_dir_get(olddir);
           //build the new path
           snprintf(buf, sizeof(buf), "%s/%s", dir, val);
           //add it as new stringshare
           newdir = eina_stringshare_add(eina_file_path_sanitize(buf));
           //cleanup
           free((char*)dir);
           // mark that here is a rename
           pd->rename_in_progress = EINA_TRUE;
           //TODO call rename event
           //rename the file
           ecore_file_mv(olddir, newdir);
           //set the new file
           eo_do(obj, efl_file_set(newdir, NULL));
           //mark that the rename is done
           pd->rename_in_progress = EINA_FALSE;
        }
    }
  elm_layout_signal_emit(obj, edit ? "edit_start" : "edit_done", "");
  elm_entry_editable_set(pd->entry, edit);
}

EOLIAN static Eina_Bool
_elm_file_icon_edit_get(Eo *obj EINA_UNUSED, Elm_File_Icon_Data *pd)
{
  return elm_entry_editable_get(pd->entry);
}

#include "elm_file_icon.eo.x"
