#define INEEDWIDGET
#include "../elementary_ext_priv.h"

#define PRIV_DATA(o) Elm_File_Selector_Data *pd = eo_data_scope_get(o, ELM_FILE_SELECTOR_CLASS);
#define STEP_SIZE 10
#define ICON_SIZE_INF 0
#define ICON_SIZE_SUP 101

typedef struct
{
   struct {
     Evas_Object *selection;
     Evas_Point startpoint;
   }event;

   struct {
     Evas_Object *obj;
     const Eo_Class *klass;
   } view;

   struct {
     Eina_Strbuf *buffer;
   } search;

   Evas_Object *work_indicator;

   Efm_File *file;
   Elm_File_MimeType_Cache *cache;
   Eina_List *selection;

   Efm_Filter *filter;
} Elm_File_Selector_Data;

typedef struct {
   int x;
   int y;
   Eina_List *icons;
} Animpass;

static Eina_Hash *views = NULL;
static Eina_Bool _event_rect_mouse_down(void *data, const Eo_Event *event);
static Elm_Object_Item* _dnd_item_get_cb(Evas_Object *obj, Evas_Coord x, Evas_Coord y, int *xposret, int *yposret);
static Eina_Bool _dnd_data_get_cb(Evas_Object *obj, Elm_Object_Item *it, Elm_Drag_User_Info *info);
static void _ctx_menu_open(Eo* obj, int x, int y, Elm_File_Icon *icon, Efm_File *file);
static void _search_update(Eo *obj, Elm_File_Selector_Data *pd);

/*
 *======================================
 * flush filter settings to the filter and view
 *======================================
 */
static void
_filter_update_hidden(Eo *obj EINA_UNUSED, Elm_File_Selector_Data *pd)
{
   if (!config->hidden_files)
     efm_filter_attribute_add(pd->filter, EFM_ATTRIBUTE_FILENAME, "^[^\\.]");
   else
     efm_filter_attribute_del(pd->filter,EFM_ATTRIBUTE_FILENAME, "^[^\\.]");
}

static void
_filter_update_only_folder(Eo *obj EINA_UNUSED, Elm_File_Selector_Data *pd)
{
   if (config->only_folder)
     efm_filter_type_add(pd->filter, EFM_FILE_TYPE_DIRECTORY);
   else
     efm_filter_type_del(pd->filter, EFM_FILE_TYPE_DIRECTORY);
}

/*
 *======================================
 * View pool stuff
 *======================================
 */
static void
_views_standart_init()
{
   views = eina_hash_string_small_new(NULL);
   elm_file_selector_view_pool_add(ELM_FILE_SELECTOR_CLASS, ELM_FILE_DISPLAY_VIEW_GRID_CLASS);
   elm_file_selector_view_pool_add(ELM_FILE_SELECTOR_CLASS, ELM_FILE_DISPLAY_VIEW_LIST_CLASS);
#ifdef DEBUG
   elm_file_selector_view_pool_add(ELM_FILE_DISPLAY_VIEW_DEBUG_CLASS);
#endif
}

EOLIAN static void
_elm_file_selector_view_pool_add(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED, const Eo_Class *view)
{
   const char *name;

   if (!views)
     _views_standart_init();

   name = elm_file_view_name_get(view);

   eina_hash_direct_add(views, name, view);
}

EOLIAN static void
_elm_file_selector_view_pool_del(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED, const Eo_Class *view)
{
   const char *name;

   if (!view)
     return;

   name = elm_file_view_name_get(view);
   eina_hash_del(views, name, NULL);
}

/*
 *======================================
 * View stuff
 *======================================
 */
static Eina_Bool
_view_selected_cb(void *data, const Eo_Event *event)
{
   Efm_File *f = event->info;

   eo_event_callback_call(data, ELM_FILE_SELECTOR_EVENT_ITEM_SELECTED, f);
   return EINA_TRUE;
}

static Eina_Bool
_view_choosen_cb(void *data, const Eo_Event *event)
{
   Efm_File *f = event->info;

   eo_event_callback_call(data, ELM_FILE_SELECTOR_EVENT_ITEM_CHOOSEN, f);

   return EINA_TRUE;
}

static Eina_Bool
_view_select_changed_cb(void *data, const Eo_Event *event)
{
   Evas_Object *wid = data;
   PRIV_DATA(wid);

   if (!event->info)
     return EO_CALLBACK_CONTINUE;

   pd->selection = event->info;
   eo_event_callback_call(wid, ELM_FILE_SELECTOR_EVENT_ITEM_SELECTION_CHANGED, pd->selection);
   return EO_CALLBACK_CONTINUE;
}

static Eina_Bool
_work_done(void *data, const Eo_Event *event EINA_UNUSED)
{
   PRIV_DATA(data);
   eo_unref(pd->work_indicator);
   pd->work_indicator = NULL;

   return EO_CALLBACK_CONTINUE;
}

static Eina_Bool
_work_start(void *data, const Eo_Event *event)
{
   PRIV_DATA(data);

   pd->work_indicator = elm_progressbar_add(event->object);
   elm_object_style_set(pd->work_indicator, "wheel");
   elm_progressbar_pulse_set(pd->work_indicator, EINA_TRUE);
   elm_progressbar_pulse(pd->work_indicator, EINA_TRUE);
   elm_object_part_content_set(data, "waiting", pd->work_indicator);
   evas_object_show(pd->work_indicator);

   return EO_CALLBACK_CONTINUE;
}

