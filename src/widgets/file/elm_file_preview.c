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
   Efm_File *file;
   Elm_File_MimeType_Cache *cache;
} Elm_File_Preview_Data;

EOLIAN static void
_elm_file_preview_cache_set(Eo *obj EINA_UNUSED, Elm_File_Preview_Data *pd, Elm_File_MimeType_Cache *cache)
{
   eo_weak_unref(&pd->cache);
    pd->cache = cache;
   eo_weak_ref(&pd->cache);
}

EOLIAN static Elm_File_MimeType_Cache *
_elm_file_preview_cache_get(Eo *obj EINA_UNUSED, Elm_File_Preview_Data *pd)
{
   return pd->cache;
}

EOLIAN static void
_elm_file_preview_file_set(Eo *obj, Elm_File_Preview_Data *pd, Efm_File *filee)
{
   Evas_Object *o;
   char buf[PATH_MAX];
   const char *path;
   const char *mime_type;
   const char *filename;
   Efm_File_Stat *st;

   eo_weak_unref(&pd->file);
   pd->file = filee;
   eo_weak_ref(&pd->file);

  eo_do(pd->file, path = efm_file_path_get();
              mime_type = efm_file_mimetype_get();
              filename = efm_file_filename_get();
        );


  // TODO make a nice thumbnail :)
  //- text thumbnail
  //- image file thumbnail
  //- just the mime type icon
  if (pd->filepreview)
    {
       elm_object_part_content_unset(obj, "thumb");
       evas_object_del(pd->filepreview);
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
       // make a thumb
       o = elm_thumb_add(obj);
       eo_do(o, efl_file_set(path, NULL));
    }
  else
    {
       const char *ic;
       Eina_Bool is;

       // display the mime_type icon
       o = elm_icon_add(obj);
       elm_icon_order_lookup_set(o, ELM_ICON_LOOKUP_FDO);
       if (eo_do_ret(pd->file, is, efm_file_is_type(EFM_FILE_TYPE_DIRECTORY)))
         ic = "folder";
       else
         {
            eo_do(pd->cache, ic = elm_file_mimetype_cache_mimetype_get(mime_type));
         }

       eo_do(o, elm_obj_icon_standard_set(ic));
    }

  evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
  evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

  pd->filepreview = o;
  elm_object_part_content_set(obj, "thumb", o);
  evas_object_show(o);

  eo_do(pd->file, st = efm_file_stat_get());

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

  snprintf(buf, sizeof(buf), "%s", mime_type);
  elm_object_text_set(pd->mtype, buf);
  {
    elm_object_text_set(pd->name, filename);
  }
}

EOLIAN static Efm_File *
_elm_file_preview_file_get(Eo *obj EINA_UNUSED, Elm_File_Preview_Data *pd)
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
_elm_file_preview_evas_object_smart_add(Eo *obj, Elm_File_Preview_Data *pd)
{
   Evas_Object *bx;

   eo_do_super(obj, ELM_FILE_PREVIEW_CLASS, evas_obj_smart_add());

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

   LABEL(pd->mtime_name, "Modification Time:", 0.0, EINA_FALSE);
   LABEL(pd->mtime, "", EVAS_HINT_FILL, EINA_TRUE);

   LABEL(pd->ctime_name, "Creation Time:", 0.0, EINA_FALSE);
   LABEL(pd->ctime, "", EVAS_HINT_FILL, EINA_TRUE);

   LABEL(pd->mtype_name, "Mime Type:", 0.0, EINA_FALSE);
   LABEL(pd->mtype, "", EVAS_HINT_FILL, EINA_TRUE);

   elm_object_part_content_set(obj, "content", bx);
}

#include "elm_file_preview.eo.x"