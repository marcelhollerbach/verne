#include "../elementary_ext_priv.h"

#define PRIV_DATA(o) Elm_File_Bookmarks_Data *pd = eo_data_scope_get(o, ELM_FILE_BOOKMARKS_CLASS);

typedef struct {
   int ref;
   Elm_Genlist_Item_Class *dnd_item_class, *place_item_class, *device_item_class, *group_item_class;
} Elm_File_Bookmarks_Sd;

typedef struct {
   Elm_File_MimeType_Cache *cache;
   Elm_Genlist_Item *bookmark_group_it;
   Elm_Genlist_Item *device_group_it;
   const char *path; //<the current path, if in icons, the item will be selected
   Eina_Hash *icons;//< stringshared hash table with the items as data
}Elm_File_Bookmarks_Data;

typedef enum {
    BOOKMARK_TYPE_DEVICE,
    BOOKMARK_TYPE_DIRECTORY
} Bookmark_Item_Type;

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
   Bookmark_Item_Type type;
   union {
      Directory dir;
      Device dev;
   } pd;
} Bookmark_Item;

static Elm_File_Bookmarks_Sd *sd ;

static void _item_sel(void *data, Evas_Object *obj, void *event);
static void _device_update(Bookmark_Item *d);
static Bookmark_Item* _device_add(Emous_Device *d, Evas_Object *w);

EOLIAN static Eina_Bool
_elm_file_bookmarks_efl_file_file_set(Eo *obj, Elm_File_Bookmarks_Data *pd, const char *file, const char *key EINA_UNUSED)
{
   Elm_Widget_Item *it;

   eina_stringshare_replace(&pd->path, file);
   it = eina_hash_find(pd->icons, pd->path);

   if (it)
     elm_genlist_item_selected_set(it, EINA_TRUE);
   else
     {
        it = elm_genlist_selected_item_get(obj);
        elm_genlist_item_selected_set(it, EINA_FALSE);
     }

   return EINA_TRUE;
}

EOLIAN static void
_elm_file_bookmarks_efl_file_file_get(Eo *obj EINA_UNUSED, Elm_File_Bookmarks_Data *pd, const char **file, const char **key EINA_UNUSED)
{
   if (file)
     *file = pd->path;
}

static void
_file_change(Evas_Object *obj, const char *dir)
{
   efl_file_set(obj, dir, NULL);
   eo_event_callback_call(obj, ELM_FILE_BOOKMARKS_EVENT_PATH_SELECTED, NULL);

}

//============================================================================================
// Helper functions
//============================================================================================

static void
bookmark_entry_internal_add(Evas_Object *obj, Elm_File_Bookmarks_Data *pd,
                            const char *dir, const char *name, const char *icon,
                            Eina_Bool removable, Elm_Object_Item *par)
{
   Bookmark_Item *b;
   Elm_Widget_Item *it;
   Directory *d;

   b = calloc(1, sizeof(Bookmark_Item));
   b->type = BOOKMARK_TYPE_DIRECTORY;

   d = &b->pd.dir;
   d->icon = eina_stringshare_add(icon);
   d->dir = eina_stringshare_add(dir);
   d->name = eina_stringshare_add(name);
   d->removable = removable;

   it = elm_genlist_item_append(obj, sd->place_item_class, b, par, ELM_GENLIST_ITEM_NONE, _item_sel, &b->pd.dir);
   eina_hash_add(pd->icons, d->dir, it);
}

static void
bookmark_entry_add(Evas_Object *obj, Elm_File_Bookmarks_Data *pd,
                   const char *dir, const char *name, const char *icon)
{
   bookmark_entry_internal_add(obj, pd, dir, name, icon, EINA_TRUE, NULL);
   helper_bookmarks_add(dir);
}

