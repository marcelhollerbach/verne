#define INEEDWIDGET
#include "../elementary_ext_priv.h"
#include <regex.h>

#define PRIV_DATA(o) Elm_File_Selector_Data *pd = efl_data_scope_get(o, ELM_FILE_SELECTOR_CLASS);
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
     const Efl_Class *klass;
   } view;

   struct {
     Eina_Strbuf *buffer;
     char *old_regex;
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
   Elm_File_MimeType_Cache *cache;
} Animpass;

static Eina_Hash *views = NULL;
static void _event_rect_mouse_down(void *data, const Efl_Event *event);
static Elm_Object_Item* _dnd_item_get_cb(Evas_Object *obj, Evas_Coord x, Evas_Coord y, int *xposret, int *yposret);
static Eina_Bool _dnd_data_get_cb(Evas_Object *obj, Elm_Object_Item *it, Elm_Drag_User_Info *info);
static void _ctx_menu_open(Eo* obj, int x, int y, Efm_File *file);
static void _search_update(Eo *obj, Elm_File_Selector_Data *pd);

static Eina_Bool _dnd_view_drop_cb(void *data, Evas_Object *obj EINA_UNUSED, Elm_Selection_Data *ev);


/*
 *======================================
 * flush filter settings to the filter and view
 *======================================
 */
