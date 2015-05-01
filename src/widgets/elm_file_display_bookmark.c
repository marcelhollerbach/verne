#include "elm_file_display_priv.h"

typedef struct
{
   const char *dir;
   const char *name;
   const char *icon;
   Eina_Bool removable;
} Bookmark;


static char *
_item_label_get(void *data, Evas_Object *obj EINA_UNUSED, const char *part)
{
   Bookmark *b = data;

   if (!strcmp(part, "elm.text"))
     return strdup(b->name);
   return NULL;
}

static Evas_Object *
_item_content_get(void *data, Evas_Object *obj, const char *part)
{
   Bookmark *b = data;
   Evas_Object *ic;

   if (!!strcmp(part, "elm.swallow.icon")) return NULL;

   ic = elm_icon_add(obj);
   elm_icon_standard_set(ic, b->icon);
   evas_object_show(ic);
   return ic;
}

static void
_item_del(void *data, Evas_Object *obj EINA_UNUSED)
{
    Bookmark *b = data;

    eina_stringshare_del(b->dir);
    eina_stringshare_del(b->name);
    eina_stringshare_del(b->icon);

    free(b);
}

static void
_group_del(void *data, Evas_Object *obj EINA_UNUSED)
{
   eina_stringshare_del(data);
}

static void
_item_sel(void *data EINA_UNUSED, Evas_Object *obj, void *event)
{
   Elm_Object_Item *it = event;
   Evas_Object *w;
   Bookmark *b;

   w = elm_object_parent_widget_get(obj);
   b = elm_object_item_data_get(it);

   eo_do(w,
        efl_file_set(b->dir, NULL);
        eo_event_callback_call(&_ELM_FILE_DISPLAY_EVENT_PATH_CHANGED, (void*)b->dir);
       );
}


static char *
_group_label_get(void *data, Evas_Object *obj EINA_UNUSED, const char *part EINA_UNUSED)
{
   return strdup(data);
}


static char *
_dnd_item_label_get(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, const char *part EINA_UNUSED)
{
   return strdup("...Drop here...");
}

static void
_dnd_enter(void *data, Evas_Object *obj)
{
   Elm_Object_Item *it;

   it = evas_object_data_get(obj, "drop_item");
   if (it) return;

   it = elm_genlist_item_append(obj, data, NULL, NULL, 0, NULL, NULL);
   evas_object_data_set(obj, "drop_item", it);
}

static void
_dnd_leave(void *data EINA_UNUSED, Evas_Object *obj)
{
   Elm_Object_Item *it;

   it = evas_object_data_get(obj, "drop_item");
   if (!it) return;
   elm_object_item_del(it);
   evas_object_data_del(obj, "drop_item");
}

void
bookmark_entry_internal_add(Evas_Object *w, const char *dir,
                            const char *name, const char *icon, Eina_Bool removable,
                            Elm_Object_Item *par)
{
   Bookmark *b;
   Elm_Gengrid_Item_Class *ic;

   ic = evas_object_data_get(w, "__ic");

   b = calloc(1, sizeof(Bookmark));
   b->icon = eina_stringshare_add(icon);
   b->dir = eina_stringshare_add(dir);
   b->name = eina_stringshare_add(name);
   b->removable = removable;

   elm_genlist_item_append(w, ic, b, par, ELM_GENLIST_ITEM_NONE, _item_sel, b);
}

void
bookmark_entry_add(Evas_Object *w, const char *dir,
                   const char *name, const char *icon)
{
   bookmark_entry_internal_add(w, dir, name, icon, EINA_TRUE, NULL);
   helper_bookmarks_add(dir);
}

void
_setup_list(Evas_Object *o)
{
   const char *tmp;
   Eina_List *node;
   Elm_Object_Item *it;
   Elm_Genlist_Item_Class *gr;

   elm_genlist_clear(o);

   gr = evas_object_data_get(o, "__gr_ic");

   it = elm_genlist_item_append(o, gr, eina_stringshare_add("Places"), NULL, ELM_GENLIST_ITEM_GROUP, NULL, NULL);

   bookmark_entry_internal_add(o, "/", "Root", "start-here", EINA_FALSE, it);

   if ((tmp = getenv("HOME")))
     bookmark_entry_internal_add(o, tmp, "Home", "user-home", EINA_FALSE, it);

   if ((tmp = efreet_trash_dir_get(NULL)))
     bookmark_entry_internal_add(o, tmp, "Trash", "user-trash", EINA_FALSE, it);

   if ((tmp = efreet_desktop_dir_get()))
     bookmark_entry_internal_add(o, tmp, "Desktop", "user-desktop", EINA_FALSE, it);

   it = elm_genlist_item_append(o, gr, eina_stringshare_add("Devices"), NULL, ELM_GENLIST_ITEM_GROUP, NULL, NULL);
   //TODO FIXME FIXME

   it = elm_genlist_item_append(o, gr, eina_stringshare_add("Bookmarks"), NULL, ELM_GENLIST_ITEM_GROUP, NULL, NULL);

   if (!config) return;

   //standart bookmarks
   EINA_LIST_FOREACH(config->bookmarks, node, tmp)
     {
        bookmark_entry_internal_add(o, tmp, ecore_file_file_get(tmp), NULL, EINA_TRUE, it);
     }

   if(!config->display_gtk) return;

   Eina_List *lst = util_bookmarks_load_gtk();

   EINA_LIST_FREE(lst, tmp)
     {
        bookmark_entry_internal_add(o, tmp, ecore_file_file_get(tmp), NULL, EINA_FALSE, it);
        eina_stringshare_del(tmp);
     }

}

