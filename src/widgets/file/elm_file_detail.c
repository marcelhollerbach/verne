#include "../elementary_ext_priv.h"
#include <Emous.h>

typedef struct {
  Evas_Object *label;
  Evas_Object *display;
} Detail_Row_Immutable;

typedef struct {
  Evas_Object *label;
  Evas_Object *display;
  Evas_Object *change_display;
  Evas_Object *table;
} Detail_Row_Mutable;

typedef struct {
   Evas_Object *filepreview;
   Detail_Row_Mutable perm, user, group;
   Detail_Row_Immutable size, mtime, ctime, mtype, name;
   struct {
      Evas_Object *segment;
      Evas_Object *permstring;
      Evas_Object *flip[3];
      Elm_Object_Item *pmodes[3];
      Elm_Object_Item *nmodes[3];
      Elm_Object_Item *items[4];
      int user_type;
   } perm2;
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

static void detail_row_changable_changeable(Detail_Row_Mutable *row, Eina_Bool changeable);
static void _segment_update(Elm_File_Detail_Data *pd);

typedef Evas_Object* (*Mimetype_Cb)(Evas_Object *parent, Elm_File_MimeType_Cache *cache, Efm_File *file);

static Mimetype_Cb mimetype_cbs[MIME_TYPE_END];
static char*       mimetype_names[MIME_TYPE_END] = {
  "text", "image", "video", "audio", "application", "mutlipart", "message", "-"
};
static void
_chmod(Efm_File *file, int mode) {
  printf("Chmod to %d\n", mode);
}

static void
_chown(Efm_File *file, const char *user, const char *group) {
  printf("chown to %s - %s\n", user, group);
}

//================================================================
//Preview creation
//================================================================
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
_elm_file_detail_class_constructor(Eo_Class *c EINA_UNUSED) {
   mimetype_cbs[MIME_TYPE_FALLBACK] = _fallback_handler;
   mimetype_cbs[MIME_TYPE_IMAGE] = _image_handler;
   mimetype_cbs[MIME_TYPE_TEXT] = _text_handler;
}

//================================================================
//Widget impl
//================================================================
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
_generate_permissions_str(Efm_File *file, Efm_File_Stat *st, Evas_Object *o)
{
   char d,ur,uw,ux,gr,gw,gx,or,ow,ox;
   char buf[PATH_MAX];
   d = ur = uw = ux = gr = gw = gx = or = ow = ox = '-';
   Eina_Bool dir;
   if (eo_do_ret(file, dir, efm_file_is_type(EFM_FILE_TYPE_DIRECTORY))) d = 'd';
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
   elm_object_text_set(o, buf);
}

static void
_update_stat(Elm_File_Detail_Data *pd, Efm_File *file)
{
   char buf[PATH_MAX];
   const char *mime_type;
   Efm_File_Stat *st;
   Eina_Bool perm_right = EINA_FALSE;


   eo_do(file, mime_type = efm_file_mimetype_get();
               st = efm_file_stat_get()
         );


   if (!st) return;

   //check if we have the right to change the permission
   if (getuid() == 0)
     perm_right = EINA_TRUE;

   if (getuid() == (uid_t) st->uid)
     perm_right = EINA_TRUE;

   {
      char *nicestr;

      eo_do(EMOUS_CLASS, nicestr = emous_util_size_convert(EINA_TRUE, st->size));
      elm_object_text_set(pd->size.display, nicestr);
      free(nicestr);
   }

   snprintf(buf, sizeof(buf), "%s", ctime(&st->mtime));
   elm_object_text_set(pd->mtime.display, buf);

   snprintf(buf, sizeof(buf), "%s", ctime(&st->ctime));
   elm_object_text_set(pd->ctime.display, buf);

   //update the mimetype
   snprintf(buf, sizeof(buf), "%s", mime_type);
   elm_object_text_set(pd->mtype.display, buf);
   elm_object_tooltip_text_set(pd->mtype.display, buf);


   //===========
   //User
   //===========
   struct passwd *pw;

   pw = getpwuid(st->uid);
   snprintf(buf, sizeof(buf), "%s", pw->pw_name);

   detail_row_changable_changeable(&pd->user, perm_right);
   if (!perm_right)
     elm_object_text_set(pd->user.display, buf);
   else
     elm_object_text_set(pd->user.change_display, buf);

   //===========
   //Group
   //===========
   struct group *gr;

   gr = getgrgid(st->gid);
   snprintf(buf, sizeof(buf), "%s", gr->gr_name);

   detail_row_changable_changeable(&pd->group, perm_right);
   if (!perm_right)
     elm_object_text_set(pd->group.display, buf);
   else
     elm_object_text_set(pd->group.change_display, buf);

   //===========
   //Permissions
   //===========
   detail_row_changable_changeable(&pd->perm, perm_right);
   if (!perm_right)
     _generate_permissions_str(pd->file, st, pd->perm.display);
   else
     {
        _generate_permissions_str(pd->file, st, pd->perm2.permstring);
        _segment_update(pd);
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

static Eina_Bool
_file_changed(void *data, Eo *obj EINA_UNUSED, const Eo_Event_Description2 *desc EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Elm_File_Detail *oo;
   Elm_File_Detail_Data *pd;

   oo = data;
   pd = eo_data_scope_get(oo, ELM_FILE_DETAIL_CLASS);

   _update_thumbnail(oo, pd, pd->file);
   _update_stat(pd, pd->file);
   _segment_update(pd);
   return EO_CALLBACK_CONTINUE;
}

EOLIAN static void
_elm_file_detail_file_set(Eo *obj, Elm_File_Detail_Data *pd, Efm_File *file)
{
   const char *filename;

   if (pd->file)
     {
        eo_do(file, eo_event_callback_del(EFM_FILE_EVENT_CHANGED, _file_changed, obj));
        eo_weak_unref(&pd->file);
     }

   pd->file = file;
   if (!pd->file) return;

   eo_weak_ref(&pd->file);
   eo_do(pd->file, eo_event_callback_add(EFM_FILE_EVENT_CHANGED, _file_changed, obj));

   eo_do(pd->file, filename = efm_file_filename_get());

  _update_thumbnail(obj, pd, pd->file);

  //update stats
  _update_stat(pd, pd->file);

  //update the name of the preview
  elm_object_text_set(pd->name.display, filename);
}

EOLIAN static Efm_File *
_elm_file_detail_file_get(Eo *obj EINA_UNUSED, Elm_File_Detail_Data *pd)
{
   return pd->file;
}


//================================================================
//Group/User settings
//================================================================
static void
_standard_fill(Evas_Object *o, const char *name)
{
    FILE *fptr;
    char line[PATH_MAX];

    fptr = fopen(name,"r");

    if (fptr == NULL) {
         perror("Failed to read group");
         return;
    }

    while (fgets(line, sizeof(line), fptr)) {
        char *name;
        name = strtok(line, ":");
        elm_hoversel_item_add(o, name, NULL, 0, NULL, NULL);
    }

    fclose(fptr);
}

static void
_group_hover_init(Evas_Object *o) {
   _standard_fill(o, "/etc/group");
}

static void
_user_hover_init(Evas_Object *o) {
   _standard_fill(o, "/etc/passwd");
}
static void
_user_or_group_sel_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Elm_File_Detail_Data *pd;
   const char *user;
   const char *group;

   pd = eo_data_scope_get(data, ELM_FILE_DETAIL_CLASS);
   user = elm_object_text_get(pd->user.change_display);
   group = elm_object_text_get(pd->group.change_display);

   _chown(pd->file, user, group);
}
//================================================================
//Permission settings
//================================================================
static int FILE_PERM_MODES[3][3] = {{S_IRUSR,S_IWUSR,S_IXUSR},
                                    {S_IRGRP,S_IWGRP,S_IXGRP},
                                    {S_IROTH,S_IWOTH,S_IXOTH}};
static void
_segment_update(Elm_File_Detail_Data *pd)
{

   Efm_File_Stat *st;

   eo_do(pd->file, st = efm_file_stat_get());
   for (int i = 0; i < 3; i++)
     {
        if (st->mode & FILE_PERM_MODES[pd->perm2.user_type][i])
          elm_flipselector_item_selected_set(pd->perm2.pmodes[i], EINA_TRUE);
        else
          elm_flipselector_item_selected_set(pd->perm2.nmodes[i], EINA_TRUE);
     }
}

static void
_segment_changed_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Elm_File_Detail_Data *pd;

   pd = eo_data_scope_get(data, ELM_FILE_DETAIL_CLASS);

   if (pd->perm2.items[0] == event_info)
     {
        evas_object_show(pd->perm2.permstring);
        evas_object_hide(pd->perm2.flip[0]);
        evas_object_hide(pd->perm2.flip[1]);
        evas_object_hide(pd->perm2.flip[2]);
     }
   else
     {
       evas_object_hide(pd->perm2.permstring);
       evas_object_show(pd->perm2.flip[0]);
       evas_object_show(pd->perm2.flip[1]);
       evas_object_show(pd->perm2.flip[2]);

       for (int part = 1; part < 4; part ++)
         {
            if (pd->perm2.items[part] == event_info) {
              pd->perm2.user_type = part - 1;
            }
         }
       _segment_update(pd);
     }
}

static void
_flip_cb(void *data, Evas_Object *obj, void *event_info)
{
   Elm_File_Detail_Data *pd;
   Efm_File_Stat *st;
   int flip = -1;
   mode_t permission;

   pd = eo_data_scope_get(data, ELM_FILE_DETAIL_CLASS);
   eo_do(pd->file, st = efm_file_stat_get());
   permission = st->mode;

   //search the correct flip
   for(int i = 0; i < 3; i++)
     {
        if (obj == pd->perm2.flip[i])
          {
             flip = i;
             break;
          }
     }

   if (flip == -1)
     {
        ERR("Flip not found!");
        return;
     }

   //check if bit is set, but should not
   if (permission & FILE_PERM_MODES[pd->perm2.user_type][flip])
     {
        if (event_info == pd->perm2.nmodes[flip])
          permission = permission & ~FILE_PERM_MODES[pd->perm2.user_type][flip];
     }
   else
     {
        if (event_info == pd->perm2.pmodes[flip])
          permission = permission | FILE_PERM_MODES[pd->perm2.user_type][flip];
     }

   if ((mode_t)st->mode != permission)
     _chmod(pd->file, permission);
}

static void
_permission_init(Evas_Object *obj, Elm_File_Detail_Data *pd) {
   char *lbl[] = {"r","w","x"};

   pd->perm.change_display = elm_table_add(obj);

   pd->perm2.segment = elm_segment_control_add(obj);
   evas_object_smart_callback_add(pd->perm2.segment, "changed", _segment_changed_cb, obj);
   pd->perm2.items[0] = elm_segment_control_item_add(pd->perm2.segment, NULL, "View");
   pd->perm2.items[1] = elm_segment_control_item_add(pd->perm2.segment, NULL, "Owner");
   pd->perm2.items[2] = elm_segment_control_item_add(pd->perm2.segment, NULL, "Group");
   pd->perm2.items[3] = elm_segment_control_item_add(pd->perm2.segment, NULL, "Other");

   elm_segment_control_item_selected_set(pd->perm2.items[0], EINA_TRUE);
   elm_table_pack(pd->perm.change_display, pd->perm2.segment, 0, 0, 5, 1);
   evas_object_show(pd->perm2.segment);

   pd->perm2.permstring = elm_label_add(obj);
   elm_table_pack(pd->perm.change_display, pd->perm2.permstring, 0, 1, 5, 1);

   for (int i = 0; i < 3; i++)
     {
        pd->perm2.flip[i] = elm_flipselector_add(obj);
        pd->perm2.pmodes[i] = elm_flipselector_item_append(pd->perm2.flip[i], lbl[i], NULL, NULL);
        pd->perm2.nmodes[i] = elm_flipselector_item_append(pd->perm2.flip[i], "-", NULL, NULL);
        evas_object_smart_callback_add(pd->perm2.flip[i], "selected", _flip_cb, obj);
        elm_table_pack(pd->perm.change_display, pd->perm2.flip[i], i + 1, 1, 1, 1);
        evas_object_show(pd->perm2.flip[i]);
     }
   _segment_changed_cb(obj, pd->perm2.segment, pd->perm2.items[0]);
}
//================================================================
//Detail setup
//================================================================

static void
detail_row_changable_changeable(Detail_Row_Mutable *row, Eina_Bool changeable)
{
   if (changeable)
     {
        evas_object_show(row->change_display);
        evas_object_hide(row->display);
     }
   else
     {
        evas_object_hide(row->change_display);
        evas_object_show(row->display);
     }
}

static void
detail_row_changable_init(Evas_Object *obj, Detail_Row_Mutable *row)
{
   row->table = elm_table_add(obj);
   evas_object_size_hint_align_set(row->table, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(row->table, EVAS_HINT_EXPAND, 0.0); \

   row->display = elm_label_add(obj); \
   evas_object_size_hint_align_set(row->display, 0.5, 0.5); \
   evas_object_size_hint_weight_set(row->display, EVAS_HINT_EXPAND, 0.0); \
   elm_object_text_set(row->display, "-"); \
   evas_object_show(row->display);

   evas_object_size_hint_align_set(row->change_display, 0.5, 0.5);
   evas_object_size_hint_weight_set(row->change_display, EVAS_HINT_EXPAND, 0.0);
   elm_table_pack(row->table, row->change_display, 0, 0, 1, 1);

   evas_object_size_hint_align_set(row->display, 0.5, 0.5);
   evas_object_size_hint_weight_set(row->display, EVAS_HINT_EXPAND, 0.0);
   elm_table_pack(row->table, row->display, 0, 0, 1, 1);
   evas_object_show(row->table);
}

#define SEPERATOR \
  {\
  Evas_Object *s; \
  s = elm_separator_add(obj); \
  elm_separator_horizontal_set(s, EINA_TRUE); \
  elm_box_pack_end(bx, s); \
  evas_object_show(s); \
}

#define LABEL(w, v, a, e) \
   w = elm_label_add(obj); \
   evas_object_size_hint_align_set(w, a, 0.5); \
   elm_label_ellipsis_set(w, e); \
   elm_box_pack_end(bx,w ); \
   elm_object_text_set(w, v); \
   evas_object_show(w);

#define DETAIL_ROW_UNCHANGABLE(W, name) \
   LABEL(pd->W.label, name, 0.0, EINA_FALSE); \
   LABEL(pd->W.display, "", EVAS_HINT_FILL, EINA_TRUE);

#define DETAIL_ROW_CHANGABLE(W, name) \
   do { \
      LABEL(pd->W.label, name, 0.0, EINA_FALSE); \
      detail_row_changable_init(obj, &pd->W); \
      elm_box_pack_end(bx, pd->W.table); \
   } while(0)

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
   SEPERATOR
   DETAIL_ROW_UNCHANGABLE(name, "<b>Name:");
   SEPERATOR
   DETAIL_ROW_UNCHANGABLE(size, "<b>Size:");
   SEPERATOR
   pd->user.change_display = elm_hoversel_add(obj);
   evas_object_smart_callback_add(pd->user.change_display, "selected", _user_or_group_sel_cb, obj);
   _user_hover_init(pd->user.change_display);
   DETAIL_ROW_CHANGABLE(user, "<b>User:");
   SEPERATOR
   pd->group.change_display = elm_hoversel_add(obj);
   evas_object_smart_callback_add(pd->group.change_display, "selected", _user_or_group_sel_cb, obj);
   _group_hover_init(pd->group.change_display);
   DETAIL_ROW_CHANGABLE(group, "<b>Group:");
   SEPERATOR
   _permission_init(obj, pd);
   DETAIL_ROW_CHANGABLE(perm, "<b>Permission:");
   SEPERATOR
   DETAIL_ROW_UNCHANGABLE(mtime, "<b>Modification Time:");
   SEPERATOR
   DETAIL_ROW_UNCHANGABLE(ctime, "<b>Creation Time:");
   SEPERATOR
   DETAIL_ROW_UNCHANGABLE(mtype, "<b>Mime Type:");

   elm_object_part_content_set(obj, "content", bx);
}

#include "elm_file_detail.eo.x"