static void
_filter_update_hidden(Eo *obj EINA_UNUSED, Elm_File_Selector_Data *pd)
{
   if (!config->hidden_files)
     efm_filter_attribute_add(pd->filter, EFM_ATTRIBUTE_FILENAME, "^[^\\.]", 0);
   else
     efm_filter_attribute_del(pd->filter,EFM_ATTRIBUTE_FILENAME, "^[^\\.]", 0);
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
_elm_file_selector_view_pool_add(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED, const Efl_Class *view)
{
   const char *name;

   if (!views)
     _views_standart_init();

   name = elm_file_view_name_get(view);

   eina_hash_direct_add(views, name, view);
}

EOLIAN static void
_elm_file_selector_view_pool_del(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED, const Efl_Class *view)
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
static void
_view_selected_cb(void *data, const Efl_Event *event)
{
   Efm_File *f = event->info;

   efl_event_callback_call(data, ELM_FILE_SELECTOR_EVENT_ITEM_SELECTED, f);
}

static void
_view_choosen_cb(void *data, const Efl_Event *event)
{
   Efm_File *f = event->info;

   efl_event_callback_call(data, ELM_FILE_SELECTOR_EVENT_ITEM_CHOOSEN, f);
}

static void
_view_select_changed_cb(void *data, const Efl_Event *event)
{
   Evas_Object *wid = data;
   PRIV_DATA(wid);

   if (!event->info)
     return;

   pd->selection = event->info;
   efl_event_callback_call(wid, ELM_FILE_SELECTOR_EVENT_ITEM_SELECTION_CHANGED, pd->selection);
}

static void
_work_done(void *data, const Efl_Event *event EINA_UNUSED)
{
   PRIV_DATA(data);
   efl_del(pd->work_indicator);
   pd->work_indicator = NULL;
}

static void
_work_start(void *data, const Efl_Event *event)
{
   PRIV_DATA(data);

   pd->work_indicator = elm_progressbar_add(event->object);
   elm_object_style_set(pd->work_indicator, "wheel");
   elm_progressbar_pulse_set(pd->work_indicator, EINA_TRUE);
   elm_progressbar_pulse(pd->work_indicator, EINA_TRUE);
   elm_object_part_content_set(data, "waiting", pd->work_indicator);
   evas_object_show(pd->work_indicator);
}

static void
_event_rect_wheel(void *data, const Efl_Event *ev)
{
   if (!evas_key_modifier_is_set(evas_canvas_key_modifier_get(evas_object_evas_get(data)), "Control"))
     return;

   int icon_size = elm_file_selector_show_icon_size_get(data);
   elm_file_selector_show_icon_size_set(data, icon_size - 5 * efl_input_pointer_wheel_delta_get(ev->info));
}

EFL_CALLBACKS_ARRAY_DEFINE(view_events,
  {ELM_FILE_VIEW_EVENT_ITEM_SELECT_SIMPLE, _view_selected_cb},
  {ELM_FILE_VIEW_EVENT_ITEM_SELECT_CHOOSEN, _view_choosen_cb},
  {ELM_FILE_VIEW_EVENT_ITEM_SELECT_CHANGED, _view_select_changed_cb},
  {EFL_EVENT_POINTER_DOWN, _event_rect_mouse_down},
  {EFL_EVENT_POINTER_WHEEL, _event_rect_wheel},
  {ELM_FILE_VIEW_EVENT_WORKING_DONE, _work_done},
  {ELM_FILE_VIEW_EVENT_WORKING_START, _work_start}
);


EOLIAN static void
_elm_file_selector_view_set(Eo *obj, Elm_File_Selector_Data *pd, const Efl_Class *klass)
{
   pd->view.klass = klass;
   if (pd->view.obj)
     {
        efl_del(pd->view.obj);
        elm_drag_item_container_del(pd->view.obj);
        elm_drop_target_del(pd->view.obj, ELM_SEL_FORMAT_TARGETS,
                               NULL, obj, NULL, obj,
                               NULL, NULL, _dnd_view_drop_cb, obj);
     }
   pd->view.obj = efl_add(pd->view.klass, obj);
   elm_drag_item_container_add(pd->view.obj, 0.3, 0.3, _dnd_item_get_cb, _dnd_data_get_cb);
   elm_drop_target_add(pd->view.obj, ELM_SEL_FORMAT_TARGETS,
                               NULL, obj, NULL, obj,
                               NULL, NULL, _dnd_view_drop_cb, obj);
   elm_object_part_content_set(obj, "content", pd->view.obj);
   evas_object_show(pd->view.obj);
   efl_event_callback_array_add(pd->view.obj, view_events(), obj);
   elm_file_view_iconsize_set(pd->view.obj, config->icon_size);
   elm_file_view_filter_set(pd->view.obj, pd->filter);

   _filter_update_hidden(obj, pd);
   _filter_update_only_folder(obj, pd);
   elm_file_view_file_set(pd->view.obj, pd->file);
}

EOLIAN static const Efl_Class *
_elm_file_selector_view_get(Eo *obj EINA_UNUSED, Elm_File_Selector_Data *pd)
{
   return pd->view.klass;
}
/*
 *======================================
 * Object co/de - struction
 *======================================
 */
EOLIAN static Efl_Object *
_elm_file_selector_efl_object_constructor(Eo *obj, Elm_File_Selector_Data *pd)
{
   Eo *eo;
   const Efl_Class *view;

   efm_init();
   elm_ext_config_init();

   eo =  efl_constructor(efl_super(obj, ELM_FILE_SELECTOR_CLASS));

   if (!views)
     _views_standart_init();

   pd->filter = efl_add(EFM_FILTER_CLASS, NULL);
   _filter_update_hidden(obj, pd);
   _filter_update_only_folder(obj, pd);

   pd->cache = elm_file_mimetype_cache_generate(ELM_FILE_MIMETYPE_CACHE_CLASS, config->icon_size);


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

   return eo;
}

EOLIAN static void
_elm_file_selector_efl_object_destructor(Eo *obj, Elm_File_Selector_Data *pd)
{
   efl_del(pd->cache);
   elm_ext_config_shutdown();
   efl_destructor(efl_super(obj, ELM_FILE_SELECTOR_CLASS));
   efm_shutdown();
}

EOLIAN static void
_elm_file_selector_file_set(Eo *obj, Elm_File_Selector_Data *pd, Efm_File *file)
{
   if (pd->file == file) return;

   if (pd->file) efl_unref(pd->file);
   pd->file = file;
   if (pd->file) efl_ref(pd->file);

   elm_file_view_file_set(pd->view.obj, pd->file);
   efl_event_callback_call(obj, ELM_FILE_SELECTOR_EVENT_PATH_CHANGED, pd->file);

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
static void
_event_rect_mouse_move(void *data, const Efl_Event *event)
{
   PRIV_DATA(data)
   Efl_Input_Pointer *ev = event->info;
   Eina_Rectangle view, selection;
   int move_x, move_y;

   if (!pd->event.selection)
     {
        pd->event.selection = elm_layout_add(event->object);
        if (!elm_layout_theme_set(pd->event.selection, "file_display", "selection", "default"))
          ERR("Failed to set selection theme, selection is invisible ...");
        evas_object_show(pd->event.selection);
     }

   selection.x = pd->event.startpoint.x;
   selection.y = pd->event.startpoint.y;

   efl_input_pointer_position_get(ev, &move_x, &move_y);

   selection.w = move_x - pd->event.startpoint.x;
   selection.h = move_y - pd->event.startpoint.y;

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
}

static void
_event_rect_mouse_up(void *data, const Efl_Event *event)
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

   efl_event_callback_del(event->object, EFL_EVENT_POINTER_MOVE, _event_rect_mouse_move, data);
   efl_event_callback_del(event->object, EFL_EVENT_POINTER_UP, _event_rect_mouse_up, data);
}

static void
_event_rect_mouse_down(void *data, const Efl_Event *event)
{
   PRIV_DATA(data)
   Eina_List *icons;
   Efl_Input_Pointer *ev = event->info;
   Eina_Rectangle view, search;

   EINA_RECTANGLE_SET(&search, 0, 0, 1, 1);
   efl_input_pointer_position_get(ev, &search.x, &search.y);

   icons = elm_file_view_search_items(pd->view.obj, &search);
   elm_file_view_size_get(pd->view.obj, &view);

   if (!eina_rectangle_coords_inside(&view, search.x, search.y))
     return;

   if (efl_input_pointer_button_get(ev) == 1 && !icons)
     {
        pd->event.startpoint.x = search.x;
        pd->event.startpoint.y = search.y;
        efl_event_callback_add(event->object, EFL_EVENT_POINTER_MOVE, _event_rect_mouse_move, data);
        efl_event_callback_add(event->object, EFL_EVENT_POINTER_UP, _event_rect_mouse_up, data);
     }
   else if (efl_input_pointer_button_get(ev) == 3)
     {
        Elm_File_Icon *file_icon = eina_list_data_get(icons);
        _ctx_menu_open(data, search.x, search.y, file_icon);
     }
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
   Eina_Rectangle search;

   EINA_RECTANGLE_SET(&search, x, y, 1, 1);
   icons = elm_file_view_search_items(obj, &search);

   if (!icons)
     return NULL;

   icon = eina_list_data_get(icons);

   evas_object_geometry_get(icon, &xx, &yy, NULL, NULL);

   if (xposret)
     *xposret = xx;
   if (yposret)
     *yposret = yy;

   return icon;
}

static Eina_List*
_dnd_anim_ics_gen(Evas_Object *obj, Evas_Object *view)
{
   PRIV_DATA(obj)
   Eina_List *node, *list;
   Eina_List *result = NULL;
   Elm_File_Icon *icon;


   list = elm_file_view_selection_get(view);

   EINA_LIST_FOREACH(list, node, icon)
     {
        Eina_Rectangle place;
        Eo *widget_icon;
        const char *mimetype;

        widget_icon = elm_icon_add(view);
        evas_object_geometry_get(icon, &place.x, &place.y, &place.w, &place.h);
        evas_object_geometry_set(widget_icon, place.x, place.y, place.w, place.h);

        if (efm_file_is_type(icon, EFM_FILE_TYPE_DIRECTORY))
          mimetype = "folder";
        else
          mimetype = efm_file_mimetype_get(icon);
        elm_file_mimetype_cache_mimetype_set(pd->cache, widget_icon, mimetype);
        evas_object_show(widget_icon);

        result = eina_list_append(result, widget_icon);
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
        const char *path;

        path = efm_file_path_get(icon);
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
   Elm_File_Icon *file;
   int i = 0;

   res = elm_layout_add(win);
   if (!elm_layout_theme_set(res, "file_display", "drag_icon", "default"))
     ERR("Failed to set selection theme, selection is invisible ...");

   evas_object_move(res, 0, 0);
   evas_object_resize(res, 60, 60);

   EINA_LIST_FOREACH(pass->icons, node, file)
     {
        char buf[PATH_MAX];
        Eo *widget_icon;
        const char *mimetype;

        widget_icon = elm_icon_add(win);

        if (efm_file_is_type(file, EFM_FILE_TYPE_DIRECTORY))
          mimetype = "folder";
        else
          mimetype = efm_file_mimetype_get(file);

        elm_file_mimetype_cache_mimetype_set(pass->cache, widget_icon, mimetype);
        evas_object_show(widget_icon);

        i++;
        snprintf(buf, sizeof(buf), "img%d", i);
        elm_object_part_content_set(res, buf, widget_icon);
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
    Eina_List *anim_icons;

    parent = efl_parent_get(obj);
    PRIV_DATA(parent)

    anim_icons = _dnd_anim_ics_gen(parent, obj);
    pass->cache = pd->cache;

    //copy 3 items
    {
       Eina_List *selection = elm_file_view_selection_get(obj);
       for (int i = 0; i < 3; i++)
         {
            Eo *icon = eina_list_data_get(selection);

            pass->icons = eina_list_append(pass->icons, icon);

            selection = eina_list_next(selection);

            if (!selection) break;
         }
    }
    info->format = ELM_SEL_FORMAT_TARGETS;
    info->icons = anim_icons;
    info->data = _dnd_items_gen(obj);
    info->action = ELM_XDND_ACTION_ASK;
    info->createicon = _dnd_create_icon;
    info->createdata = pass;
    info->dragstart = _dnd_drag_start;
    info->startcbdata = pass;

    return EINA_TRUE;
}

static Eina_Bool
_dnd_view_drop_cb(void *data, Evas_Object *obj EINA_UNUSED, Elm_Selection_Data *ev)
{
   //if someone does stop the event, the drop action is not performed
   return efl_event_callback_call(data, ELM_FILE_SELECTOR_EVENT_DND_DROPED, ev);
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
   Eina_Bool empty;

   if (pd->search.old_regex)
     {
        efm_filter_attribute_del(pd->filter, EFM_ATTRIBUTE_FILENAME, pd->search.old_regex, REG_ICASE);
        free(pd->search.old_regex);
        pd->search.old_regex = NULL;
     }

   //prepare the new text and the new regex
   if (pd->search.buffer)
     {
        search = eina_strbuf_string_get(pd->search.buffer);
        elm_layout_signal_emit(obj, "search,enable", "elm");
        pd->search.old_regex = search ? strdup(search) : NULL;
        efm_filter_attribute_add(pd->filter, EFM_ATTRIBUTE_FILENAME, pd->search.old_regex, REG_ICASE);
     }

   if (!pd->search.old_regex)
     elm_layout_signal_emit(obj, "search,disable", "elm");

   elm_layout_text_set(obj, "search", search);


   empty = !!elm_file_view_count(pd->view.obj);

   if (!empty)
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
_elm_file_selector_elm_widget_widget_event(Eo *obj, Elm_File_Selector_Data *pd, Evas_Object *source EINA_UNUSED, Evas_Callback_Type type, void *event_info)
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
         (isprint(ev->string[0]) || ev->string[0] == '.')
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
_apply(void *data, const Efl_Event *event)
{
   Evas_Object *entry = evas_object_data_get(event->object, "__entry");
   const char *dir;
   char path[PATH_MAX];

   dir = efm_file_path_get(data);
   snprintf(path, sizeof(path), "%s/%s", dir, elm_object_text_get(entry));

   if (ecore_file_exists(path))
     {
        //FIXME some better message
        elm_win_title_set(event->object, "File already exists");
     }
   else
     {
        ecore_file_mkdir(path);
        evas_object_del(event->object);
     }
}

static void
_cancel(void *data EINA_UNUSED, const Efl_Event *event)
{
   evas_object_del(event->object);
}

static void
_ctx_new_folder(void *data, Evas_Object *obj EINA_UNUSED, void *event EINA_UNUSED)
{
  Efm_File *file;
  Elm_Dialog_Decision *dialog;
  Evas_Object *entry;

  file = elm_file_selector_file_get(data);
  dialog = efl_add(ELM_DIALOG_DECISION_CLASS, NULL);

  elm_win_title_set(dialog, "Give the file a name");
  elm_dialog_icon_set(dialog, "dialog-question");

  entry = elm_entry_add(dialog);
  evas_object_size_hint_align_set(entry, EVAS_HINT_FILL, EVAS_HINT_FILL);
  evas_object_size_hint_weight_set(entry, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
  elm_entry_single_line_set(entry, EINA_TRUE);
  elm_entry_scrollable_set(entry, EINA_FALSE);
  efl_content_set(dialog, entry);
  elm_object_part_text_set(entry, "guide", "New directories name");
  evas_object_show(entry);

  evas_object_data_set(dialog, "__entry", entry);

  evas_object_show(dialog);

  efl_event_callback_add(dialog, ELM_DIALOG_DECISION_EVENT_APPLY, _apply, file);
  efl_event_callback_add(dialog, ELM_DIALOG_DECISION_EVENT_CANCEL, _cancel, file);
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
_ctx_menu_open(Eo* obj, int x, int y, Efm_File *file)
{
   Evas_Object *menu;
   Elm_Object_Item *it, *it2;
   Elm_File_Selector_Menu_Hook ev;

   menu = elm_menu_add(obj);
   evas_object_data_set(menu, "__w", obj);

   ev.menu = menu;
   ev.file = file;

   efl_event_callback_call(obj, ELM_FILE_SELECTOR_EVENT_HOOK_MENU_SELECTOR_START,&ev);
   elm_menu_item_separator_add(menu, NULL);
   /*
    * New folder
    */
   it = elm_menu_item_add(menu, NULL, "folder-new", "New Folder", _ctx_new_folder, obj);

   elm_menu_item_separator_add(menu, NULL);
   /*
    * Views
    */
   it = elm_menu_item_add(menu, NULL, NULL, "Views", NULL, NULL);
   const char *name;

   if (views)
     {
        Eina_Iterator *iter;
        const Efl_Class *klass;

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
   efl_event_callback_call(obj, &_ELM_FILE_SELECTOR_EVENT_HOOK_MENU_SELECTOR_END, &ev);
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
   efl_del(pd->cache);

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

static void
_drop_cb(void *data, const Efl_Event *event)
{
   Elm_File_Selector_Dnd_Drop_Event ev;

   ev.file = event->object;
   ev.selection_data = event->info;

   efl_ref(event->object);
   efl_event_callback_call(data, ELM_FILE_SELECTOR_EVENT_DND_ITEM_DROPED, &ev);
   efl_unref(event->object);
}

static void
_hover_cb(void *data, const Efl_Event *event)
{
   efl_ref(event->object);
   efl_event_callback_call(data, ELM_FILE_SELECTOR_EVENT_DND_ITEM_HOVER, event->object);
   efl_unref(event->object);
}

EOLIAN static Elm_File_Icon *
_elm_file_selector_icon_generate(Eo *obj, Elm_File_Selector_Data *pd EINA_UNUSED, Efm_File *file)
{
   Evas_Object *ic;

#if 1
   ic = efl_add(ELM_FILE_ICON_CLASS, obj,
    elm_file_icon_cache_set(efl_added, pd->cache),
    elm_file_icon_file_set(efl_added, file),
    elm_file_icon_preview_set(efl_added, config->image_preview)
   );
   efl_event_callback_add(ic, ELM_FILE_ICON_EVENT_ITEM_DROP, _drop_cb, obj);
   efl_event_callback_add(ic, ELM_FILE_ICON_EVENT_ITEM_HOVER, _hover_cb, obj);
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
