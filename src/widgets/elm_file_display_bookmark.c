#include "elm_file_display_priv.h"

#define BOOKMARK_TYPE_DIRECTORY 0
#define BOOKMARK_TYPE_DEVICE 1
#define PRIV_DATA(o) Bookmark_Pd *pd = evas_object_data_get(o,"__bookmark_pd");

typedef struct
{
   Eina_Bool removable;
   const char *dir;
   const char *name;
   const char *icon;
} Directory;

typedef struct
{
   Emous_Device *d;
   Evas_Object *indicator;
} Device;

typedef struct
{
   int type;
   union {
      Directory dir;
      Device dev;
   } pd;
} Bookmark_Item;

typedef struct {
   Elm_Genlist_Item_Class *dnd_ic, *place_item, *device_item, *group_item;
   Elm_Genlist_Item *bookmark_group_it;
   Elm_Genlist_Item *device_group_it;
} Bookmark_Pd;

static void _item_sel(void *data, Evas_Object *obj, void *event);
static void _device_update(Bookmark_Item *d);
static Bookmark_Item* _device_add(Emous_Device *d, Evas_Object *w);

//============================================================================================
// Helper functions
//============================================================================================

void
bookmark_entry_internal_add(Evas_Object *w, const char *dir,
                            const char *name, const char *icon, Eina_Bool removable,
                            Elm_Object_Item *par)
{
   Bookmark_Item *b;
   Directory *d;
   PRIV_DATA(w);

   b = calloc(1, sizeof(Bookmark_Item));
   b->type = BOOKMARK_TYPE_DIRECTORY;

   d = &b->pd.dir;
   d->icon = eina_stringshare_add(icon);
   d->dir = eina_stringshare_add(dir);
   d->name = eina_stringshare_add(name);
   d->removable = removable;

   elm_genlist_item_append(w, pd->place_item, b, par, ELM_GENLIST_ITEM_NONE, _item_sel, &b->pd.dir);
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

   PRIV_DATA(o);
   elm_genlist_clear(o);

   it = elm_genlist_item_append(o, pd->group_item, eina_stringshare_add("Places"), NULL, 0, NULL, NULL);

   bookmark_entry_internal_add(o, "/", "Root", "start-here", EINA_FALSE, it);

   if ((tmp = getenv("HOME")))
     bookmark_entry_internal_add(o, tmp, "Home", "user-home", EINA_FALSE, it);

   if ((tmp = efreet_trash_dir_get(NULL)))
     bookmark_entry_internal_add(o, tmp, "Trash", "user-trash", EINA_FALSE, it);

   if ((tmp = efreet_desktop_dir_get()))
     bookmark_entry_internal_add(o, tmp, "Desktop", "user-desktop", EINA_FALSE, it);

   pd->bookmark_group_it = elm_genlist_item_append(o, pd->group_item, eina_stringshare_add("Bookmarks"), NULL, 0, NULL, NULL);

   if (!config) return;

   //standart bookmarks
   EINA_LIST_FOREACH(config->bookmarks, node, tmp)
     {
        bookmark_entry_internal_add(o, tmp, ecore_file_file_get(tmp), "folder", EINA_TRUE, pd->bookmark_group_it);
     }

   if(!config->display_gtk) goto device;

   Eina_List *lst = util_bookmarks_load_gtk();

   EINA_LIST_FREE(lst, tmp)
     {
        bookmark_entry_internal_add(o, tmp, ecore_file_file_get(tmp), "folder", EINA_FALSE, pd->bookmark_group_it);
        eina_stringshare_del(tmp);
     }
device:

   pd->device_group_it = it = elm_genlist_item_append(o, pd->group_item, eina_stringshare_add("Devices"), NULL, 0, NULL, NULL);

   {
      Eina_List *list, *node;
      Emous_Device *device;

      eo_do(EMOUS_MANAGER_CLASS, list = emous_manager_devices_get());

      EINA_LIST_FOREACH(list, node, device)
        {
           Bookmark_Item *it;
           it = _device_add(device, o);
           _device_update(it);
        }
   }

}