EO_CALLBACKS_ARRAY_DEFINE(view_events,
  {ELM_FILE_VIEW_EVENT_ITEM_SELECT_SIMPLE, _view_selected_cb},
  {ELM_FILE_VIEW_EVENT_ITEM_SELECT_CHOOSEN, _view_choosen_cb},
  {ELM_FILE_VIEW_EVENT_ITEM_SELECT_CHANGED, _view_select_changed_cb},
  {EVAS_OBJECT_EVENT_MOUSE_DOWN, _event_rect_mouse_down},
  {ELM_FILE_VIEW_EVENT_WORKING_DONE, _work_done},
  {ELM_FILE_VIEW_EVENT_WORKING_START, _work_start}
);

EOLIAN static void
_elm_file_selector_view_set(Eo *obj, Elm_File_Selector_Data *pd, const Eo_Class *klass)
{
   pd->view.klass = klass;
   if (pd->view.obj)
     {
        eo_del(pd->view.obj);
        elm_drag_item_container_del(pd->view.obj);
     }
   pd->view.obj = eo_add(pd->view.klass, obj);
   elm_drag_item_container_add(pd->view.obj, 0.3, 0.3, _dnd_item_get_cb, _dnd_data_get_cb);
   elm_object_part_content_set(obj, "content", pd->view.obj);
   evas_object_show(pd->view.obj);
   eo_event_callback_array_add(pd->view.obj, view_events(), obj);
   elm_file_view_iconsize_set(pd->view.obj, config->icon_size);
   elm_file_view_filter_set(pd->view.obj, pd->filter);

   _filter_update_hidden(obj, pd);
   _filter_update_only_folder(obj, pd);
   elm_file_view_file_set(pd->view.obj, pd->file);
}

EOLIAN static const Eo_Class *
_elm_file_selector_view_get(Eo *obj EINA_UNUSED, Elm_File_Selector_Data *pd)
{
   return pd->view.klass;
}
/*
 *======================================
 * Object co/de - struction
 *======================================
 */
EOLIAN static Eo_Base *
_elm_file_selector_eo_base_constructor(Eo *obj, Elm_File_Selector_Data *pd)
{
   efm_init();
   elm_ext_config_init();

   if (!views)
     _views_standart_init();

   pd->filter = eo_add(EFM_FILTER_CLASS, NULL);
   _filter_update_hidden(obj, pd);
   _filter_update_only_folder(obj, pd);

   pd->cache = elm_file_mimetype_cache_generate(ELM_FILE_MIMETYPE_CACHE_CLASS, config->icon_size);

   return eo_constructor(eo_super(obj, ELM_FILE_SELECTOR_CLASS));
}

EOLIAN static void
_elm_file_selector_eo_base_destructor(Eo *obj, Elm_File_Selector_Data *pd)
{
   eo_del(pd->cache);
   elm_ext_config_shutdown();
   eo_destructor(eo_super(obj, ELM_FILE_SELECTOR_CLASS));
   efm_shutdown();
}

EOLIAN static void
_elm_file_selector_evas_object_smart_add(Eo *obj, Elm_File_Selector_Data *pd EINA_UNUSED)
{
   const Eo_Class *view;

   evas_obj_smart_add(eo_super(obj, ELM_FILE_SELECTOR_CLASS));

   if (!elm_layout_theme_set(obj, "file_selector", "base", "default"))
     {
        CRI("Failed to set theme file\n");
     }

   view = eina_hash_find(views, config->viewname);
   if (!view)
     {
        ERR("The saved view cannot be found, falling back to grid");
        view = ELM_FILE_DISPLAY_VIEW_GRID_CLASS;
     }

   elm_file_selector_view_set(obj, view);
}

EOLIAN static void
_elm_file_selector_file_set(Eo *obj, Elm_File_Selector_Data *pd, Efm_File *file)
{
   if (pd->file == file) return;

   if (pd->file) eo_unref(pd->file);
   pd->file = file;
   if (pd->file) eo_ref(pd->file);

   elm_file_view_file_set(pd->view.obj, pd->file);
   eo_event_callback_call(obj, ELM_FILE_SELECTOR_EVENT_PATH_CHANGED, pd->file);

   eina_strbuf_free(pd->search.buffer);
   pd->search.buffer = NULL;
   _search_update(obj, pd);
}

EOLIAN static Efm_File *
_elm_file_selector_file_get(Eo *obj EINA_UNUSED, Elm_File_Selector_Data *pd)
{
   return pd->file;
}

/*
 *======================================
 * Event Rect mouse events
 *======================================
 */
