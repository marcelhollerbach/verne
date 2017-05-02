#define INEEDWIDGET
#include "../elementary_ext_priv.h"

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
  Evas_Object *notify;
  Evas_Object *label;
} Notify_Ui;

typedef struct {
   Evas_Object *filepreview;
   Detail_Row_Mutable perm, user, group, name;
   Detail_Row_Immutable size, mtime, ctime, mtype;
   struct {
      Evas_Object *segment;
      Evas_Object *permstring;
      Evas_Object *flip[3];
      Elm_Object_Item *pmodes[3];
      Elm_Object_Item *nmodes[3];
      Elm_Object_Item *items[4];
      int user_type;
   } perm2;
   struct {
     const char *user;
     const char *group;
     int mode;
     Notify_Ui chmod, chown;
   } changes;
   Efm_File *file;
   Elm_File_MimeType_Cache *cache;
   Eina_Bool havy_setup;
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
static void _flip_update(Elm_File_Detail_Data *pd);
static void _generate_permissions_str(char *buf, size_t s, int mode);

typedef Evas_Object* (*Mimetype_Cb)(Evas_Object *parent, Elm_File_MimeType_Cache *cache, Efm_File *file);

static Mimetype_Cb mimetype_cbs[MIME_TYPE_END];
static char*       mimetype_names[MIME_TYPE_END] = {
  "text", "image", "video", "audio", "application", "mutlipart", "message", "-"
};
static void
_switch_to_edit(Evas_Object *obj)
{
   Elm_File_Detail_Data *pd;
   pd = efl_data_scope_get(obj, ELM_FILE_DETAIL_CLASS);

   evas_object_hide(elm_object_parent_widget_get(pd->name.display));
   evas_object_show(pd->name.change_display);
   elm_object_focus_set(pd->name.change_display, EINA_TRUE);
}

static void
_switch_to_edit_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   _switch_to_edit(data);
}

static void
_switch_to_display(Evas_Object *obj)
{
   Elm_File_Detail_Data *pd;
   pd = efl_data_scope_get(obj, ELM_FILE_DETAIL_CLASS);

   evas_object_show(elm_object_parent_widget_get(pd->name.display));
   evas_object_hide(pd->name.change_display);
}

static void
_hide(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
  evas_object_hide(data);
}

static void
_notify_ui_init(Evas_Object *obj, Notify_Ui *ui, Evas_Smart_Cb _no, Evas_Smart_Cb _yes) {
   Evas_Object *box, *yes, *no;
   if (ui->notify)
     {
        elm_notify_timeout_set(ui->notify, 5.0);
        evas_object_show(ui->notify);
        return;
     }

   ui->notify = elm_notify_add(elm_object_top_widget_get(obj));
   evas_object_smart_callback_add(ui->notify, "timeout", _no, obj);
   efl_weak_ref(&ui->notify);
   elm_notify_timeout_set(ui->notify, 5.0);
   elm_notify_align_set(ui->notify, 1.0, 0.0);
   evas_object_show(ui->notify);

   ui->label = elm_label_add(ui->notify);
   efl_weak_ref(&ui->label);
   evas_object_show(ui->label);

   yes = elm_button_add(ui->notify);
   evas_object_smart_callback_add(yes, "clicked", _hide, ui->notify);
   evas_object_smart_callback_add(yes, "clicked", _yes, obj);
   elm_object_text_set(yes, "Yes");
   evas_object_show(yes);

   no = elm_button_add(ui->notify);
   evas_object_smart_callback_add(no, "clicked", _hide, ui->notify);
   evas_object_smart_callback_add(no, "clicked", _no, obj);
   elm_object_text_set(no, "no");
   evas_object_show(no);

   box = elm_box_add(ui->notify);
   elm_box_horizontal_set(box, EINA_TRUE);
   elm_box_pack_end(box, ui->label);
   elm_box_pack_end(box, yes);
   elm_box_pack_end(box, no);
   evas_object_show(box);

   elm_object_content_set(ui->notify, box);
}

static void
_chmod_yes(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Elm_File_Detail_Data *pd;
   const char *path;

   pd = efl_data_scope_get(data, ELM_FILE_DETAIL_CLASS);
   path = efm_file_path_get(pd->file);

   if (chmod(path, pd->changes.mode) < 0)
     {
        perror("modding file failed");
     }


   pd->changes.mode = 0;
}

static void
_chmod_no(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Elm_File_Detail_Data *pd;

   pd = efl_data_scope_get(data, ELM_FILE_DETAIL_CLASS);
   pd->changes.mode = 0;
}