//============================================================================================
// Device Item Genlist Implements
//============================================================================================

static void
_device_item_sel(void *data, Evas_Object *obj, void *event EINA_UNUSED)
{
   Device *d = data;
   const char *mountpoint;

   eo_do(d->d, if (!emous_device_mountpoints_get())
                 mountpoint = NULL;
               else
                 mountpoint = eina_list_data_get(emous_device_mountpoints_get());
         );

   if (mountpoint)
     {
        Evas_Object *w;

        w = elm_object_parent_widget_get(obj);

        eo_do(w,
           efl_file_set(mountpoint, NULL);
           eo_event_callback_call(&_ELM_FILE_DISPLAY_EVENT_PATH_CHANGED, (void*)mountpoint);
       );
     }
   else
     {
        //TODO add a option pane which asks if you want to mount this device
     }
}

static char *
_device_item_text_get(void *data, Evas_Object *obj EINA_UNUSED, const char *part EINA_UNUSED)
{
   Bookmark_Item *it = data;
   Emous_Device *d = it->pd.dev.d;
   const char *displayname;

   eo_do(d, displayname = emous_device_displayname_get());
   if (!displayname)
     displayname = "<No Name>";

   return strdup(displayname);
}

static Evas_Object *
_device_item_content_get(void *data, Evas_Object *obj, const char *part EINA_UNUSED)
{
   Bookmark_Item *it = data;
   Device *d = &it->pd.dev;
   Evas_Object *indicator;

   if (!!strcmp(part, "elm.swallow.icon")) return NULL;

   d->indicator = indicator = elm_layout_add(obj);

   if (!elm_layout_theme_set(indicator, "file_display", "mount_indicator", "default"))
     ERR("Failed to set indicator file");

   _device_update(it);

   return indicator;
}

static void
_device_item_del(void *data, Evas_Object *obj EINA_UNUSED)
{
   free(data);
}

//============================================================================================
// Directory Item Genlist Implements
//============================================================================================

static char *
_item_label_get(void *data, Evas_Object *obj EINA_UNUSED, const char *part)
{
   Bookmark_Item *item = data;
   Directory *b = &item->pd.dir;

   if (!strcmp(part, "elm.text"))
     return strdup(b->name);
   return NULL;
}

static Evas_Object *
_item_content_get(void *data, Evas_Object *obj, const char *part)
{
   Bookmark_Item *item = data;
   Directory *b = &item->pd.dir;
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
   Bookmark_Item *item = data;
   Directory *b = &item->pd.dir;

   eina_stringshare_del(b->dir);
   eina_stringshare_del(b->name);
   eina_stringshare_del(b->icon);

   free(item);
}

//============================================================================================
// Group Item Genlist Implements
//============================================================================================

static void
_group_del(void *data, Evas_Object *obj EINA_UNUSED)
{
   eina_stringshare_del(data);
}

static void
_item_sel(void *data, Evas_Object *obj, void *event EINA_UNUSED)
{
   Evas_Object *w;
   Directory *b = data;

   w = elm_object_parent_widget_get(obj);

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

//============================================================================================
// Drag and drop stuff
//============================================================================================

static void
_dnd_enter(void *data EINA_UNUSED, Evas_Object *obj)
{
   Elm_Object_Item *it;
   PRIV_DATA(obj);

   it = evas_object_data_get(obj, "drop_item");
   if (it) return;

   it = elm_genlist_item_append(obj, pd->dnd_ic, NULL, pd->bookmark_group_it, 0, NULL, NULL);
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


        eina_stringshare_del(rawdata);
     }
   _dnd_leave(NULL, obj);
   _setup_list(obj);
   return EINA_FALSE;
}

//============================================================================================
// Device handling stuff
//============================================================================================