static Eina_Bool
_event_rect_mouse_move(void *data, const Eo_Event *event)
{
   PRIV_DATA(data)
   Evas_Event_Mouse_Move *ev = event->info;
   Eina_Rectangle view, selection;

   if (!pd->event.selection)
     {
        pd->event.selection = elm_layout_add(event->object);
        if (!elm_layout_theme_set(pd->event.selection, "file_display", "selection", "default"))
          ERR("Failed to set selection theme, selection is invisible ...");
        evas_object_show(pd->event.selection);
     }

   selection.x = pd->event.startpoint.x;
   selection.y = pd->event.startpoint.y;

   selection.w = ev->cur.canvas.x - pd->event.startpoint.x;
   selection.h = ev->cur.canvas.y - pd->event.startpoint.y;

   elm_file_view_size_get(pd->view.obj, &view);

   if (selection.w < 0)
     {
        selection.w *= -1;
        if (selection.x - selection.w > view.x)
          {
             selection.x -= selection.w;
          }
        else
          {
             selection.x = view.x;
             selection.w = pd->event.startpoint.x - view.x;
          }
     }

   if (selection.h < 0)
     {
        selection.h *= -1;
        if (selection.y - selection.h > view.y)
          {
             selection.y -= selection.h;
          }
        else
          {
             selection.y = view.y;
             selection.h = pd->event.startpoint.y - view.y;
          }
     }

   if (selection.x+selection.w > view.x+view.w)
     selection.w = (view.x+view.w)-selection.x;

   if (selection.y+selection.h > view.y+view.h)
     selection.h = (view.y+view.h)-selection.y;

   evas_object_geometry_set(pd->event.selection, selection.x, selection.y, selection.w, selection.h);

   return EO_CALLBACK_CONTINUE;
}

static Eina_Bool
_event_rect_mouse_up(void *data, const Eo_Event *event)
{
   PRIV_DATA(data);
   Eina_List *sel;
   Eina_Rectangle selection;

   evas_object_geometry_get(pd->event.selection, &selection.x, &selection.y, &selection.w, &selection.h);
   evas_object_del(pd->event.selection);
   pd->event.selection = NULL;

   /* call for selection*/
   sel = elm_file_view_search_items(pd->view.obj, &selection);
   elm_file_view_selection_set(pd->view.obj, sel);

   eo_event_callback_del(event->object, EVAS_OBJECT_EVENT_MOUSE_MOVE, _event_rect_mouse_move, data);
   eo_event_callback_del(event->object, EVAS_OBJECT_EVENT_MOUSE_UP, _event_rect_mouse_up, data);
   return EO_CALLBACK_CONTINUE;
}

static Eina_Bool
_event_rect_mouse_down(void *data, const Eo_Event *event)
{
   PRIV_DATA(data)
   Eina_List *icons;
   Evas_Event_Mouse_Down *ev = event->info;
   Eina_Rectangle view, search;

   EINA_RECTANGLE_SET(&search, ev->output.x, ev->output.y, 1, 1);

   icons = elm_file_view_search_items(pd->view.obj, &search);
   elm_file_view_size_get(pd->view.obj, &view);

   if (!eina_rectangle_coords_inside(&view, ev->output.x, ev->output.y))
     return EO_CALLBACK_CONTINUE;

   if (ev->button == 1 && !icons)
     {
        pd->event.startpoint.x = ev->output.x;
        pd->event.startpoint.y = ev->output.y;
        eo_event_callback_add(event->object, EVAS_OBJECT_EVENT_MOUSE_MOVE, _event_rect_mouse_move, data);
        eo_event_callback_add(event->object, EVAS_OBJECT_EVENT_MOUSE_UP, _event_rect_mouse_up, data);
     }
   else if (ev->button == 3)
     {
        Elm_File_Icon *file_icon = eina_list_data_get(icons);
        Efm_File *file = NULL;
        file = elm_obj_file_icon_file_get(file_icon);
        _ctx_menu_open(data, ev->output.x, ev->output.y, file_icon, file);
     }

   return EO_CALLBACK_CONTINUE;
}

/*
 *======================================
 * Drag and Drop stuff
 *======================================
 */
static Elm_Object_Item*
_dnd_item_get_cb(Evas_Object *obj, Evas_Coord x, Evas_Coord y, int *xposret, int *yposret)
{
   Eina_List *icons;
   Elm_File_Icon *icon;
   int xx,yy;
   // XXX this should not be needed
   Eina_Bool rename_mode;
   Eina_Rectangle search;

   EINA_RECTANGLE_SET(&search, x, y, 1, 1);
   icons = elm_file_view_search_items(obj, &search);

   if (!icons)
     return NULL;

   icon = eina_list_data_get(icons);
   rename_mode = elm_obj_file_icon_rename_get(icon);

   if (rename_mode)
     return NULL;

   evas_object_geometry_get(icon, &xx, &yy, NULL, NULL);

   if (xposret)
     *xposret = xx;
   if (yposret)
     *yposret = yy;

   return icon;
}

static Eina_List*
_dnd_anim_ics_gen(Evas_Object *obj, Evas_Object *view, Eina_List **anim_icons, Eina_List **mimetypes)
{
   PRIV_DATA(obj)
   Eina_List *node, *list;
   Eina_List *result = NULL;
   Elm_File_Icon *icon;


   list = elm_file_view_selection_get(view);
   *anim_icons = NULL;
   *mimetypes = NULL;
   EINA_LIST_FOREACH(list, node, icon)
     {
        Eina_Rectangle place;
        Efm_File *f;
        const char *file;
        const char *mimetype;

        f = elm_obj_file_icon_file_get(icon);
        mimetype = efm_file_mimetype_get(f);
        file = elm_file_mimetype_cache_mimetype_get(pd->cache, mimetype);

        icon = elm_icon_add(view);
        evas_object_geometry_get(icon, &place.x, &place.y, &place.w, &place.h);
        evas_object_geometry_set(icon, place.x, place.y, place.w, place.h);
        elm_image_file_set(icon, file, NULL);
        evas_object_show(icon);

        *anim_icons = eina_list_append(*anim_icons, icon);
        *mimetypes = eina_list_append(*mimetypes, file);
     }

   return result;
}

