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

        eo_do(icon, file = elm_obj_file_icon_file_get());

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
   eo_do(obj, eo_event_callback_call(ELM_FILE_VIEW_EVENT_ITEM_SELECT_SIMPLE, data));
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

EOLIAN static Eina_Bool
_elm_file_display_view_grid_efl_file_file_set(Eo *obj, Elm_File_Display_View_Grid_Data *pd EINA_UNUSED, const char *dir, const char *key EINA_UNUSED)
{
   view_path_set(&pd->common, dir);
   elm_gengrid_clear(obj);
   return EINA_TRUE;
}

EOLIAN static void
_elm_file_display_view_grid_efl_file_file_get(Eo *obj EINA_UNUSED, Elm_File_Display_View_Grid_Data *pd EINA_UNUSED, const char **dir, const char **key)
{
   eo_do(pd->common.monitor, efl_file_get(dir, key));
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
    eo_do(obj,
    elm_interface_scrollable_content_viewport_geometry_get(&size->x, &size->y, &size->w, &size->h));
}

static Evas_Object *
_grid_content_get(void *data, Evas_Object *obj, const char *part)
{
   Evas_Object *ic;
   Evas_Object *parent;
   Efm_File *file;

   eo_do(obj, parent = eo_parent_get());

   if (!!strcmp(part, "elm.swallow.icon")) return NULL;

   file = data;

   eo_do(parent, ic = elm_file_selector_icon_generate(file));
   return ic;
}

static void
_double_click(void *data EINA_UNUSED, Evas_Object *obj, void *event_info)
{
   Elm_Object_Item *it = event_info;
   Efm_File *fmm_file = elm_object_item_data_get(it);

   eo_do(obj, eo_event_callback_call(ELM_FILE_VIEW_EVENT_ITEM_SELECT_CHOOSEN, fmm_file));
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

static Eina_Bool
_key_down(void *data, Eo *obj EINA_UNUSED, const Eo_Event_Description2 *desc EINA_UNUSED, void *event)
{
   Evas_Event_Key_Down *ev = event;
   const Eina_List *selected;
   Eo *grid = data;
   Elm_Object_Item *mover;
   Elm_Object_Item *next;
   Evas_Object *track;
   int x,y,w,h;

   if (!strcmp(ev->key, "Right"))
     {
        if (!empty_check(grid))
          return EO_CALLBACK_STOP;

        selected = elm_gengrid_selected_items_get(grid);
        mover =  eina_list_data_get(eina_list_last(selected));

        next = elm_gengrid_item_next_get(mover);

        _item_select_swap(grid, selected, next);
        return EO_CALLBACK_STOP;
     }
   else if (!strcmp(ev->key, "Left"))
     {
        if (!empty_check(grid))
          return EO_CALLBACK_STOP;

        selected = elm_gengrid_selected_items_get(grid);
        mover =  eina_list_data_get(eina_list_last(selected));

        next = elm_gengrid_item_prev_get(mover);

        _item_select_swap(grid, selected, next);

        return EO_CALLBACK_STOP;
     }
   else if (!strcmp(ev->key, "Up"))
     {
        if (!empty_check(grid))
          return EO_CALLBACK_STOP;

        selected = elm_gengrid_selected_items_get(grid);
        mover =  eina_list_data_get((selected));
        track = elm_object_item_track(mover);

        evas_object_geometry_get(track, &x, &y, &w, &h);

        y -= w/2;
        x += w/2;

        next = elm_gengrid_at_xy_item_get(grid, x, y, NULL, NULL);

        _item_select_swap(grid, eina_list_clone(selected), next);

        return EO_CALLBACK_STOP;
     }
   else if (!strcmp(ev->key, "Down"))
     {
        if (!empty_check(grid))
          return EO_CALLBACK_STOP;

        selected = elm_gengrid_selected_items_get(grid);
        mover =  eina_list_data_get(eina_list_last(selected));
        track = elm_object_item_track(mover);

        evas_object_geometry_get(track, &x, &y, &w, &h);

        y += w+w/2;
        x += w/2;

        next = elm_gengrid_at_xy_item_get(grid, x, y, NULL, NULL);

        _item_select_swap(grid, eina_list_clone(selected), next);

        return EO_CALLBACK_STOP;
     }
   else if (!strcmp(ev->key, "Return"))
     {
        Efm_File *fmm_file;
        selected = elm_gengrid_selected_items_get(grid);

        if (eina_list_count(selected) > 1)
          return EO_CALLBACK_STOP;

        mover =  eina_list_data_get(eina_list_last(selected));

        fmm_file = elm_object_item_data_get(mover);
        eo_do(grid, eo_event_callback_call(ELM_FILE_VIEW_EVENT_ITEM_SELECT_CHOOSEN, fmm_file));

        return EO_CALLBACK_STOP;
     }
   else if (!strcmp(ev->key, "Home"))
     {
        // first item
        eo_do(grid, elm_interface_scrollable_page_bring_in(0, 0));
        return EO_CALLBACK_STOP;
     }
   else if (!strcmp(ev->key, "End"))
     {
        // last item
        int h,v;
        eo_do(grid, elm_interface_scrollable_last_page_get(&h, &v);
                    elm_interface_scrollable_page_bring_in(h, v));
        return EO_CALLBACK_STOP;
     }
   else if (!strcmp(ev->key, "Next"))
     {
        // next page
        int h,v;
        eo_do(grid, elm_interface_scrollable_current_page_get(&h, &v);
                    elm_interface_scrollable_page_bring_in(h, v+1));
        return EO_CALLBACK_STOP;
     }
   else if (!strcmp(ev->key, "Prior"))
     {
        // prior page
        int h,v;
        eo_do(grid, elm_interface_scrollable_current_page_get(&h, &v);
                    elm_interface_scrollable_page_bring_in(h, v-1));
        return EO_CALLBACK_STOP;
     }

   return EO_CALLBACK_CONTINUE;
}

EOLIAN static Eo *
_elm_file_display_view_grid_eo_base_constructor(Eo *obj, Elm_File_Display_View_Grid_Data *pd)
{
   Eo *eo;
   Eo *parent;

   pd->gic = elm_gengrid_item_class_new();
   pd->gic->item_style = "default";
   pd->gic->func.content_get = _grid_content_get;

   eo_do_super(obj, ELM_FILE_DISPLAY_VIEW_GRID_CLASS, eo = eo_constructor());

   eo_do(obj, elm_interface_scrollable_page_relative_set(1.0, 0.9));
   elm_gengrid_align_set(obj, 0.5, 0.0);
   eo_do(obj, parent = eo_parent_get());

   eo_do(parent, eo_event_callback_add(EVAS_OBJECT_EVENT_KEY_DOWN, _key_down, obj));
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
   eo_do_super(obj, ELM_FILE_DISPLAY_VIEW_GRID_CLASS, eo_destructor());
}

EOLIAN static void
_elm_file_display_view_grid_elm_file_view_filter_set(Eo *obj EINA_UNUSED, Elm_File_Display_View_Grid_Data *pd, Efm_Filter *filter)
{
   view_filter_set(&pd->common, filter);
}

#include "elm_file_display_view_grid.eo.x"