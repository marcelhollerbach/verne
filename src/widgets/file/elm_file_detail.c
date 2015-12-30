#include "../elementary_ext_priv.h"
#include <Emous.h>

typedef struct {
   Evas_Object *filepreview;
   Evas_Object *size, *size_name;
   Evas_Object *mtime, *mtime_name;
   Evas_Object *user, *user_name;
   Evas_Object *group, *group_name;
   Evas_Object *ctime, *ctime_name;
   Evas_Object *mtype, *mtype_name;
   Evas_Object *name, *name_name;
   Evas_Object *perm, *perm_name;
   Efm_File *file;
   Elm_File_MimeType_Cache *cache;
} Elm_File_Detail_Data;

typedef enum {
  MIME_TYPE_TEXT = 0,
  MIME_TYPE_IMAGE = 1,
  MIME_TYPE_VIDEO = 2,
  MIME_TYPE_AUDIO = 3,
  MIME_TYPE_APPLICATION = 4,
  MIME_TYPE_MULTIPART = 5,
  MIME_TYPE_MESSAGE = 6,
  MIME_TYPE_FALLBACK = 7,
  MIME_TYPE_END = 8
} MIME_TYPES;

typedef Evas_Object* (*Mimetype_Cb)(Evas_Object *parent, Elm_File_MimeType_Cache *cache, Efm_File *file);

static Mimetype_Cb mimetype_cbs[MIME_TYPE_END];
static char*       mimetype_names[MIME_TYPE_END] = {
  "text", "image", "video", "audio", "application", "mutlipart", "message", "-"
};
static Evas_Object*
_fallback_handler(Evas_Object *obj, Elm_File_MimeType_Cache *cache ,Efm_File *file)
{
   const char *ic;
   const char *mime_type;
   Evas_Object *o;
   Eina_Bool is;

   eo_do(file, mime_type = efm_file_mimetype_get());

   // display the mime_type icon
   o = elm_icon_add(obj);
   elm_icon_order_lookup_set(o, ELM_ICON_LOOKUP_FDO);
   if (eo_do_ret(file, is, efm_file_is_type(EFM_FILE_TYPE_DIRECTORY)))
     ic = "folder";
   else
     eo_do(cache, ic = elm_file_mimetype_cache_mimetype_get(mime_type));
   eo_do(o, elm_obj_icon_standard_set(ic));

   return o;
}

static Evas_Object*
_image_handler(Evas_Object *obj, Elm_File_MimeType_Cache *cache EINA_UNUSED, Efm_File *file)
{
   Evas_Object *o;
   const char *path;

   o = elm_thumb_add(obj);
   elm_thumb_file_set(o, eo_do_ret(file, path, efm_file_path_get()), NULL);

   return o;
}

static Evas_Object*
_text_handler(Evas_Object *obj, Elm_File_MimeType_Cache *cache EINA_UNUSED, Efm_File *file)
{
   Evas_Object *o;
   FILE *fd;
   const char *path;
   Eina_Strbuf *buffer;
   char buf[PATH_MAX];
   int i = 0;

   eo_do(file, path = efm_file_path_get());

   o = elm_layout_add(obj);
   if (!elm_layout_theme_set(o, "file_preview", "base", "default"))
     {
        CRI("Failed to set theme file\n");
        return NULL;
     }

   fd = fopen(path, "r");

   buffer = eina_strbuf_new();

   while (fgets(buf, sizeof(buf), fd) != NULL) {
      eina_strbuf_append(buffer, buf);
      eina_strbuf_append(buffer, "<br>");
      i++;
      if (i > 5) break;
   }

   elm_object_part_text_set(o, "visible", eina_strbuf_string_get(buffer));

   eina_strbuf_free(buffer);

   fclose(fd);

   return o;
}
EOLIAN static void
_elm_file_detail_class_constructor(Eo_Class *c) {
   mimetype_cbs[MIME_TYPE_FALLBACK] = _fallback_handler;
   mimetype_cbs[MIME_TYPE_IMAGE] = _image_handler;
   mimetype_cbs[MIME_TYPE_TEXT] = _text_handler;
}

EOLIAN static void
_elm_file_detail_cache_set(Eo *obj EINA_UNUSED, Elm_File_Detail_Data *pd, Elm_File_MimeType_Cache *cache)
{
   eo_weak_unref(&pd->cache);
    pd->cache = cache;
   eo_weak_ref(&pd->cache);
}

EOLIAN static Elm_File_MimeType_Cache *
_elm_file_detail_cache_get(Eo *obj EINA_UNUSED, Elm_File_Detail_Data *pd)
{
   return pd->cache;
}

