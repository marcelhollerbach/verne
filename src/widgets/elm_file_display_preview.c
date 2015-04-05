#include "elm_file_display_priv.h"

typedef struct
{
   Evas_Object *filepreview;
   Evas_Object *size, *size_name;
   Evas_Object *mtime, *mtime_name;
   Evas_Object *user, *user_name;
   Evas_Object *group, *group_name;
   Evas_Object *ctime, *ctime_name;
   Evas_Object *mtype, *mtype_name;
} Filepreview;
 #if 0
static const char*
_preview_read_file(FM_Monitor_File *file)
{
   FILE *f;
   char buf[PATH_MAX];
   const char *esc;

   f = fopen(file->path, "r");

   fread(buf, sizeof(char), sizeof(buf), f);

   ERR("WE GOT: %s", buf);

   esc = evas_textblock_escape_string_get(buf);
   if (esc)
     return esc;
   else
     return strdup(buf);
}
#endif
void
filepreview_file_set(Evas_Object *w, FM_Monitor_File *file)
{
  Filepreview *f;
  Evas_Object *o;
  char buf[PATH_MAX];
  const char *path;
  const char *mime_type;
  struct stat *st;

  //char[] bytes = {"B", "KB", "MB", "GB", "TB", "PM", "EM", NULL}
  f = evas_object_data_get(w, "__ctx");
  path = efm_file_path_get(file);
  mime_type = efm_file_mimetype_get(file);

  if (!f) return;

  //TODO make a nice thumbnail :)
  //- text thumbnail
  //- image file thumbnail
  //- just the mime type icon
  if (f->filepreview)
    {
       elm_object_part_content_unset(w, "thumb");
       evas_object_del(f->filepreview);
    }
#if 0
  if (!strncmp("text/", file->mime_type, 5))
    {
       Evas_Object *tx;

       const char *content;
       //text preview
       o = elm_layout_add(w);
       if (!elm_layout_theme_set(o, "file_display", "file_text_preview", "default"))
         {
            CRI("Failed to set theme file\n");
         }

       tx = elm_entry_add(o);
       elm_entry_editable_set(tx, EINA_TRUE);
       content = _preview_read_file(file);
       snprintf(buf, sizeof(buf), "<font_size=30><color=BLACK>%s", content);
       elm_entry_entry_set(tx, buf);
       elm_object_part_content_set(o, "content", tx);
       free((char*)content);
    }
  else
    #endif
    if (evas_object_image_extension_can_load_fast_get(path))
    {
       //make a thumb
       o = elm_thumb_add(w);
       eo_do(o, efl_file_set(path, NULL));
    }
  else
    {
       char *theme;
       const char *ic;
       const char *mime_type;

       //display the mime_type icon
       o = elm_icon_add(w);
       mime_type = efm_file_mimetype_get(file);

       theme = getenv("E_ICON_THEME");
       if (!theme)
         theme = "hicolor";

       ic = efreet_mime_type_icon_get(mime_type, theme, 256);
       eo_do(o, efl_file_set(ic, NULL));
    }

  evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
  evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

  f->filepreview = o;
  elm_object_part_content_set(w, "thumb", o);
  evas_object_show(o);

  st = efm_file_stat_get(file);

  snprintf(buf, sizeof(buf), "%zu B", st->st_size);
  elm_object_text_set(f->size, buf);

  snprintf(buf, sizeof(buf), "%s", ctime(&st->st_mtim.tv_sec));
  elm_object_text_set(f->mtime, buf);

  snprintf(buf, sizeof(buf), "%s", ctime(&st->st_ctim.tv_sec));
  elm_object_text_set(f->ctime, buf);

  {
    struct passwd *pw;
    pw = getpwuid(st->st_uid);

    snprintf(buf, sizeof(buf), "%s", pw->pw_name);
    elm_object_text_set(f->user, buf);
  }

  {
    struct group *gr;

    gr = getgrgid(st->st_gid);

    snprintf(buf, sizeof(buf), "%s", gr->gr_name);
    elm_object_text_set(f->group, buf);
  }

  snprintf(buf, sizeof(buf), "%s", mime_type);
  elm_object_text_set(f->mtype, buf);

  evas_object_show(w);
}

#define LABEL(w, v, a, e) \
   w = elm_label_add(resu); \
   evas_object_size_hint_align_set(w, a, 0.5); \
   elm_label_ellipsis_set(w, e); \
   elm_box_pack_end(bx,w ); \
   elm_object_text_set(w, v); \
   evas_object_show(w);

Evas_Object*
filepreview_add(Evas_Object *w)
{
   Evas_Object *resu, *bx;
   Filepreview *f;

   f = calloc(1, sizeof(Filepreview));

   resu = elm_layout_add(w);

   bx = elm_box_add(resu);

   LABEL(f->size_name, "Size:", 0.0, EINA_FALSE);
   LABEL(f->size, "", EVAS_HINT_FILL, EINA_TRUE);

   LABEL(f->user_name, "User:", 0.0, EINA_FALSE);
   LABEL(f->user, "", EVAS_HINT_FILL, EINA_TRUE);

   LABEL(f->group_name, "Group:", 0.0, EINA_FALSE);
   LABEL(f->group, "", EVAS_HINT_FILL, EINA_TRUE);

   LABEL(f->mtime_name, "Modification Time:", 0.0, EINA_FALSE);
   LABEL(f->mtime, "", EVAS_HINT_FILL, EINA_TRUE);

   LABEL(f->ctime_name, "Creation Time:", 0.0, EINA_FALSE);
   LABEL(f->ctime, "", EVAS_HINT_FILL, EINA_TRUE);

   LABEL(f->mtype_name, "Mime Type:", 0.0, EINA_FALSE);
   LABEL(f->mtype, "", EVAS_HINT_FILL, EINA_TRUE);

   evas_object_data_set(resu, "__ctx", f);

   if (!elm_layout_theme_set(resu, "file_display", "file_preview", "default"))
     {
        CRI("Failed to set theme file");
     }

   elm_object_part_content_set(resu, "content", bx);

   return resu;
}