static void
_setup_list(Evas_Object *o)
{
   const char *tmp;
   Eina_List *node;
   Elm_Object_Item *it;

   PRIV_DATA(o);
   elm_genlist_clear(o);
   eina_hash_free(pd->icons);
   pd->icons = eina_hash_stringshared_new(NULL);

   it = elm_genlist_item_append(o, sd->group_item_class, eina_stringshare_add("Places"), NULL, 0, NULL, NULL);

   bookmark_entry_internal_add(o, pd, "/", "Root", "start-here", EINA_FALSE, it);

   if ((tmp = getenv("HOME")))
     bookmark_entry_internal_add(o, pd, tmp, "Home", "user-home", EINA_FALSE, it);

   if ((tmp = efreet_trash_dir_get(NULL)))
     bookmark_entry_internal_add(o, pd, tmp, "Trash", "user-trash", EINA_FALSE, it);

   if ((tmp = efreet_desktop_dir_get()))
     bookmark_entry_internal_add(o, pd, tmp, "Desktop", "user-desktop", EINA_FALSE, it);

   pd->bookmark_group_it = elm_genlist_item_append(o, sd->group_item_class, eina_stringshare_add("Bookmarks"), NULL, 0, NULL, NULL);

   if (!config) return;

   // standard bookmarks
   EINA_LIST_FOREACH(config->bookmarks, node, tmp)
     {
        bookmark_entry_internal_add(o, pd, tmp, ecore_file_file_get(tmp), "folder", EINA_TRUE, pd->bookmark_group_it);
     }

   if(!config->display_gtk) goto device;

   Eina_List *lst = util_bookmarks_load_gtk();

   EINA_LIST_FREE(lst, tmp)
     {
        bookmark_entry_internal_add(o, pd, tmp, ecore_file_file_get(tmp), "folder", EINA_FALSE, pd->bookmark_group_it);
        eina_stringshare_del(tmp);
     }
device:

   pd->device_group_it = it = elm_genlist_item_append(o, sd->group_item_class, eina_stringshare_add("Devices"), NULL, 0, NULL, NULL);

   {
      Eina_List *list, *node;
      Emous_Device *device;

      list = emous_manager_devices_get(EMOUS_MANAGER_CLASS);

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
   Eina_List *mountpoints;

   mountpoints = emous_device_mountpoints_get(d->d);

   if (mountpoints)
     {
        //TODO call file changed
        _file_change(obj, eina_list_data_get(mountpoints));
     }
   else
     {
        // TODO add a option pane which asks if you want to mount this device
     }
}

static char *
_device_item_text_get(void *data, Evas_Object *obj EINA_UNUSED, const char *part EINA_UNUSED)
{
   Bookmark_Item *it = data;

   return emous_util_device_name_get(EMOUS_CLASS, it->pd.dev.d);
}

static Evas_Object *
_device_item_content_get(void *data, Evas_Object *obj, const char *part EINA_UNUSED)
{
   Bookmark_Item *it = data;
   Device *d = &it->pd.dev;
   Evas_Object *indicator, *icon;
   Emous_Device_Type type;
   const char *iconname;

   if (!!strcmp(part, "elm.swallow.icon")) return NULL;

   type = emous_device_type_get(d->d);

   switch(type)
     {
        case EMOUS_DEVICE_TYPE_DISK:
           iconname = "drive-harddisk";
        break;
        case EMOUS_DEVICE_TYPE_NETWORK:
           iconname = "network-wireless"; // XXX: here is no way to get just a simple "network"
        break;
        case EMOUS_DEVICE_TYPE_FLOPPY:
           iconname = "drive-floppy";
        break;
        case EMOUS_DEVICE_TYPE_REMOVABLE:
           iconname = "media-flash";
        break;
        case EMOUS_DEVICE_TYPE_CD:
           iconname = "drive-optical";
        break;
        default:
          iconname = "computer"; // Better show some thing shitty than nothing ...
        break;
     }

   indicator = elm_layout_add(obj);
   eo_wref_add(indicator, &d->indicator);

   icon = elm_icon_add(indicator);
   elm_icon_order_lookup_set(icon, ELM_ICON_LOOKUP_FDO_THEME);
   elm_icon_standard_set(icon, iconname);
   evas_object_show(icon);

   if (!elm_layout_theme_set(indicator, "file_display", "mount_indicator", "default"))
     ERR("Failed to set indicator file");

   elm_object_part_content_set(indicator, "icon", icon);
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
   Elm_File_Bookmarks_Data *pd;

   pd = eo_data_scope_get(obj, ELM_FILE_BOOKMARKS_CLASS);

   if (!!strcmp(part, "elm.swallow.icon")) return NULL;

   ic = elm_icon_add(obj);
   elm_icon_order_lookup_set(ic, ELM_ICON_LOOKUP_FDO_THEME);
   elm_file_mimetype_cache_mimetype_set(pd->cache, ic, b->icon);
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
   Directory *d = data;
   _file_change(obj, d->dir);
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

   it = elm_genlist_item_append(obj, sd->dnd_item_class, NULL, pd->bookmark_group_it, 0, NULL, NULL);
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

static Eina_Bool
_dnd_droped(void *data EINA_UNUSED, Evas_Object *obj, Elm_Selection_Data *ev)
{
   char **splits;
   PRIV_DATA(obj);
   splits = eina_str_split(ev->data, "\n", 0);

   for (int i = 0; splits[i]; i++)
     {
        // parse out the name
      if (ecore_file_is_dir(splits[i]))
        {
           bookmark_entry_add(obj, pd, splits[i], ecore_file_file_get(splits[i]), NULL);
        }
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
   Emous_Device_State state;
   const char *signal;

   state = emous_device_state_get(item->pd.dev.d);

   switch(state)
     {
        case EMOUS_DEVICE_STATE_MOUNTED:
          signal = "mounted";
        break;
        case EMOUS_DEVICE_STATE_UMOUNTED:
          signal = "unmounted";
        break;
        case EMOUS_DEVICE_STATE_MOUNT_REQ:
        case EMOUS_DEVICE_STATE_UNMOUNT_REQ:
          signal = "mountrequest";
        break;
        default:
          ERR("Device state not known");
          break;
     }
   if (item->pd.dev.indicator)
     elm_layout_signal_emit(item->pd.dev.indicator, signal, "elm");
}

static Eina_Bool
_state_changed(void *data, const Eo_Event *event EINA_UNUSED)
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

  eo_event_callback_add(d, EMOUS_DEVICE_EVENT_STATE_CHANGED, _state_changed, it);

  elm_genlist_item_append(w, sd->device_item_class, it, pd->device_group_it, 0, _device_item_sel, &it->pd.dev.d);

  return it;
}

static Eina_Bool
_device_add_cb(void *data, const Eo_Event *event)
{
  Emous_Device *d = event->info;

  _device_add(d, data);

  return EINA_TRUE;
}

static Eina_Bool
_device_del_cb(void *data, const Eo_Event *event EINA_UNUSED)
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
  elm_ext_config_save();
}

static void
_ctx_menu_directory(Evas_Object *w, int x, int y, Directory *b)
{
  Evas_Object *menu, *o;
  Elm_Object_Item *it;

  // create new menu
  menu = elm_menu_add(elm_object_top_widget_get(w));
  // save the widget
  evas_object_data_set(menu, "_genlist", w);

  eo_event_callback_call(w, ELM_FILE_BOOKMARKS_EVENT_HOOK_MENU_BOOKMARKS_START , menu);

  // if we are removable remove it
  if (b->removable)
    elm_menu_item_add(menu, NULL, "edit-delete" , "Del Bookmark", _ctx_del, b);

  // option to display or hide gtk
  o = elm_check_add(w);
  if (config)
    elm_check_state_set(o, config->display_gtk);
  elm_object_text_set(o, "Show GTK bookmarks");

  it = elm_menu_item_add(menu, NULL, NULL, NULL, _ctx_gtk, w);
  elm_object_item_content_set(it, o);

  eo_event_callback_call(w, ELM_FILE_BOOKMARKS_EVENT_HOOK_MENU_BOOKMARKS_END, menu);

  // move and show
  elm_menu_move(menu, x, y);
  evas_object_show(menu);
}

//============================================================================================
// Contextmenu for device functions
//============================================================================================
static void
_unmount_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_data EINA_UNUSED)
{
   emous_device_umount(data);
}
  static void