static void
_update_stat(Elm_File_Detail_Data *pd, Efm_File *file)
{
   char buf[PATH_MAX];
   const char *mime_type;
   Efm_File_Stat *st;

   eo_do(file, mime_type = efm_file_mimetype_get();
               st = efm_file_stat_get()
         );

   if (!st) return;

   {
      char *nicestr;

      eo_do(EMOUS_CLASS, nicestr = emous_util_size_convert(EINA_TRUE, st->size));
      elm_object_text_set(pd->size, nicestr);
      free(nicestr);
   }
   snprintf(buf, sizeof(buf), "%s", ctime(&st->mtime));
   elm_object_text_set(pd->mtime, buf);

   snprintf(buf, sizeof(buf), "%s", ctime(&st->ctime));
   elm_object_text_set(pd->ctime, buf);

   {
     struct passwd *pw;
     pw = getpwuid(st->uid);

     snprintf(buf, sizeof(buf), "%s", pw->pw_name);
     elm_object_text_set(pd->user, buf);
   }

   {
     struct group *gr;

     gr = getgrgid(st->gid);

     snprintf(buf, sizeof(buf), "%s", gr->gr_name);
     elm_object_text_set(pd->group, buf);
   }

   {
      char d,ur,uw,ux,gr,gw,gx,or,ow,ox;
      d = ur = uw = ux = gr = gw = gx = or = ow = ox = '-';
      Eina_Bool dir;
      if (eo_do_ret(pd->file, dir, efm_file_is_type(EFM_FILE_TYPE_DIRECTORY))) d = 'd';
      if (st->mode & S_IRUSR) ur = 'r';
      if (st->mode & S_IWUSR) uw = 'w';
      if (st->mode & S_IXUSR) ux = 'x';
      if (st->mode & S_IRGRP) gr = 'r';
      if (st->mode & S_IWGRP) gw = 'w';
      if (st->mode & S_IXGRP) gx = 'x';
      if (st->mode & S_IROTH) or = 'r';
      if (st->mode & S_IWOTH) ow = 'w';
      if (st->mode & S_IXOTH) ox = 'x';

      snprintf(buf, sizeof(buf), "%c%c%c%c%c%c%c%c%c%c\n", d, ur, uw, ux, gr, gw, gx, or, ow, ox);
      elm_object_text_set(pd->perm, buf);
   }
   {
      //update the mimetype
      snprintf(buf, sizeof(buf), "%s", mime_type);
      elm_object_text_set(pd->mtype, buf);
      elm_object_tooltip_text_set(pd->mtype, buf);
   }
}

EOLIAN static void
_update_thumbnail(Eo *obj, Elm_File_Detail_Data *pd, Efm_File *file)
{
   const char *mime_type;
   Evas_Object *o = NULL;
   //delete existing filepreview
   if (pd->filepreview)
     {
        elm_object_part_content_unset(obj, "thumb");
        evas_object_del(pd->filepreview);
     }
   eo_do(file, mime_type = efm_file_mimetype_get());
   //there are "group mimetypes" for text/image/video/audio/application/multipart/message and model.
   //handlers can subsribe to them to provide a good thumbnail
   for (int i = 0; i < MIME_TYPE_END; i++)
     {
        if (!strncmp(mimetype_names[i], mime_type, strlen(mimetype_names[i])))
          {
             if (!mimetype_cbs[i]) break;

             o = mimetype_cbs[i](obj, pd->cache, file);
             break;
          }
     }
   if (!o)
     o = mimetype_cbs[MIME_TYPE_FALLBACK](obj, pd->cache, file);

  evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
  evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

  //update filepreview
  pd->filepreview = o;
  elm_object_part_content_set(obj, "thumb", o);
  evas_object_show(o);
}

EOLIAN static void
_elm_file_detail_file_set(Eo *obj, Elm_File_Detail_Data *pd, Efm_File *filee)
{
   const char *filename;

   eo_weak_unref(&pd->file);
   pd->file = filee;
   if (!pd->file) return;
   eo_weak_ref(&pd->file);

   eo_do(pd->file, filename = efm_file_filename_get());

  _update_thumbnail(obj, pd, filee);

  //update stats
  _update_stat(pd, pd->file);

  //update the name of the preview
  elm_object_text_set(pd->name, filename);
}

EOLIAN static Efm_File *
_elm_file_detail_file_get(Eo *obj EINA_UNUSED, Elm_File_Detail_Data *pd)
{
   return pd->file;
}

#define LABEL(w, v, a, e) \
   w = elm_label_add(obj); \
   evas_object_size_hint_align_set(w, a, 0.5); \
   elm_label_ellipsis_set(w, e); \
   elm_box_pack_end(bx,w ); \
   elm_object_text_set(w, v); \
   evas_object_show(w);

EOLIAN static void
_elm_file_detail_evas_object_smart_add(Eo *obj, Elm_File_Detail_Data *pd)
{
   Evas_Object *bx;

   eo_do_super(obj, ELM_FILE_DETAIL_CLASS, evas_obj_smart_add());

   if (!elm_layout_theme_set(obj, "file_display", "file_preview", "default"))
     {
        CRI("Failed to set theme file");
     }

   bx = elm_box_add(obj);

   LABEL(pd->name_name, "Name:", 0.0, EINA_FALSE);
   LABEL(pd->name, "", EVAS_HINT_FILL, EINA_TRUE);

   LABEL(pd->size_name, "Size:", 0.0, EINA_FALSE);
   LABEL(pd->size, "", EVAS_HINT_FILL, EINA_TRUE);

   LABEL(pd->user_name, "User:", 0.0, EINA_FALSE);
   LABEL(pd->user, "", EVAS_HINT_FILL, EINA_TRUE);

   LABEL(pd->group_name, "Group:", 0.0, EINA_FALSE);
   LABEL(pd->group, "", EVAS_HINT_FILL, EINA_TRUE);

   LABEL(pd->perm_name, "Permission:", 0.0, EINA_FALSE);
   LABEL(pd->perm, "", EVAS_HINT_FILL, EINA_TRUE);

   LABEL(pd->mtime_name, "Modification Time:", 0.0, EINA_FALSE);
   LABEL(pd->mtime, "", EVAS_HINT_FILL, EINA_TRUE);

   LABEL(pd->ctime_name, "Creation Time:", 0.0, EINA_FALSE);
   LABEL(pd->ctime, "", EVAS_HINT_FILL, EINA_TRUE);

   LABEL(pd->mtype_name, "Mime Type:", 0.0, EINA_FALSE);
   LABEL(pd->mtype, "", EVAS_HINT_FILL, EINA_TRUE);

   elm_object_part_content_set(obj, "content", bx);
}

#include "elm_file_detail.eo.x"