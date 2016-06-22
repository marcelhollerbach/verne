#include "View.h"

typedef struct {
   Elm_Gengrid_Item_Class *gic;
   View_Common common;
   struct {
      int icon_size;
   } config;
} Elm_File_Display_View_Grid_Data;

#define SMALL 80
#define SIZE 70

static int
_calc_icon_size(int bounce)
{
   return SMALL + SIZE*((float)bounce/100)*elm_config_scale_get();
}

EOLIAN static const char *
_elm_file_display_view_grid_elm_file_view_name_get(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED)
{
    return "Grid";
}

EOLIAN static Eina_List*
_elm_file_display_view_grid_elm_file_view_search_items(Eo *obj EINA_UNUSED, Elm_File_Display_View_Grid_Data *pd EINA_UNUSED, Eina_Rectangle *view)
{
   int x1 = 0, y1 = 0;
   Elm_Object_Item *it;
   Eina_List *result = NULL;
   int size;

   size = _calc_icon_size(pd->config.icon_size);

   for (x1 = view->x; x1 < view->x+view->w; x1 += size)
     {
        for (y1 = view->y; y1 < view->y+view->h; y1 += size)
          {
             Evas_Object *content;

             it = elm_gengrid_at_xy_item_get(obj, x1, y1, NULL, NULL);

             if (!it) continue;

             content = elm_object_item_part_content_get(it, "elm.swallow.icon");

             result = eina_list_append(result, content);
          }
     }
   return result;
}

EOLIAN static void
_elm_file_display_view_grid_elm_file_view_selection_set(Eo *obj EINA_UNUSED, Elm_File_Display_View_Grid_Data *pd, Eina_List *file)
{
   Eina_List *node;
   Evas_Object *icon;

   EINA_LIST_FOREACH(file, node, icon)
     {
        Efm_File *file;

        file = elm_obj_file_icon_file_get(icon);

        view_file_select(&pd->common, file);
     }
}

EOLIAN static Eina_List *
_elm_file_display_view_grid_elm_file_view_selection_get(Eo *obj EINA_UNUSED, Elm_File_Display_View_Grid_Data *pd EINA_UNUSED)
{
   const Eina_List *sel_list, *node;
   Eina_List *result = NULL;
   Elm_Object_Item *it;

   sel_list = elm_gengrid_selected_items_get(obj);

   EINA_LIST_FOREACH(sel_list, node, it)
     {
        Evas_Object *content;

        content = elm_object_item_part_content_get(it, "elm.swallow.icon");

        result = eina_list_append(result, content);
     }
     return result;
}

static void
_sel(void *data EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   eo_event_callback_call(obj, ELM_FILE_VIEW_EVENT_ITEM_SELECT_SIMPLE, data);
}

static void
_file_del(View_Common *common EINA_UNUSED, Elm_Object_Item *res)
{
   elm_object_item_del(res);
}

static Elm_Object_Item*
_file_add(View_Common *common, Efm_File *file)
{
   Elm_File_Display_View_Grid_Data *pd = eo_data_scope_get(common->obj, ELM_FILE_DISPLAY_VIEW_GRID_CLASS);
   return elm_gengrid_item_sorted_insert(common->obj, pd->gic, file, sort_func, _sel, file);
}

static void
_error(View_Common *common EINA_UNUSED)
{

}

static void
_file_select(View_Common *common EINA_UNUSED, Elm_Object_Item *it, Eina_Bool sel)
{
   elm_gengrid_item_selected_set(it, sel);
}

EOLIAN static void
_elm_file_display_view_grid_elm_file_view_file_set(Eo *obj, Elm_File_Display_View_Grid_Data *pd, Efm_File *file)
{
  view_file_set(&pd->common, file);
  elm_gengrid_clear(obj);
}

EOLIAN static void
_elm_file_display_view_grid_elm_file_view_iconsize_set(Eo *obj, Elm_File_Display_View_Grid_Data *pd, int iconsize)
{
    int size;
    pd->config.icon_size = iconsize;
    size = _calc_icon_size(pd->config.icon_size);
    elm_gengrid_item_size_set(obj, size, size);
}

EOLIAN static void
_elm_file_display_view_grid_elm_file_view_size_get(Eo *obj, Elm_File_Display_View_Grid_Data *pd EINA_UNUSED, Eina_Rectangle *size)
{
    elm_interface_scrollable_content_viewport_geometry_get(obj, &size->x, &size->y, &size->w, &size->h);
}

static Evas_Object *
_grid_content_get(void *data, Evas_Object *obj, const char *part)
{
   Evas_Object *ic;
   Evas_Object *parent;
   Efm_File *file;

   parent = eo_parent_get(obj);

   if (!!strcmp(part, "elm.swallow.icon")) return NULL;

   file = data;

   ic = elm_file_selector_icon_generate(parent, file);
   return ic;
}

static void
_double_click(void *data EINA_UNUSED, Evas_Object *obj, void *event_info)
{
   Elm_Object_Item *it = event_info;
   Efm_File *fmm_file = elm_object_item_data_get(it);

   eo_event_callback_call(obj, ELM_FILE_VIEW_EVENT_ITEM_SELECT_CHOOSEN, fmm_file);
}