static void
_device_update(Bookmark_Item *item)
{
   Device_State state;
   const char *signal;

   eo_do(item->pd.dev.d, state = emous_device_state_get());

   switch(state)
     {
        case DEVICE_STATE_MOUNTED:
          signal = "mounted";
        break;
        case DEVICE_STATE_UMOUNTED:
          signal = "unmounted";
        break;
        case DEVICE_STATE_MOUNTREQ:
        case DEVICE_STATE_UMOUNTREQ:
          signal = "mountrequest";
        break;
     }
   elm_layout_signal_emit(item->pd.dev.indicator, signal, "elm");

}

static Eina_Bool
_state_changed(void *data, Eo *obj EINA_UNUSED, const Eo_Event_Description *desc EINA_UNUSED, void *event EINA_UNUSED)
{
   Bookmark_Item *item = data;
   _device_update(item);

   return EINA_TRUE;
}

static Bookmark_Item*
_device_add(Emous_Device *d, Evas_Object *w)
{
  Bookmark_Item *it;
  PRIV_DATA(w);
  it = calloc(1, sizeof(Bookmark_Item));

  it->type = BOOKMARK_TYPE_DEVICE;
  it->pd.dev.d = d;

  eo_do(d, eo_event_callback_add(EMOUS_DEVICE_EVENT_STATE_CHANGED, _state_changed, it););

  elm_genlist_item_append(w, pd->device_item, it, pd->device_group_it, 0, _device_item_sel, &it->pd.dev.d);

  return it;
}

static Eina_Bool
_device_add_cb(void *data, Eo *obj EINA_UNUSED, const Eo_Event_Description *desc EINA_UNUSED, void *event EINA_UNUSED)
{
  Emous_Device *d = event;

  _device_add(d, data);

  return EINA_TRUE;
}

static Eina_Bool
_device_del_cb(void *data, Eo *obj EINA_UNUSED, const Eo_Event_Description *desc EINA_UNUSED, void *event EINA_UNUSED)
{
   _setup_list(data);

   return EINA_TRUE;
}

//============================================================================================
// Contextmenu for directory functions
//============================================================================================

static void
_ctx_del(void *data, Evas_Object *obj, void *event EINA_UNUSED)
{
   Directory *todel = data;
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
_ctx_menu_directory(Evas_Object *w, int x, int y, Directory *b)
{
  Evas_Object *menu, *o;
  Elm_Object_Item *it;

  //create new menu
  menu = elm_menu_add(elm_object_top_widget_get(w));
  //save the widget
  evas_object_data_set(menu, "_genlist", w);

  eo_do(w, eo_event_callback_call(ELM_FILE_DISPLAY_EVENT_HOOK_MENU_BOOKMARKS_START , menu));

  //if we are removable remove it
  if (b->removable)
    elm_menu_item_add(menu, NULL, "edit-delete" , "Del Bookmark", _ctx_del, b);

  //option to display or hide gtk
  o = elm_check_add(w);
  if (config)
    elm_check_state_set(o, config->display_gtk);
  elm_object_text_set(o, "Show GTK bookmarks");

  it = elm_menu_item_add(menu, NULL, NULL, NULL, _ctx_gtk, w);
  elm_object_item_content_set(it, o);

  eo_do(w, eo_event_callback_call(ELM_FILE_DISPLAY_EVENT_HOOK_MENU_BOOKMARKS_END, menu));

  //move and show
  elm_menu_move(menu, x, y);
  evas_object_show(menu);
}

//============================================================================================
// Contextmenu for device functions
//============================================================================================
static void
_unmount_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_data EINA_UNUSED)
{
   eo_do(data, emous_device_umount());
}
  static void
_mount_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_data EINA_UNUSED)
{
   eo_do(data, emous_device_mount());
}