_mount_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_data EINA_UNUSED)
{
   emous_device_mount(data);
}

static void
_ctx_menu_device(Evas_Object *w, int x, int y, Device *b)
{
   Evas_Object *menu;
   Eina_Bool mounted;
   // create menu
   menu = elm_menu_add(elm_object_top_widget_get(w));
   // save the widget
   evas_object_data_set(menu, "_genlist", w);

   mounted = !!emous_device_mountpoints_get(b->d);

   //eo_do(w, eo_event_callback_call(ELM_FILE_DISPLAY_EVENT_HOOK_MENU_DEVICE_START, menu));

   elm_menu_item_separator_add(menu, NULL);

   if (mounted)
     elm_menu_item_add(menu, NULL, NULL, "Unmounted", _unmount_cb, b->d);
   else
     elm_menu_item_add(menu, NULL, NULL, "Mount", _mount_cb, b->d);

   elm_menu_item_separator_add(menu, NULL);

   //eo_do(w, eo_event_callback_call(ELM_FILE_DISPLAY_EVENT_HOOK_MENU_DEVICE_END, menu));

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
   // get Bookmark of this item
   b = elm_object_item_data_get(it);
   // get mous position
   evas_pointer_canvas_xy_get(evas_object_evas_get(obj), &x, &y);
   // open menu
   if (b->type == BOOKMARK_TYPE_DIRECTORY)
     _ctx_menu_directory(obj, x, y, &(b->pd.dir));
   else
     _ctx_menu_device(obj, x, y, &(b->pd.dev));
}