static const char*
_dnd_items_gen(Evas_Object *view)
{
   Eina_Strbuf *buf;
   Eina_List *node, *list;
   Elm_File_Icon *icon;

   buf = eina_strbuf_new();
   list = elm_file_view_selection_get(view);

   EINA_LIST_FOREACH(list, node, icon)
     {
        Efm_File *f;
        const char *path;

        f = elm_obj_file_icon_file_get(icon);
        path = efm_file_path_get(f);
        eina_strbuf_append(buf, FILESEP);
        eina_strbuf_append(buf, path);
        eina_strbuf_append(buf, "\n");
     }

  const char *drag_data;
  drag_data = eina_strbuf_string_steal(buf);
  eina_strbuf_free(buf);

  return drag_data;
}

static Evas_Object*
_dnd_create_icon(void *data, Evas_Object *win, Evas_Coord *xoff, Evas_Coord *yoff)
{
   Evas_Object *res;
   Eina_List *node;
   Animpass *pass = data;
   const char *file;
   int i = 0;

   res = elm_layout_add(win);
   if (!elm_layout_theme_set(res, "file_display", "drag_icon", "default"))
     ERR("Failed to set selection theme, selection is invisible ...");

   evas_object_move(res, 0, 0);
   evas_object_resize(res, 60, 60);

   EINA_LIST_FOREACH(pass->icons, node, file)
     {
        Evas_Object *icon;
        char buf[PATH_MAX];
        i++;

        icon = elm_icon_add(win);
        elm_icon_order_lookup_set(icon, ELM_ICON_LOOKUP_FDO_THEME);
        elm_image_file_set(icon, file, NULL);
        elm_icon_standard_set(icon, file);
        evas_object_show(icon);

        snprintf(buf, sizeof(buf), "img%d", i);
        elm_object_part_content_set(res, buf, icon);
        if (i == 3)
          break;
     }
   evas_object_show(res);
   evas_object_layer_set(res, EVAS_LAYER_MAX);

   if (xoff) *xoff = pass->x - 30;
   if (yoff) *yoff = pass->y - 30;

   return res;
}

static void
_dnd_drag_start(void *data, Evas_Object *obj)
{
   Animpass *pass = data;

   evas_pointer_canvas_xy_get(
      evas_object_evas_get(obj), &pass->x, &pass->y);
}

static Eina_Bool
_dnd_data_get_cb(Evas_Object *obj, Elm_Object_Item *it EINA_UNUSED, Elm_Drag_User_Info *info)
{
    Animpass *pass = calloc(1, sizeof(Animpass));
    Evas_Object *parent;
    Eina_List *mimetypelist;
    Eina_List *anim_icons;

    parent = eo_parent_get(obj);

    _dnd_anim_ics_gen(parent, obj, &anim_icons, &mimetypelist);
    pass->icons = mimetypelist;

    info->format = ELM_SEL_FORMAT_TARGETS;
    info->icons = anim_icons;
    info->data = _dnd_items_gen(obj);
    info->action = ELM_XDND_ACTION_PRIVATE;
    info->createicon = _dnd_create_icon;
    info->createdata = pass;
    info->dragstart = _dnd_drag_start;
    info->startcbdata = pass;

    return EINA_TRUE;
}
/*
 *======================================
 * icon rename things
 *======================================
 */
static Eina_Bool
_icon_rename_cb(void *data EINA_UNUSED, const Eo_Event *event)
{
   const char *name = event->info;
   const char *filename;
   Efm_File *file;

   file = elm_obj_file_icon_file_get(event->object);

   if (!file) return EINA_FALSE;

   filename = efm_file_filename_get(file);

   if (!!strcmp(name, filename))
     {
        const char *path;
        const char *dir;
        char buf[PATH_MAX];

        path = efm_file_path_get(file);
        dir = ecore_file_dir_get(path);
        snprintf(buf, sizeof(buf), "%s/%s", dir, name);
        if (!ecore_file_mv(path, buf))
          ERR("Rename failed!");
     }
   return EINA_TRUE;
}

/*
 *======================================
 * search things
 *======================================
 */
static void
_search_update(Eo *obj, Elm_File_Selector_Data *pd)
{
   const char *search = NULL;
   Eina_Bool found;

   if (pd->search.buffer)
     search = eina_strbuf_string_get(pd->search.buffer);

   if (search)
     elm_layout_signal_emit(obj, "search,enable", "elm");
   else
     elm_layout_signal_emit(obj, "search,disable", "elm");

   elm_layout_text_set(obj, "search", search);

   found = elm_file_view_search(pd->view.obj, search);

   if (!found)
     elm_layout_signal_emit(obj, "search,failed", "elm");
   else
     elm_layout_signal_emit(obj, "search,found", "elm");
}

/*
 *======================================
 * Key down Event stuff
 *======================================
 */