static void
_request_chmod(Evas_Object *obj, int mode) {
   Elm_File_Detail_Data *pd;
   char buf[PATH_MAX], perm[PATH_MAX];
   Efm_File_Stat *st;

   pd = efl_data_scope_get(obj, ELM_FILE_DETAIL_CLASS);
   pd->changes.mode = mode;
   st = efm_file_stat_get(pd->file);

   if (pd->changes.mode == st->mode)
     {
        pd->changes.mode = 0;
        evas_object_hide(pd->changes.chmod.notify);
     }
   else
     {
        //
        _notify_ui_init(obj, &pd->changes.chmod, _chmod_no, _chmod_yes);
        _generate_permissions_str(perm, sizeof(perm), pd->changes.mode);
        snprintf(buf, sizeof(buf), "Change permission to %s", perm);

        elm_object_text_set(pd->changes.chmod.label, buf);
     }


}

static void
_chown_yes(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Elm_File_Detail_Data *pd;
   Efm_File_Stat *st;
   const char *path;
   uid_t user;
   gid_t group;

   pd = efl_data_scope_get(data, ELM_FILE_DETAIL_CLASS);

   if (pd->changes.group)
     {
        struct group *grp;

        grp = getgrnam(pd->changes.group);

        group = grp->gr_gid;
     }
   else
     {
        st = efm_file_stat_get(pd->file);
        group = st->gid;
     }

   if (pd->changes.user)
     {
        struct passwd *usr;

        usr = getpwnam(pd->changes.user);

        user = usr->pw_uid;
     }
   else
     {
        st = efm_file_stat_get(pd->file);
        user = st->uid;
     }

   path = efm_file_path_get(pd->file);
   if (chown(path, user, group) < 0)
     {
        perror("chown failed");
     }

   pd->changes.user = NULL;
   pd->changes.group = NULL;
}

static void
_chown_no(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Elm_File_Detail_Data *pd;

   pd = efl_data_scope_get(data, ELM_FILE_DETAIL_CLASS);
   pd->changes.user = NULL;
   pd->changes.group = NULL;
}
static void
_request_chown_user(Evas_Object *obj, const char *user) {
   Elm_File_Detail_Data *pd;
   Efm_File_Stat *st;
   char buf[PATH_MAX];
   const char *group;

   pd = efl_data_scope_get(obj, ELM_FILE_DETAIL_CLASS);
   pd->changes.user = user;

   if (pd->changes.group)
     group = pd->changes.group;
   else
     {
        struct group *grp;

        st = efm_file_stat_get(pd->file);
        grp = getgrgid(st->gid);
        group = grp->gr_name;
     }

   snprintf(buf, sizeof(buf), "Change owner to %s - %s", pd->changes.user, group);

   _notify_ui_init(obj, &pd->changes.chown, _chown_no, _chown_yes);
   elm_object_text_set(pd->changes.chown.label, buf);
}

static void
_request_chown_group(Evas_Object *obj, const char *group) {
   Elm_File_Detail_Data *pd;
   Efm_File_Stat *st;
   char buf[PATH_MAX];
   const char *user;

   pd = efl_data_scope_get(obj, ELM_FILE_DETAIL_CLASS);
   pd->changes.group = group;

   if (pd->changes.user)
     user = pd->changes.user;
   else
     {
        struct passwd *pwd;

        st = efm_file_stat_get(pd->file);
        pwd = getpwuid(st->uid);
        user = pwd->pw_name;
     }

   snprintf(buf, sizeof(buf), "Change owner to %s - %s", user, pd->changes.group);

   _notify_ui_init(obj, &pd->changes.chown, _chown_no, _chown_yes);
   elm_object_text_set(pd->changes.chown.label, buf);
}


//================================================================
//Preview creation
//================================================================
static Evas_Object*
_fallback_handler(Evas_Object *obj, Elm_File_MimeType_Cache *cache ,Efm_File *file)
{
   const char *mime_type;
   Evas_Object *o;

   mime_type = efm_file_mimetype_get(file);

   // display the mime_type icon
   o = elm_icon_add(obj);

   if (efm_file_is_type(file, EFM_FILE_TYPE_DIRECTORY))
     elm_file_mimetype_cache_mimetype_set(cache, o, "folder");
   else
     elm_file_mimetype_cache_mimetype_set(cache, o, mime_type);

   return o;
}