static void
_ctx_menu_device(Evas_Object *w, int x, int y, Device *b)
{
   Evas_Object *menu;
   Eina_Bool mounted;
   //create menu
   menu = elm_menu_add(elm_object_top_widget_get(w));
   //save the widget
   evas_object_data_set(menu, "_genlist", w);

   eo_do(b->d, mounted = !!emous_device_mountpoints_get());

   eo_do(w, eo_event_callback_call(ELM_FILE_DISPLAY_EVENT_HOOK_MENU_DEVICE_START, menu));

   elm_menu_item_separator_add(menu, NULL);

   if (mounted)
     elm_menu_item_add(menu, NULL, NULL, "Unmounted", _unmount_cb, b->d);
   else
     elm_menu_item_add(menu, NULL, NULL, "Mount", _mount_cb, b->d);

   elm_menu_item_separator_add(menu, NULL);

   eo_do(w, eo_event_callback_call(ELM_FILE_DISPLAY_EVENT_HOOK_MENU_DEVICE_END, menu));

   elm_menu_move(menu, x, y);
   evas_object_show(menu);
}

static void
_right_click(void *data EINA_UNUSED, Evas_Object *obj, void *event_data)
{
   Elm_Object_Item *it;
   Bookmark_Item *b;
   int x,y;

   it = event_data;
   //get Bookmark of this item
   b = elm_object_item_data_get(it);
   //get mous position
   evas_pointer_canvas_xy_get(evas_object_evas_get(obj), &x, &y);
   //open menu
   if (b->type == BOOKMARK_TYPE_DIRECTORY)
     _ctx_menu_directory(obj, x, y, &(b->pd.dir));
   else
     _ctx_menu_device(obj, x, y, &(b->pd.dev));
}

//============================================================================================
// Widget functions
//============================================================================================

static void
_del(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *obj, void *event EINA_UNUSED)
{
   PRIV_DATA(obj);

   elm_genlist_item_class_free(pd->place_item);
   elm_genlist_item_class_free(pd->device_item);
   elm_genlist_item_class_free(pd->dnd_ic);
   elm_genlist_item_class_free(pd->group_item);

   free(pd);
}

Evas_Object*
bookmark_add(Evas_Object *w)
{
   Evas_Object *o;
   Emous_Manager *m;

   Bookmark_Pd *pd;

   pd = calloc(1, sizeof(Bookmark_Pd));

   o = elm_genlist_add(w);
   elm_object_style_set(o, "file_display");
   evas_object_data_set(o, "__bookmark_pd", pd);

   evas_object_event_callback_add(o, EVAS_CALLBACK_DEL, _del, NULL);
   evas_object_smart_callback_add(o, "clicked,right", _right_click, NULL);

   pd->dnd_ic = elm_genlist_item_class_new();
   pd->dnd_ic->item_style = "default";
   pd->dnd_ic->func.text_get = _dnd_item_label_get;

   pd->place_item = elm_genlist_item_class_new();
   pd->place_item->item_style = strdup("default");
   pd->place_item->func.text_get = _item_label_get;
   pd->place_item->func.content_get = _item_content_get;
   pd->place_item->func.del = _item_del;

   pd->group_item = elm_genlist_item_class_new();
   pd->group_item->item_style = strdup("group_index");
   pd->group_item->func.text_get = _group_label_get;
   pd->group_item->func.del = _group_del;

   pd->device_item = elm_genlist_item_class_new();
   pd->device_item->item_style = strdup("default");
   pd->device_item->func.content_get = _device_item_content_get;
   pd->device_item->func.text_get = _device_item_text_get;
   pd->device_item->func.del = _device_item_del;

   //setup drop zone
   elm_drop_target_add(o, ELM_SEL_FORMAT_TARGETS,
                       _dnd_enter, NULL, _dnd_leave,
                       NULL, NULL, NULL,
                       _dnd_droped, NULL);

   //subscribe to emous device events
   eo_do(EMOUS_MANAGER_CLASS, m = emous_manager_object_get());
   eo_do(m, eo_event_callback_add(EMOUS_MANAGER_EVENT_DEVICE_ADD, _device_add_cb, o);
            eo_event_callback_add(EMOUS_MANAGER_EVENT_DEVICE_DEL, _device_del_cb, o););

   _setup_list(o);

   evas_object_show(o);
   return o;
}