EOLIAN static Eina_Bool
_elm_file_selector_elm_widget_event(Eo *obj, Elm_File_Selector_Data *pd, Evas_Object *source EINA_UNUSED, Evas_Callback_Type type, void *event_info)
{
   Evas_Event_Key_Down *ev;

   if (type != EVAS_CALLBACK_KEY_DOWN)
     return EINA_FALSE;
   ev = event_info;
   // XXX: add some code for more "general" shortcuts
   if (evas_key_modifier_is_set(ev->modifiers, "Control") && (!strcmp(ev->key, "H") || !strcmp(ev->key, "h")))
     {
        Eina_Bool b;
        b = elm_file_selector_show_hidden_file_get(obj);
        elm_file_selector_show_hidden_file_set(obj, !b);
     }
   if (ev->string && strlen(ev->string) == 1 &&
         (isalnum(ev->string[0]) || ev->string[0] == '.')
      )
     {
        if (!pd->search.buffer)
          pd->search.buffer = eina_strbuf_new();
        eina_strbuf_append_char(pd->search.buffer, ev->string[0]);
        _search_update(obj, pd);
     }
   else if (!strcmp(ev->key, "BackSpace"))
     {
        int oldlength;
        char *oldstr;

        if (!pd->search.buffer)
          return EINA_FALSE;

        if (eina_strbuf_length_get(pd->search.buffer) == 1)
          {
             eina_strbuf_free(pd->search.buffer);
             pd->search.buffer = NULL;
             _search_update(obj, pd);
             return EINA_TRUE;
          }

         oldlength = eina_strbuf_length_get(pd->search.buffer);
         oldstr = eina_strbuf_string_steal(pd->search.buffer);

         // reset the string
         eina_strbuf_reset(pd->search.buffer);
         // cut off the last bit
         oldstr[oldlength - 1] = '\0';

         eina_strbuf_append(pd->search.buffer, oldstr);
         free(oldstr);
         _search_update(obj, pd);
     }
   else if (!strcmp(ev->key, "F2"))
      {
        // start rename mode
        Eina_List *node, *selection;
        Elm_File_Icon *icon;

        selection = elm_file_view_selection_get(pd->view.obj);
        EINA_LIST_FOREACH(selection, node, icon)
          {
             eo_event_callback_add(icon, ELM_FILE_ICON_EVENT_RENAME_DONE, _icon_rename_cb, NULL);
             elm_obj_file_icon_rename_set(icon, EINA_TRUE, EINA_FALSE);
          }
        return EO_CALLBACK_STOP;
      }
   else if (!strcmp(ev->key, "Escape"))
      {
         eina_strbuf_free(pd->search.buffer);
         pd->search.buffer = NULL;
         _search_update(obj, pd);
      }
   return EINA_FALSE;
}
/*
 *======================================
 * Context menu stuff
 *======================================
 */

static void
_ctx_view_sel(void *data, Evas_Object *obj, void *event EINA_UNUSED)
{
   Evas_Object *w = evas_object_data_get(obj, "__w");

   config->viewname = elm_file_view_name_get(data);

   elm_ext_config_save();

   elm_file_selector_view_set(w, data);
}

static void
_ctx_icon_size(void *data, Evas_Object *obj, void *event EINA_UNUSED)
{
   Evas_Object *w = evas_object_data_get(obj, "__w");
   int size = (int)(uintptr_t)data;

   elm_file_selector_show_icon_size_set(w, size);
}

static void
_ctx_sort_type(void *data, Evas_Object *obj, void *event EINA_UNUSED)
{
   Evas_Object *w = evas_object_data_get(obj, "__w");
   int sort_type = (int)(uintptr_t)data;

   elm_file_selector_sort_type_set(w, sort_type);
}

static void
_ctx_folder_placement(void *data, Evas_Object *obj, void *event EINA_UNUSED)
{
   Evas_Object *w = evas_object_data_get(obj, "__w");
   Elm_File_Selector_Folder_Placement p = ((Elm_File_Selector_Folder_Placement)data);

   if (p == (Elm_File_Selector_Folder_Placement)config->sort.folder_placement)
     p = ELM_FILE_SELECTOR_FOLDER_PLACEMENT_TRIVIAL;

   elm_file_selector_folder_placement_set(w, p);
}

static void
_ctx_reverse(void *data EINA_UNUSED, Evas_Object *obj, void *event EINA_UNUSED)
{
   Evas_Object *w = evas_object_data_get(obj, "__w");
   Eina_Bool p;

   if (config->sort.reverse != 0)
     p = EINA_FALSE;
   else
     p = EINA_TRUE;
   elm_file_selector_reverse_sort_set(w, p);
}

static void
_ctx_casesensetive(void *data EINA_UNUSED, Evas_Object *obj, void *event EINA_UNUSED)
{
   Evas_Object *w = evas_object_data_get(obj, "__w");
   Eina_Bool p;

   if (config->sort.casesensetive != 0)
     p = EINA_FALSE;
   else
     p = EINA_TRUE;
   elm_file_selector_case_sensetive_sort_set(w, p);
}

static void
_ctx_hidden_files(void *data EINA_UNUSED, Evas_Object *obj, void *event EINA_UNUSED)
{
   Evas_Object *w = evas_object_data_get(obj, "__w");
   Eina_Bool b;

   b = elm_file_selector_show_hidden_file_get(w);
   elm_file_selector_show_hidden_file_set(w, !b);
}

static void
_ctx_only_folder(void *data EINA_UNUSED, Evas_Object *obj, void *event EINA_UNUSED)
{
   Evas_Object *w = evas_object_data_get(obj, "__w");
   Eina_Bool b;

   b = elm_file_selector_only_folder_get(w);
   elm_file_selector_only_folder_set(w, !b);
}