Eina_Bool
_dnd_droped(void *data EINA_UNUSED, Evas_Object *obj, Elm_Selection_Data *ev)
{
   const char *rawdata = ev->data;
   Eina_List *list = NULL, *node;
   char buf[PATH_MAX];
   int c = 0;
   int lm = 0;

   while(rawdata[c] != '\0')
     {
        if (rawdata[c] == '\n')
          {
             const char *newbookmark;

             buf[c - lm] = '\0';
             newbookmark = eina_stringshare_add(buf+FILESEP_LEN);
             list = eina_list_append(list, newbookmark);
             lm = c+1;
          }
        else
          {
             buf[c - lm] = rawdata[c];
          }
        c++;
     }

   EINA_LIST_FOREACH(list, node, rawdata)
     {
        if (ecore_file_is_dir(rawdata))
          bookmark_entry_add(obj, rawdata,
                             ecore_file_file_get(rawdata), NULL);
        _setup_list(obj);

        eina_stringshare_del(rawdata);
     }
   _dnd_leave(NULL, obj);
   return EINA_FALSE;
}

static void
_ctx_del(void *data, Evas_Object *obj, void *event EINA_UNUSED)
{
   Bookmark *todel = data;
   Evas_Object *list;

   list = evas_object_data_get(obj, "_genlist");
   helper_bookmarks_del(todel->dir);
   _setup_list(list);
}

static void
_ctx_gtk(void *data, Evas_Object *obj EINA_UNUSED, void *event)
{
  Eina_Bool b;
  Evas_Object *check;

  check = elm_object_item_content_get(event);
  b = elm_check_state_get(check);
  elm_check_state_set(check, !b);
  if (config)
    config->display_gtk = !b;
  _setup_list(data);
  config_save();
}

static void
_ctx_menu(Evas_Object *w, int x, int y, Bookmark *b)
{
  Evas_Object *menu, *o;
  Elm_Object_Item *it;


  menu = elm_menu_add(elm_object_top_widget_get(w));
  evas_object_data_set(menu, "_genlist", w);
  //TODO call callback for before

  if (b->removable)
    elm_menu_item_add(menu, NULL, "edit-delete" , "Del Bookmark", _ctx_del, b);

  o = elm_check_add(w);
  if (config)
    elm_check_state_set(o, config->display_gtk);
  elm_object_text_set(o, "Show GTK bookmarks");

  it = elm_menu_item_add(menu, NULL, NULL, NULL, _ctx_gtk, w);
  elm_object_item_content_set(it, o);

  //TODO call callback for after

  elm_menu_move(menu, x, y);
  evas_object_show(menu);
}

static void
_right_click(void *data EINA_UNUSED, Evas_Object *obj, void *event_data)
{
   Elm_Object_Item *it;
   Bookmark *b;
   int x,y;

   it = event_data;
   b = elm_object_item_data_get(it);
   evas_pointer_canvas_xy_get(evas_object_evas_get(obj), &x, &y);
   _ctx_menu(obj, x, y, b);
}

static void
_del(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *obj, void *event EINA_UNUSED)
{
   Elm_Gengrid_Item_Class *dnd_ic, *ic, *gr;


   ic = evas_object_data_get(obj, "__ic");
   dnd_ic = evas_object_data_get(obj, "__dnd_ic");
   gr = evas_object_data_get(obj, "__gr_ic");

   elm_genlist_item_class_free(dnd_ic);
   elm_genlist_item_class_free(ic);
   elm_genlist_item_class_free(gr);
}

Evas_Object*
bookmark_add(Evas_Object *w)
{
   Evas_Object *o;
   Elm_Gengrid_Item_Class *dnd_ic;
   Elm_Gengrid_Item_Class *ic;

   o = elm_genlist_add(w);
   ic = elm_genlist_item_class_new();
   evas_object_event_callback_add(o, EVAS_CALLBACK_DEL, _del, NULL);
   evas_object_smart_callback_add(o, "clicked,right", _right_click, NULL);
   dnd_ic = elm_genlist_item_class_new();

   dnd_ic = elm_genlist_item_class_new();
   dnd_ic->item_style = "default";
   dnd_ic->func.text_get = _dnd_item_label_get;
   dnd_ic->func.content_get = NULL;
   dnd_ic->func.state_get = NULL;
   dnd_ic->func.del = NULL;

   evas_object_data_set(o, "__dnd_ic", ic);

   ic = elm_genlist_item_class_new();
   ic->item_style = "default";
   ic->func.text_get = _item_label_get;
   ic->func.content_get = _item_content_get;
   ic->func.state_get = NULL;
   ic->func.del = _item_del;

   //setup drop zone
   elm_drop_target_add(o, ELM_SEL_FORMAT_TARGETS,
                       _dnd_enter, dnd_ic, _dnd_leave,
                       dnd_ic, NULL, NULL,
                       _dnd_droped, ic);

   //setup group class
   evas_object_data_set(o, "__ic", ic);

   ic = elm_genlist_item_class_new();
   ic->item_style = "group_index";
   ic->func.text_get = _group_label_get;
   ic->func.content_get = NULL;
   ic->func.state_get = NULL;
   ic->func.del = _group_del;

   evas_object_data_set(o, "__gr_ic", ic);

   _setup_list(o);

   evas_object_show(o);
   return o;
}