static void
_static_data_init()
{
   sd->dnd_item_class = elm_genlist_item_class_new();
   sd->dnd_item_class->item_style = "default";
   sd->dnd_item_class->func.text_get = _dnd_item_label_get;

   sd->place_item_class = elm_genlist_item_class_new();
   sd->place_item_class->item_style = "default";
   sd->place_item_class->func.text_get = _item_label_get;
   sd->place_item_class->func.content_get = _item_content_get;
   sd->place_item_class->func.del = _item_del;

   sd->group_item_class = elm_genlist_item_class_new();
   sd->group_item_class->item_style = "group_index";
   sd->group_item_class->func.text_get = _group_label_get;
   sd->group_item_class->func.del = _group_del;

   sd->device_item_class = elm_genlist_item_class_new();
   sd->device_item_class->item_style = "default";
   sd->device_item_class->func.content_get = _device_item_content_get;
   sd->device_item_class->func.text_get = _device_item_text_get;
   sd->device_item_class->func.del = _device_item_del;
}

static void
_static_data_ref()
{
    if (!sd)
      {
         sd = calloc(1, sizeof(Elm_File_Bookmarks_Sd));
         _static_data_init();
      }
    sd->ref ++;
}

static void
_static_data_unref()
{
   if (!sd) return;
   sd->ref --;
   if (sd->ref == 0)
     {
        elm_genlist_item_class_unref(sd->group_item_class);
        elm_genlist_item_class_unref(sd->place_item_class);
        elm_genlist_item_class_unref(sd->device_item_class);
        elm_genlist_item_class_unref(sd->dnd_item_class);
        free(sd);
        sd = NULL;
     }
}


EOLIAN static Eo_Base *
_elm_file_bookmarks_eo_base_constructor(Eo *obj, Elm_File_Bookmarks_Data *pd)
{
   Eo *eo;
   Emous_Manager *m;

   elm_ext_config_init();
   emous_init();

   _static_data_ref();
   pd->icons = eina_hash_stringshared_new(NULL);


   m = emous_manager_object_get(EMOUS_MANAGER_CLASS);
   eo_event_callback_add(m, EMOUS_MANAGER_EVENT_DEVICE_ADD, _device_add_cb, obj);
   eo_event_callback_add(m, EMOUS_MANAGER_EVENT_DEVICE_DEL, _device_del_cb, obj);


   eo = eo_constructor(eo_super(obj, ELM_FILE_BOOKMARKS_CLASS));

   elm_object_focus_allow_set(obj, EINA_FALSE);

   // setup drop zone
   elm_drop_target_add(obj, ELM_SEL_FORMAT_TARGETS,
                       _dnd_enter, NULL, _dnd_leave,
                       NULL, NULL, NULL,
                       _dnd_droped, NULL);

   elm_scroller_policy_set(obj, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_AUTO);
   elm_object_style_set(obj, "file_display");

   evas_object_smart_callback_add(obj, "clicked,right", _right_click, NULL);

   _setup_list(obj);

   return eo;
}

EOLIAN static void
_elm_file_bookmarks_eo_base_destructor(Eo *obj, Elm_File_Bookmarks_Data *pd EINA_UNUSED)
{
    elm_ext_config_shutdown();
    _static_data_unref();
    emous_shutdown();

    eo_destructor(eo_super(obj, ELM_FILE_BOOKMARKS_CLASS));
}

EOLIAN static void
_elm_file_bookmarks_cache_set(Eo *obj EINA_UNUSED, Elm_File_Bookmarks_Data *pd, Elm_File_MimeType_Cache *cache)
{
   if (pd->cache)
     eo_unref(pd->cache);

   pd->cache = cache;

   if (pd->cache)
     eo_ref(pd->cache);
}

EOLIAN static Elm_File_MimeType_Cache *
_elm_file_bookmarks_cache_get(Eo *obj EINA_UNUSED, Elm_File_Bookmarks_Data *pd)
{
   return pd->cache;
}

EOLIAN static Eo_Base *
_elm_file_bookmarks_eo_base_finalize(Eo *obj, Elm_File_Bookmarks_Data *pd)
{
   if (!pd->cache) return NULL;

   return eo_finalize(eo_super(obj, ELM_FILE_BOOKMARKS_CLASS));
}

#include "elm_file_bookmarks.eo.x"