static void
_ctx_rename(void *data, Evas_Object *obj EINA_UNUSED, void *event EINA_UNUSED)
{
   eo_event_callback_add(data, ELM_FILE_ICON_EVENT_RENAME_DONE, _icon_rename_cb, NULL);
   elm_obj_file_icon_rename_set(data, EINA_TRUE, EINA_FALSE);
}

static void
_ctx_new_folder(void *data, Evas_Object *obj EINA_UNUSED, void *event EINA_UNUSED)
{
  Efm_File *file;
  const char *dir;
  char path[PATH_MAX];
  int c;

  file = elm_file_selector_file_get(data);
  dir = efm_file_path_get(file);

  snprintf(path, sizeof(path), "%s/new_directory", dir);
  for(c = 0; ecore_file_exists(path); c++)
    snprintf(path, sizeof(path), "%s/new_directory_%d", dir, c);

  ecore_file_mkdir(path);
}

static void
_ctx_image_preview(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event EINA_UNUSED)
{
   Evas_Object *w = evas_object_data_get(obj, "__w");
   Eina_Bool b;

   b = elm_file_selector_image_preview_get(w);
   elm_file_selector_image_preview_set(w, !b);
}

static void
_ctx_menu_open(Eo* obj, int x, int y, Elm_File_Icon *icon, Efm_File *file)
{
   Evas_Object *menu;
   Elm_Object_Item *it, *it2;
   Elm_File_Selector_Menu_Hook ev;

   menu = elm_menu_add(obj);
   evas_object_data_set(menu, "__w", obj);

   ev.menu = menu;
   ev.file = file;

   eo_event_callback_call(obj, ELM_FILE_SELECTOR_EVENT_HOOK_MENU_SELECTOR_START,&ev);
   elm_menu_item_separator_add(menu, NULL);
   /*
    * Rename
    */
   if (file) {
      it = elm_menu_item_add(menu, NULL, NULL, "Rename", _ctx_rename, icon);
   }
   /*
    * New folder
    */
   it = elm_menu_item_add(menu, NULL, NULL, "New Folder", _ctx_new_folder, obj);

   elm_menu_item_separator_add(menu, NULL);
   /*
    * Views
    */
   it = elm_menu_item_add(menu, NULL, NULL, "Views", NULL, NULL);
   const char *name;

   if (views)
     {
        Eina_Iterator *iter;
        const Eo_Class *klass;

        iter = eina_hash_iterator_data_new(views);

        EINA_ITERATOR_FOREACH(iter, klass)
          {
             name = elm_file_view_name_get(klass);
             it2 = elm_menu_item_add(menu, it, NULL, name, _ctx_view_sel, klass);
          }
     }

   /*
    * Icon size
    */
   it = elm_menu_item_add(menu, NULL, NULL, "Icon Size", NULL, NULL);
   {
      Evas_Object *rad;
      int c;
      char buf[PATH_MAX];
      // make the menu for the icons

      for (c = ICON_SIZE_INF; c < ICON_SIZE_SUP; c += STEP_SIZE)
        {
           snprintf(buf, sizeof(buf), "%d", c);

           rad = elm_check_add(menu);
           elm_object_text_set(rad, buf);

           if (config->icon_size == c)
             elm_check_state_set(rad, EINA_TRUE);

           it2 = elm_menu_item_add(menu, it, NULL, NULL, _ctx_icon_size, (void *)(uintptr_t)c);
           elm_object_item_content_set(it2, rad);
        }
   }

   /*
    * Sort Settings
    */
   it = elm_menu_item_add(menu, NULL, NULL, "Sort", NULL, NULL);
   {
      Evas_Object *gr, *rad, *ck;

      // SORT_TYPE_SIZE
      it2 = elm_menu_item_add(menu, it, NULL, NULL, _ctx_sort_type, (void *)(uintptr_t)ELM_FILE_SELECTOR_SORT_TYPE_SIZE);
      gr = rad = elm_radio_add(menu);
      elm_object_text_set(rad, "Size");
      elm_radio_state_value_set(rad, 0);
      elm_object_item_content_set(it2, rad);

      // SORT_TYPE_DATE
      it2 = elm_menu_item_add(menu, it, NULL, NULL, _ctx_sort_type, (void *)(uintptr_t)ELM_FILE_SELECTOR_SORT_TYPE_DATE);
      rad = elm_radio_add(menu);
      elm_radio_group_add(rad, gr);
      elm_object_text_set(rad, "Date");
      elm_radio_state_value_set(rad, 1);
      elm_object_item_content_set(it2, rad);

      // SORT_TYPE_NAME
      it2 = elm_menu_item_add(menu, it, NULL, NULL, _ctx_sort_type, (void *)(uintptr_t)ELM_FILE_SELECTOR_SORT_TYPE_NAME);
      rad = elm_radio_add(menu);
      elm_radio_group_add(rad, gr);
      elm_object_text_set(rad, "Name");
      elm_radio_state_value_set(rad, 2);
      elm_object_item_content_set(it2, rad);

      // SORT_TYPE_EXTENSION
      it2 = elm_menu_item_add(menu, it, NULL, NULL, _ctx_sort_type, (void *)(uintptr_t)ELM_FILE_SELECTOR_SORT_TYPE_EXTENSION);
      rad = elm_radio_add(menu);
      elm_radio_group_add(rad, gr);
      elm_object_text_set(rad, "Extension");
      elm_radio_state_value_set(rad, 3);
      elm_object_item_content_set(it2, rad);
      elm_radio_value_set(gr, config->sort.type);

      elm_menu_item_separator_add(menu, it);

      // FOLDER_FIRST
      it2 = elm_menu_item_add(menu, it, NULL, NULL, _ctx_folder_placement, (void *)(uintptr_t)ELM_FILE_SELECTOR_FOLDER_PLACEMENT_FIRST);
      ck = elm_check_add(menu);
      if (config->sort.folder_placement == ELM_FILE_SELECTOR_FOLDER_PLACEMENT_FIRST)
        elm_check_state_set(ck, EINA_TRUE);
      elm_object_text_set(ck, "Folder first");
      elm_object_item_content_set(it2, ck);

      it2 = elm_menu_item_add(menu, it, NULL, NULL, _ctx_folder_placement, (void *)(uintptr_t)ELM_FILE_SELECTOR_FOLDER_PLACEMENT_LAST);
      ck = elm_check_add(menu);
      if (config->sort.folder_placement == ELM_FILE_SELECTOR_FOLDER_PLACEMENT_LAST)
        elm_check_state_set(ck, EINA_TRUE);
      elm_object_text_set(ck, "Folder last");
      elm_object_item_content_set(it2, ck);

      elm_menu_item_separator_add(menu, it);

      it2 = elm_menu_item_add(menu, it, NULL, NULL, _ctx_reverse, NULL);
      ck = elm_check_add(menu);
      if (config->sort.reverse != 0)
        elm_check_state_set(ck, EINA_TRUE);
      elm_object_text_set(ck, "Reverse");
      elm_object_item_content_set(it2, ck);

      it2 = elm_menu_item_add(menu, it, NULL, NULL, _ctx_casesensetive, NULL);
      ck = elm_check_add(menu);
      if (config->sort.casesensetive != 0)
        elm_check_state_set(ck, EINA_TRUE);
      elm_object_text_set(ck, "Case Sensetive");
      elm_object_item_content_set(it2, ck);
   }
   elm_menu_item_separator_add(menu, NULL);
   /*
     Icon preview
   */
   it = elm_menu_item_add(menu, NULL, NULL, "", _ctx_image_preview, NULL);
   {
      Evas_Object *ck;
      ck = elm_check_add(menu);
      if (config->image_preview != 0)
        elm_check_state_set(ck, EINA_TRUE);
      else
        elm_check_state_set(ck, EINA_FALSE);
      elm_object_text_set(ck, "Icon preview");
      elm_object_item_content_set(it, ck);
   }
   /*
     Hidden files
    */
   it = elm_menu_item_add(menu, NULL, NULL, "", _ctx_hidden_files, NULL);
   {
      Evas_Object *ck;
      ck = elm_check_add(menu);
      if (config->hidden_files != 0)
        elm_check_state_set(ck, EINA_TRUE);
      else
        elm_check_state_set(ck, EINA_FALSE);
      elm_object_text_set(ck, "Hidden files");
      elm_object_item_content_set(it, ck);
   }

   /*
     Only folder
    */
   it = elm_menu_item_add(menu, NULL, NULL, "", _ctx_only_folder, NULL);
   {
      Evas_Object *ck;
      ck = elm_check_add(menu);
      if (config->only_folder != 0)
        elm_check_state_set(ck, EINA_TRUE);
      else
        elm_check_state_set(ck, EINA_FALSE);
      elm_object_text_set(ck, "Only folder");
      elm_object_item_content_set(it, ck);
   }

   elm_menu_item_separator_add(menu, NULL);
   eo_event_callback_call(obj, &_ELM_FILE_SELECTOR_EVENT_HOOK_MENU_SELECTOR_END, &ev);
   elm_menu_move(menu, x, y);
   evas_object_show(menu);
}