static Evas_Object*
_image_handler(Evas_Object *obj, Elm_File_MimeType_Cache *cache EINA_UNUSED, Efm_File *file)
{
   Evas_Object *o;

   o = elm_thumb_add(obj);
   elm_thumb_file_set(o, efm_file_path_get(file), NULL);

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

   path = efm_file_path_get(file);

   o = elm_layout_add(obj);
   if (!elm_layout_theme_set(o, "file_preview", "base", "default"))
     {
        CRI("Failed to set theme file\n");
        return NULL;
     }

   fd = fopen(path, "r");

   if (!fd) goto end;

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

   end:

   return o;
}

EOLIAN static void
_elm_file_detail_class_constructor(Efl_Class *c EINA_UNUSED) {
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
   efl_weak_unref(&pd->cache);
    pd->cache = cache;
   efl_weak_ref(&pd->cache);
}

EOLIAN static Elm_File_MimeType_Cache *
_elm_file_detail_cache_get(Eo *obj EINA_UNUSED, Elm_File_Detail_Data *pd)
{
   return pd->cache;
}

static void
_generate_permissions_str(char *buf, size_t s, int mode)
{
   char d,ur,uw,ux,gr,gw,gx,or,ow,ox;

   d = ur = uw = ux = gr = gw = gx = or = ow = ox = '-';

   if (S_ISDIR(mode)) d = 'd';
   if (mode & S_IRUSR) ur = 'r';
   if (mode & S_IWUSR) uw = 'w';
   if (mode & S_IXUSR) ux = 'x';
   if (mode & S_IRGRP) gr = 'r';
   if (mode & S_IWGRP) gw = 'w';
   if (mode & S_IXGRP) gx = 'x';
   if (mode & S_IROTH) or = 'r';
   if (mode & S_IWOTH) ow = 'w';
   if (mode & S_IXOTH) ox = 'x';

   snprintf(buf, s, "%c%c%c%c%c%c%c%c%c%c\n", d, ur, uw, ux, gr, gw, gx, or, ow, ox);
}

static void
_generate_permissions(Efm_File_Stat *st, Evas_Object *o)
{
   char buf[PATH_MAX];

   _generate_permissions_str(buf, PATH_MAX, st->mode);
   elm_object_text_set(o, buf);
}