static void
_selection_add(void *data EINA_UNUSED, Evas_Object *obj, void *event_info)
{
   Elm_File_Display_View_Grid_Data *pd = eo_data_scope_get(obj, ELM_FILE_DISPLAY_VIEW_GRID_CLASS);
   Efm_File *file;

   file = elm_object_item_data_get(event_info);

   view_file_select(&pd->common, file);
}

static void
_selection_del(void *data EINA_UNUSED, Evas_Object *obj, void *event_info)
{
   Elm_File_Display_View_Grid_Data *pd = eo_data_scope_get(obj, ELM_FILE_DISPLAY_VIEW_GRID_CLASS);
   Efm_File *file;

   file = elm_object_item_data_get(event_info);

   view_file_unselect(&pd->common, file);
}

static Eina_Bool
empty_check(Evas_Object *obj)
{
   if (!elm_gengrid_selected_items_get(obj))
     {
        Elm_Gengrid_Item *it;

        it = elm_gengrid_first_item_get(obj);

        elm_gengrid_item_selected_set(it, EINA_TRUE);
        return EINA_FALSE;
     }
   return EINA_TRUE;
}

static void
_item_select_swap(Evas_Object *obj, const Eina_List *selected, Elm_Object_Item *it)
{
   Eina_List *selected_safe;
   Elm_Object_Item *mover;

   selected_safe = eina_list_clone(selected);

   EINA_LIST_FREE(selected_safe, mover)
     {
        elm_gengrid_item_selected_set(mover, EINA_FALSE);
     }

   elm_gengrid_item_selected_set(it, EINA_TRUE);

   empty_check(obj);
}

static void
_key_down(void *data, const Eo_Event *event)
{
   Evas_Event_Key_Down *ev = event->info;
   const Eina_List *selected;
   Eo *grid = data;
   Elm_Object_Item *mover;

   if (!strcmp(ev->key, "Return"))
     {
        Efm_File *fmm_file;
        selected = elm_gengrid_selected_items_get(grid);

        if (eina_list_count(selected) > 1)
          eo_event_callback_stop(event->object);

        mover =  eina_list_data_get(eina_list_last(selected));

        fmm_file = elm_object_item_data_get(mover);
        eo_event_callback_call(grid, ELM_FILE_VIEW_EVENT_ITEM_SELECT_CHOOSEN, fmm_file);

        eo_event_callback_stop(event->object);
     }
}

EOLIAN static Eo *
_elm_file_display_view_grid_eo_base_constructor(Eo *obj, Elm_File_Display_View_Grid_Data *pd)
{
   Eo *eo;
   Eo *parent;

   pd->gic = elm_gengrid_item_class_new();
   pd->gic->item_style = "default";
   pd->gic->func.content_get = _grid_content_get;

   eo = eo_constructor(eo_super(obj, ELM_FILE_DISPLAY_VIEW_GRID_CLASS));

   elm_gengrid_align_set(obj, 0.5, 0.0);
   parent = eo_parent_get(obj);

   eo_event_callback_add(parent, EFL_CANVAS_OBJECT_EVENT_KEY_DOWN, _key_down, obj);
   elm_gengrid_multi_select_mode_set(eo, ELM_OBJECT_MULTI_SELECT_MODE_WITH_CONTROL );
   elm_gengrid_multi_select_set(eo, EINA_TRUE);
   evas_object_smart_callback_add(obj, "selected", _selection_add, NULL);
   evas_object_smart_callback_add(obj, "unselected", _selection_del, NULL);

   evas_object_smart_callback_add(eo, "clicked,double", _double_click, NULL);

   view_common_init(&pd->common, obj, _file_add, _file_del, _error, _file_select);
   return eo;
}

EOLIAN static Eina_Bool
_elm_file_display_view_grid_elm_file_view_search(Eo *obj, Elm_File_Display_View_Grid_Data *pd, const char *needle)
{
   Elm_Object_Item *searched;
   const Eina_List *selected;

   if (!needle) return EINA_TRUE;

   searched = view_search(&pd->common, needle);
   if (!searched) return EINA_FALSE;

   selected = elm_gengrid_selected_items_get(obj);
   _item_select_swap(obj, selected, searched);

   return EINA_TRUE;
}

EOLIAN static void
_elm_file_display_view_grid_eo_base_destructor(Eo *obj, Elm_File_Display_View_Grid_Data *pd)
{
   elm_gengrid_item_class_free(pd->gic);
   eo_destructor(eo_super(obj, ELM_FILE_DISPLAY_VIEW_GRID_CLASS));
}

EOLIAN static void
_elm_file_display_view_grid_elm_file_view_filter_set(Eo *obj EINA_UNUSED, Elm_File_Display_View_Grid_Data *pd, Efm_Filter *filter)
{
   view_filter_set(&pd->common, filter);
}

#include "elm_file_display_view_grid.eo.x"