/*
 *======================================
 * Settings stuff
 *======================================
 */

EOLIAN static void
_elm_file_selector_show_icon_size_set(Eo *obj EINA_UNUSED, Elm_File_Selector_Data *pd, int size)
{
   config->icon_size = size;
   elm_ext_config_save();
   eo_del(pd->cache);

   pd->cache = elm_file_mimetype_cache_generate(ELM_FILE_MIMETYPE_CACHE_CLASS, size);
   elm_file_view_iconsize_set(pd->view.obj, config->icon_size);
}

EOLIAN static int
_elm_file_selector_show_icon_size_get(Eo *obj EINA_UNUSED, Elm_File_Selector_Data *pd EINA_UNUSED)
{
   return config->icon_size;
}


EOLIAN static void
_elm_file_selector_show_hidden_file_set(Eo *obj EINA_UNUSED, Elm_File_Selector_Data *pd, Eina_Bool hidden)
{
   config->hidden_files = hidden;
   elm_ext_config_save();
   _filter_update_hidden(obj, pd);
}

EOLIAN static Eina_Bool
_elm_file_selector_show_hidden_file_get(Eo *obj EINA_UNUSED, Elm_File_Selector_Data *pd EINA_UNUSED)
{
   return config->hidden_files;
}

EOLIAN static void
_elm_file_selector_only_folder_set(Eo *obj EINA_UNUSED, Elm_File_Selector_Data *pd, Eina_Bool hidden)
{
   config->only_folder = hidden;
   elm_ext_config_save();
   _filter_update_only_folder(obj, pd);
}