static void
_update_stat(Elm_File_Detail_Data *pd, Efm_File *file)
{
   char buf[PATH_MAX];
   const char *mime_type;
   Efm_File_Stat *st;
   Eina_Bool perm_right = EINA_FALSE;


   mime_type = efm_file_mimetype_get(file);
   st = efm_file_stat_get(file);


   if (!st) return;

   //check if we have the right to change the permission
   if (getuid() == 0)
     perm_right = EINA_TRUE;

   if (getuid() == (uid_t) st->uid)
     perm_right = EINA_TRUE;

   {
      char *nicestr;

      nicestr = emous_util_size_convert(EMOUS_CLASS, EINA_TRUE, st->size);
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
   if (pw)
     snprintf(buf, sizeof(buf), "%s", pw->pw_name);
   else
     snprintf(buf, sizeof(buf), "User not found");

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
   if (gr)
     snprintf(buf, sizeof(buf), "%s", gr->gr_name);
   else
     snprintf(buf, sizeof(buf), "Group not found");

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
     _generate_permissions(st, pd->perm.display);
   else
     {
        _generate_permissions(st, pd->perm2.permstring);
        _flip_update(pd);
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
   mime_type = efm_file_mimetype_get(file);

   //we cannot do anything if there is no mimetype for this file
   if (!mime_type) return;

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

static void
_file_changed(void *data, const Efl_Event *event EINA_UNUSED)
{
   Elm_File_Detail *oo;
   Elm_File_Detail_Data *pd;

   oo = data;
   pd = efl_data_scope_get(oo, ELM_FILE_DETAIL_CLASS);

   _update_thumbnail(oo, pd, pd->file);
   _update_stat(pd, pd->file);
   _flip_update(pd);
}

EOLIAN static void
_elm_file_detail_file_set(Eo *obj, Elm_File_Detail_Data *pd, Efm_File *file)
{
   const char *filename;
   Efm_File_Stat *st;

   if (pd->file)
     {
        efl_event_callback_del(file, EFM_FILE_EVENT_CHANGED, _file_changed, obj);
        efl_weak_unref(&pd->file);
     }

   pd->file = file;
   if (!pd->file) return;

   efl_weak_ref(&pd->file);
   st = efm_file_stat_get(pd->file);
   efl_event_callback_add(pd->file, EFM_FILE_EVENT_CHANGED, _file_changed, obj);

   filename = efm_file_filename_get(pd->file);

  _update_thumbnail(obj, pd, pd->file);

  //update stats
  _update_stat(pd, pd->file);

  //update the name of the preview
  elm_object_text_set(pd->name.change_display, filename);
  elm_object_text_set(pd->name.display, filename);
  elm_object_tooltip_text_set(pd->name.display, filename);
  _switch_to_display(obj);

  pd->changes.user = NULL;
  pd->changes.group = NULL;
  pd->changes.mode = 0;
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
_standard_fill(Evas_Object *obj, Evas_Object *o, Evas_Smart_Cb cb, const char *filename)
{
    FILE *fptr;
    char line[PATH_MAX];

    fptr = fopen(filename,"r");

    if (fptr == NULL) {
         perror("Failed to read group");
         return;
    }

    while (fgets(line, sizeof(line), fptr)) {
        char *name;
        name = strtok(line, ":");
        elm_hoversel_item_add(o, name, NULL, 0, cb, obj);
    }

    fclose(fptr);
}

static void
_sel_group(void *data, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   const char *groupname;

   groupname = elm_object_item_text_get(event_info);
   _request_chown_group(data, groupname);
}

static void
_sel_user(void *data, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   const char *username;

   username = elm_object_item_text_get(event_info);
   _request_chown_user(data, username);
}

static void
_group_hover_init(Evas_Object *obj, Evas_Object *o) {
   _standard_fill(obj, o, _sel_group, "/etc/group");
}

static void
_user_hover_init(Evas_Object *obj, Evas_Object *o) {
   _standard_fill(obj, o, _sel_user, "/etc/passwd");
}

//================================================================
//Permission settings
//================================================================
static int FILE_PERM_MODES[3][3] = {{S_IRUSR,S_IWUSR,S_IXUSR},
                                    {S_IRGRP,S_IWGRP,S_IXGRP},
                                    {S_IROTH,S_IWOTH,S_IXOTH}};
static void
_flip_update(Elm_File_Detail_Data *pd)
{

   Efm_File_Stat *st;

   if (elm_segment_control_item_selected_get(pd->perm2.segment) == pd->perm2.items[0]) return;

   st = efm_file_stat_get(pd->file);
   for (int i = 0; i < 3; i++)
     {
        if (st->mode & FILE_PERM_MODES[pd->perm2.user_type][i])
          elm_flipselector_item_selected_set(pd->perm2.pmodes[i], EINA_TRUE);
        else
          elm_flipselector_item_selected_set(pd->perm2.nmodes[i], EINA_TRUE);
     }
}

static void
_flip_cb(void *data, Evas_Object *obj, void *event_info)
{
   Elm_File_Detail_Data *pd;
   Efm_File_Stat *st;
   int flip = -1;
   mode_t permission;

   pd = efl_data_scope_get(data, ELM_FILE_DETAIL_CLASS);
   st = efm_file_stat_get(pd->file);
   if (!pd->changes.mode)
     permission = st->mode;
   else
     permission = pd->changes.mode;

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

   _request_chmod(data, permission);
}

static void
_init_flip_selectors(Eo *obj, Elm_File_Detail_Data *pd)
{
   char *lbl[] = {"r","w","x"};

   for (int i = 0; i < 3; i++)
     {
        pd->perm2.flip[i] = elm_flipselector_add(obj);
        pd->perm2.pmodes[i] = elm_flipselector_item_append(pd->perm2.flip[i], lbl[i], NULL, NULL);
        pd->perm2.nmodes[i] = elm_flipselector_item_append(pd->perm2.flip[i], "-", NULL, NULL);
        evas_object_smart_callback_add(pd->perm2.flip[i], "selected", _flip_cb, obj);
        elm_table_pack(pd->perm.change_display, pd->perm2.flip[i], i + 1, 1, 1, 1);
        evas_object_show(pd->perm2.flip[i]);
     }
}

static void
_segment_changed_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Elm_File_Detail_Data *pd;
   Eina_Bool flip_visible;

   pd = efl_data_scope_get(data, ELM_FILE_DETAIL_CLASS);

   flip_visible = pd->perm2.items[0] != event_info;

   efl_gfx_visible_set(pd->perm2.permstring, !flip_visible);

   if (!pd->perm2.flip[0] && flip_visible)
     {
        //we are not initializied yet, do it now
        _init_flip_selectors(data, pd);
     }

   for (int i = 0; i < 3; i++)
     efl_gfx_visible_set(pd->perm2.flip[i], flip_visible);

   if (flip_visible)
     {
       //check which part is active, owner, group or other
       for (int part = 1; part < 4; part ++)
         {
            if (pd->perm2.items[part] == event_info)
              {
                 //update the current usertype to the correct part
                 pd->perm2.user_type = part - 1;
              }
         }
       //update flips
       _flip_update(pd);
     }
}

static void
_permission_init(Evas_Object *obj, Elm_File_Detail_Data *pd) {
   char *segments[] = {"View", "Owner", "Group", "Other", NULL};

   pd->perm.change_display = elm_table_add(obj);

   pd->perm2.segment = elm_segment_control_add(obj);
   evas_object_smart_callback_add(pd->perm2.segment, "changed", _segment_changed_cb, obj);
   //init elements
   for (int i = 0; segments[i]; i++)
     pd->perm2.items[i] = elm_segment_control_item_add(pd->perm2.segment, NULL, segments[i]);
   elm_segment_control_item_selected_set(pd->perm2.items[0], EINA_TRUE);
   elm_table_pack(pd->perm.change_display, pd->perm2.segment, 0, 0, 5, 1);
   evas_object_show(pd->perm2.segment);

   pd->perm2.permstring = elm_label_add(obj);
   elm_table_pack(pd->perm.change_display, pd->perm2.permstring, 0, 1, 5, 1);

   //fake selection to select part 0
   _segment_changed_cb(obj, pd->perm2.segment, pd->perm2.items[0]);
}
//================================================================
//Detail setup
//================================================================

static void
detail_row_changable_changeable(Detail_Row_Mutable *row, Eina_Bool changeable)
{
   efl_gfx_visible_set(row->display, !changeable);
   efl_gfx_visible_set(row->change_display, changeable);
}

static void
detail_row_changable_init(Evas_Object *obj, Detail_Row_Mutable *row)
{
   row->table = elm_table_add(obj);
   evas_object_size_hint_align_set(row->table, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(row->table, EVAS_HINT_EXPAND, 0.0);

   row->display = elm_label_add(obj);
   evas_object_size_hint_align_set(row->display, 0.5, 0.5);
   evas_object_size_hint_weight_set(row->display, EVAS_HINT_EXPAND, 0.0);
   elm_object_text_set(row->display, "-");
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

static void
_setup_cb(void *data, const Efl_Event *info EINA_UNUSED)
{
   Elm_File_Detail_Data *pd;

   pd = efl_data_scope_get(data, ELM_FILE_DETAIL_CLASS);

   if (pd->havy_setup) return;

   pd->havy_setup = EINA_TRUE;

   _user_hover_init(data, pd->user.change_display);
   _group_hover_init(data, pd->group.change_display);

}

static void
_box_visible_set(Eo *bx, void *pd EINA_UNUSED, Eina_Bool v)
{
   Eina_List *lst, *node;
   Evas_Object *o;

   lst = elm_box_children_get(bx);
   EINA_LIST_FOREACH(lst, node, o)
     {
        if (v)
          evas_object_show(o);
        else
          evas_object_hide(o);
     }
   efl_gfx_visible_set(efl_super(bx, EFL_OBJECT_OVERRIDE_CLASS), v);
}

static void
_box_override(Eo *box)
{
   Efl_Object_Ops ops;
   Efl_Op_Description desc[] = {
    EFL_OBJECT_OP_FUNC(efl_gfx_visible_set, _box_visible_set),
   };

   ops.count = 1;
   ops.descs = desc;
   efl_object_override(box, &ops);
}

static Efm_File*
_rename(Efm_File *f, const char *filename)
{
   char buf[PATH_MAX];
   char *dir;
   const char *path;

   path = efm_file_path_get(f);
   dir = ecore_file_dir_get(path);

   snprintf(buf, sizeof(buf), "%s/%s", dir, filename);

   if (!!rename(path, buf))
     {
        perror("Failed to rename, reason");
        return NULL;
     }

   return efm_file_get(EFM_CLASS, buf);
}

static void
_key_down(void *data, const Efl_Event *ev)
{
   Elm_File_Detail_Data *pd;
   Efl_Input_Key *key;

   pd = efl_data_scope_get(data, ELM_FILE_DETAIL_CLASS);
   key = ev->info;

   if (!strcmp(efl_input_key_get(key), "Return"))
     {
        const char *newname;
        Efm_File *renamed;

        newname = elm_object_text_get(pd->name.change_display);

        renamed = _rename(elm_file_detail_file_get(data), newname);

        if (renamed)
          elm_file_detail_file_set(data, renamed);

        _switch_to_display(data);
     }
   else if (!strcmp(efl_input_key_get(key), "Escape"))
     {
        Efm_File *f;

        f = elm_file_detail_file_get(data);

        elm_object_text_set(pd->name.change_display, efm_file_filename_get(f));

        _switch_to_display(data);
     }
}

static void
_focus_out(void *data, const Efl_Event *event EINA_UNUSED)
{
   _switch_to_display(data);
}

static void
_name_init(Evas_Object *obj, Elm_Box *box)
{
   Elm_File_Detail_Data *pd;
   Evas_Object *table, *bx, *o, *ic;

   pd = efl_data_scope_get(obj, ELM_FILE_DETAIL_CLASS);

   table = elm_table_add(box);
   evas_object_size_hint_align_set(table, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(table, EVAS_HINT_EXPAND, 0.0);
   evas_object_show(table);

   pd->name.display = bx = elm_box_add(box);
   _box_override(bx);
   elm_box_horizontal_set(bx, EINA_TRUE);
   evas_object_size_hint_align_set(bx, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, 0.0);
   elm_table_pack(table, bx, 0, 0, 1, 1);
   evas_object_show(bx);

   ic = elm_icon_add(box);
   elm_icon_standard_set(ic, "document-new");
   evas_object_show(ic);

   pd->name.change_display = elm_entry_add(obj);
   efl_event_callback_add(pd->name.change_display, EFL_EVENT_KEY_DOWN, _key_down, obj);
   efl_event_callback_add(pd->name.change_display, ELM_WIDGET_EVENT_UNFOCUSED, _focus_out, obj);
   elm_entry_single_line_set(pd->name.change_display, EINA_TRUE);
   elm_entry_scrollable_set(pd->name.change_display, EINA_TRUE);
   evas_object_size_hint_align_set(pd->name.change_display, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(pd->name.change_display, EVAS_HINT_EXPAND, 0.0);
   elm_table_pack(table, pd->name.change_display, 0, 0, 1, 1);

   o = elm_button_add(box);
   elm_object_part_content_set(o, "icon", ic);
   evas_object_smart_callback_add(o, "clicked", _switch_to_edit_cb, obj);
   elm_box_pack_end(bx, o);
   evas_object_show(o);

   pd->name.display = elm_label_add(box);
   elm_label_ellipsis_set(pd->name.display, EINA_TRUE);
   evas_object_size_hint_align_set(pd->name.display, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(pd->name.display, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx, pd->name.display);
   evas_object_show(pd->name.display);

   elm_box_pack_end(box, table);
}

EOLIAN static void
_elm_file_detail_rename(Eo *obj, Elm_File_Detail_Data *pd EINA_UNUSED)
{
   _switch_to_edit(obj);
}


EOLIAN static Efl_Object*
_elm_file_detail_efl_object_constructor(Eo *obj, Elm_File_Detail_Data *pd)
{
   Eo *ret;
   Evas_Object *bx, *sc;

   ret = efl_constructor(efl_super(obj, ELM_FILE_DETAIL_CLASS));

   if (!elm_layout_theme_set(obj, "file_display", "file_preview", "default"))
     {
        CRI("Failed to set theme file");
     }

   bx = elm_box_add(obj);
   sc = elm_scroller_add(obj);
   elm_object_style_set(sc, "popup/no_inset_shadow");
   elm_object_content_set(sc, bx);

   SEPERATOR

   LABEL(pd->name.label, "<b>Name:", 0.0, EINA_FALSE);
   _name_init(obj, bx);
   SEPERATOR
   DETAIL_ROW_UNCHANGABLE(size, "<b>Size:");
   SEPERATOR
   pd->user.change_display = elm_hoversel_add(obj);
   efl_event_callback_add(pd->user.change_display, EFL_UI_EVENT_CLICKED, _setup_cb, obj);
   DETAIL_ROW_CHANGABLE(user, "<b>User:");
   SEPERATOR
   pd->group.change_display = elm_hoversel_add(obj);
   efl_event_callback_add(pd->group.change_display, EFL_UI_EVENT_CLICKED, _setup_cb, obj);
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

   elm_object_part_content_set(obj, "content", sc);

   return ret;
}

#include "elm_file_detail.eo.x"