EOLIAN static Eina_Bool
_elm_file_selector_only_folder_get(Eo *obj EINA_UNUSED, Elm_File_Selector_Data *pd EINA_UNUSED)
{
   return config->only_folder;
}

EOLIAN static void
_elm_file_selector_sort_type_set(Eo *obj EINA_UNUSED, Elm_File_Selector_Data *pd, Elm_File_Selector_Sort_Type t)
{
   config->sort.type = t;
   elm_ext_config_save();
   elm_file_view_file_set(pd->view.obj, pd->file);
}

EOLIAN static Elm_File_Selector_Sort_Type
_elm_file_selector_sort_type_get(Eo *obj EINA_UNUSED, Elm_File_Selector_Data *pd EINA_UNUSED)
{
   return config->sort.type;
}

EOLIAN static void
_elm_file_selector_folder_placement_set(Eo *obj EINA_UNUSED, Elm_File_Selector_Data *pd, Elm_File_Selector_Folder_Placement t)
{
   config->sort.folder_placement = t;
   elm_ext_config_save();
   elm_file_view_file_set(pd->view.obj, pd->file);
}

EOLIAN static Elm_File_Selector_Folder_Placement
_elm_file_selector_folder_placement_get(Eo *obj EINA_UNUSED, Elm_File_Selector_Data *pd EINA_UNUSED)
{
   return config->sort.folder_placement;
}

EOLIAN static void
_elm_file_selector_reverse_sort_set(Eo *obj EINA_UNUSED, Elm_File_Selector_Data *pd, Eina_Bool b)
{
   config->sort.reverse = b;
   elm_ext_config_save();
   elm_file_view_file_set(pd->view.obj, pd->file);
}

EOLIAN static Eina_Bool
_elm_file_selector_reverse_sort_get(Eo *obj EINA_UNUSED, Elm_File_Selector_Data *pd EINA_UNUSED)
{
   return config->sort.reverse;
}

EOLIAN static void
_elm_file_selector_case_sensetive_sort_set(Eo *obj EINA_UNUSED, Elm_File_Selector_Data *pd, Eina_Bool b)
{
   config->sort.casesensetive = b;
   elm_ext_config_save();
   elm_file_view_file_set(pd->view.obj, pd->file);
}

EOLIAN static Eina_Bool
_elm_file_selector_case_sensetive_sort_get(Eo *obj EINA_UNUSED, Elm_File_Selector_Data *pd  EINA_UNUSED)
{
   return config->sort.casesensetive;
}

EOLIAN static void
_elm_file_selector_image_preview_set(Eo *obj EINA_UNUSED, Elm_File_Selector_Data *pd EINA_UNUSED, Eina_Bool b)
{
   config->image_preview = b;
   elm_ext_config_save();
   elm_file_view_file_set(pd->view.obj, pd->file);
}

EOLIAN static Eina_Bool
_elm_file_selector_image_preview_get(Eo *obj EINA_UNUSED, Elm_File_Selector_Data *pd EINA_UNUSED)
{
   return config->image_preview;
}

EOLIAN static Eina_List *
_elm_file_selector_selection_get(Eo *obj EINA_UNUSED, Elm_File_Selector_Data *pd)
{
   return pd->selection;
}

EOLIAN static void
_elm_file_selector_search(Eo *obj EINA_UNUSED, Elm_File_Selector_Data *pd, const char *value)
{
   if (!pd->search.buffer)
     pd->search.buffer = eina_strbuf_new();
   //update buffer
   eina_strbuf_reset(pd->search.buffer);
   eina_strbuf_append(pd->search.buffer, value);
   //call for display and view update
   _search_update(obj, pd);
}

static Eina_Bool
_drop_cb(void *data, const Eo_Event *event)
{
   eo_ref(event->object);
   eo_event_callback_call(data, ELM_FILE_SELECTOR_EVENT_DND_ITEM_DROPED, NULL);
   eo_unref(event->object);
   return EINA_FALSE;
}

static Eina_Bool
_hover_cb(void *data, const Eo_Event *event)
{
   eo_ref(event->object);
   eo_event_callback_call(data, ELM_FILE_SELECTOR_EVENT_DND_ITEM_HOVER, event->object);
   eo_unref(event->object);
   return EINA_FALSE;
}

EOLIAN static Elm_File_Icon *
_elm_file_selector_icon_generate(Eo *obj, Elm_File_Selector_Data *pd EINA_UNUSED, Efm_File *file)
{
   Evas_Object *ic;

#if 1
   ic = eo_add(ELM_FILE_ICON_CLASS, obj, elm_obj_file_icon_install(__eo_self, pd->cache, file, config->image_preview));
   eo_event_callback_add(ic, ELM_FILE_ICON_EVENT_ITEM_DROP, _drop_cb, obj);
   eo_event_callback_add(ic, ELM_FILE_ICON_EVENT_ITEM_HOVER, _hover_cb, obj);
#else
   const char *name;
   name = efm_file_path_get(file);
   ic = elm_label_add(par);
   elm_object_text_set(ic, name);
#endif
   evas_object_show(ic);

   return ic;
}

EOLIAN static Elm_File_MimeType_Cache *
_elm_file_selector_cache_get(Eo *obj EINA_UNUSED, Elm_File_Selector_Data *pd)
{
   return pd->cache;
}

#include "elm_file_selector.eo.x"
