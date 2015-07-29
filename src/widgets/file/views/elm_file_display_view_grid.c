#define EFM_EO_NEED

#include "../elm_file_display_priv.h"

typedef struct {
   Elm_Gengrid_Item_Class *gic;
   Efm_Monitor *fm;
   Eina_Hash *files;
   Eina_List *sel_files;
   struct {
      Eina_Bool only_folder;
      Eina_Bool show_hidden;
      int icon_size;
   } config;
} Elm_File_Display_View_Grid_Data;

EOLIAN static const char *
_elm_file_display_view_grid_elm_file_display_view_name_get(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED)
{
    return "Grid";
}

EOLIAN static Efm_File *
_elm_file_display_view_grid_elm_file_display_view_item_get(Eo *obj EINA_UNUSED, Elm_File_Display_View_Grid_Data *pd EINA_UNUSED, int x EINA_UNUSED, int y EINA_UNUSED)
{
   Elm_Object_Item *it;
   Evas_Object *content;

   it = elm_gengrid_at_xy_item_get(obj, x, y, NULL, NULL);
   if (!it)
     return NULL;

   content = elm_object_item_part_content_get(it, "elm.swallow.icon");
   return content;
}

EOLIAN static void
_elm_file_display_view_grid_elm_file_display_view_items_select(Eo *obj EINA_UNUSED, Elm_File_Display_View_Grid_Data *pd EINA_UNUSED, int x, int y, int w, int h)
{
   int x1 = 0, y1 = 0;
   Elm_Object_Item *it;

   for (x1 = x; x1 < x+w; x1 += pd->config.icon_size)
     {
        for (y1 = y; y1 < y+h; y1 += pd->config.icon_size)
          {
             it = elm_gengrid_at_xy_item_get(obj, x1, y1, NULL, NULL);
             elm_gengrid_item_selected_set(it, EINA_TRUE);
          }
     }
}

EOLIAN static Eina_List *
_elm_file_display_view_grid_elm_file_display_view_selection_get(Eo *obj EINA_UNUSED, Elm_File_Display_View_Grid_Data *pd EINA_UNUSED)
{
   const Eina_List *sel_list, *node;
   Eina_List *result = NULL;
   Elm_Object_Item *it;

   sel_list = elm_gengrid_selected_items_get(obj);

   EINA_LIST_FOREACH(sel_list, node, it)
     {
        int x = 0, y = 0, w = 20, h = 20;
        Elm_File_Display_View_DndFile *dnd;
        Evas_Object *content;

        content = elm_object_item_part_content_get(it, "elm.swallow.icon");
        evas_object_geometry_get(content, &x, &y, &w, &h);

        dnd = calloc(1, sizeof(Elm_File_Display_View_DndFile));
        dnd->x = x;
        dnd->y = y;
        dnd->w = w;
        dnd->h = h;
        dnd->file_icon = content;

        result = eina_list_append(result, dnd);
     }
     return result;
}

static void
_sel(void *data EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   eo_do(obj, eo_event_callback_call(ELM_FILE_DISPLAY_VIEW_EVENT_ITEM_SELECT_SIMPLE, data));
}

static Eina_Bool
_file_del(void *data, Eo *obj EINA_UNUSED, const Eo_Event_Description *desc EINA_UNUSED, void *event)
{
   Efm_File *icon = event;
   Elm_File_Display_View_Grid_Data *pd = eo_data_scope_get(data, ELM_FILE_DISPLAY_VIEW_GRID_CLASS);
   Elm_Object_Item *it;

   eo_do(icon, eo_event_callback_del(EO_BASE_EVENT_DEL, _file_del, data));
   it = eina_hash_find(pd->files, &icon);
   elm_object_item_del(it);
   eina_hash_del(pd->files, &icon, it);

   pd->sel_files =  eina_list_remove(pd->sel_files, icon);

   return EINA_TRUE;
}

static Eina_Bool
_file_add(void *data, Eo *obj EINA_UNUSED, const Eo_Event_Description *desc EINA_UNUSED, void *event)
{
   Efm_File *icon = event;
   Elm_File_Display_View_Grid_Data *pd = eo_data_scope_get(data, ELM_FILE_DISPLAY_VIEW_GRID_CLASS);
   Elm_Object_Item *it;

   eo_do(icon, eo_event_callback_add(EO_BASE_EVENT_DEL, _file_del, data));

   it = elm_gengrid_item_sorted_insert(data, pd->gic, icon, sort_func, _sel, icon);
   eina_hash_add(pd->files, &icon, it);

   return EINA_TRUE;
}

static Eina_Bool
_error(void *data, Eo *obj EINA_UNUSED, const Eo_Event_Description *desc EINA_UNUSED, void *event EINA_UNUSED)
{
   Elm_File_Display_View_Grid_Data *pd = eo_data_scope_get(data, ELM_FILE_DISPLAY_VIEW_GRID_CLASS);

   elm_gengrid_clear(data);
   pd->fm = NULL;

   return EINA_TRUE;
}

EOLIAN static void
_elm_file_display_view_grid_elm_file_display_view_path_set(Eo *obj, Elm_File_Display_View_Grid_Data *pd EINA_UNUSED, const char *dir)
{
   if (pd->fm)
     eo_del(pd->fm);

   if (pd->files)
     eina_hash_free(pd->files);

   eina_list_free(pd->sel_files);
   pd->sel_files = NULL;
   eo_do(obj, eo_event_callback_call(ELM_FILE_DISPLAY_VIEW_EVENT_ITEM_SELECT_CHANGED, pd->sel_files));

   pd->files = eina_hash_pointer_new(NULL);

   elm_gengrid_clear(obj);

   eo_do(EFM_MONITOR_CLASS, pd->fm = efm_monitor_start(dir,pd->config.show_hidden,
                              pd->config.only_folder));

   eo_do(pd->fm, eo_event_callback_add(EFM_MONITOR_EVENT_FILE_ADD, _file_add, obj);
                  eo_event_callback_add(EFM_MONITOR_EVENT_FILE_HIDE, _file_del, obj);
                  eo_event_callback_add(EFM_MONITOR_EVENT_ERROR, _error, obj);
        );
}

EOLIAN static void
_elm_file_display_view_grid_elm_file_display_view_config_set(Eo *obj, Elm_File_Display_View_Grid_Data *pd, int iconsize, Eina_Bool only_folder, Eina_Bool hidden_files)
{
    pd->config.icon_size = iconsize;
    pd->config.only_folder = only_folder;
    pd->config.show_hidden = hidden_files;
    elm_gengrid_item_size_set(obj, pd->config.icon_size, pd->config.icon_size);

    if (pd->fm)
      eo_do(pd->fm, efm_monitor_config_hidden_files_set(pd->config.show_hidden);
                    efm_monitor_config_only_folder_set(pd->config.only_folder);
            );
}

EOLIAN static void
_elm_file_display_view_grid_elm_file_display_view_size_get(Eo *obj, Elm_File_Display_View_Grid_Data *pd EINA_UNUSED, int *x, int *y, int *w, int *h)
{
    eo_do(obj,
    elm_interface_scrollable_content_viewport_geometry_get(x, y, w, h));
}

static Evas_Object *
_grid_content_get(void *data, Evas_Object *obj, const char *part)
{
   Evas_Object *ic;
   Efm_File *file;

   if (!!strcmp(part, "elm.swallow.icon")) return NULL;

   file = data;

   ic = icon_create(obj, file);
   return ic;
}

static void
_double_click(void *data EINA_UNUSED, Evas_Object *obj, void *event_info)
{
   Elm_Object_Item *it = event_info;
   Efm_File *fmm_file = elm_object_item_data_get(it);

   eo_do(obj, eo_event_callback_call(ELM_FILE_DISPLAY_VIEW_EVENT_ITEM_SELECT_CHOOSEN, fmm_file));
}

static void
_selection_add(void *data EINA_UNUSED, Evas_Object *obj, void *event_info)
{
   Elm_File_Display_View_Grid_Data *pd = eo_data_scope_get(obj, ELM_FILE_DISPLAY_VIEW_GRID_CLASS);
   Efm_File *file;

   file = elm_object_item_data_get(event_info);

   pd->sel_files = eina_list_append(pd->sel_files, file);
   eo_do(obj, eo_event_callback_call(ELM_FILE_DISPLAY_VIEW_EVENT_ITEM_SELECT_CHANGED, pd->sel_files));
}

static void
_selection_del(void *data EINA_UNUSED, Evas_Object *obj, void *event_info)
{
   Elm_File_Display_View_Grid_Data *pd = eo_data_scope_get(obj, ELM_FILE_DISPLAY_VIEW_GRID_CLASS);
   Efm_File *file;

   file = elm_object_item_data_get(event_info);

   pd->sel_files = eina_list_remove(pd->sel_files, file);
   eo_do(obj, eo_event_callback_call(ELM_FILE_DISPLAY_VIEW_EVENT_ITEM_SELECT_CHANGED, pd->sel_files));
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
   const Eina_List *node;
   Elm_Object_Item *mover;

   EINA_LIST_FOREACH(selected, node, mover)
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
   const Eina_List *selected, *node;
   Eo *grid = data;
   Elm_Object_Item *mover;
   Elm_Object_Item *next;
   Elm_Object_Item *sel;
   Evas_Object *track;
   int x,y,w,h;

   selected = elm_gengrid_selected_items_get(grid);
   EINA_LIST_FOREACH(selected, node, sel)
     {
        Evas_Object *icon;
        Eina_Bool rename;

        icon = elm_object_item_part_content_get(sel, "elm.swallow.icon");
        eo_do(icon, rename = elm_obj_file_icon_rename_get());
        if (rename)
          return EO_CALLBACK_CONTINUE;
     }

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
        eo_do(grid, eo_event_callback_call(ELM_FILE_DISPLAY_VIEW_EVENT_ITEM_SELECT_CHOOSEN, fmm_file));

        return EO_CALLBACK_STOP;
     }
   else if (!strcmp(ev->key, "Home"))
     {
        //first item
        eo_do(grid, elm_interface_scrollable_page_bring_in(0, 0));
        return EO_CALLBACK_STOP;
     }
   else if (!strcmp(ev->key, "End"))
     {
        //last item
        int h,v;
        eo_do(grid, elm_interface_scrollable_last_page_get(&h, &v);
                    elm_interface_scrollable_page_bring_in(h, v));
        return EO_CALLBACK_STOP;
     }
   else if (!strcmp(ev->key, "Next"))
     {
        //next page
        int h,v;
        eo_do(grid, elm_interface_scrollable_current_page_get(&h, &v);
                    elm_interface_scrollable_page_bring_in(h, v+1));
        return EO_CALLBACK_STOP;
     }
   else if (!strcmp(ev->key, "Prior"))
     {
        //prior page
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
   elm_gengrid_item_size_set(eo, config->icon_size, config->icon_size);
   elm_gengrid_multi_select_mode_set(eo, ELM_OBJECT_MULTI_SELECT_MODE_WITH_CONTROL );
   elm_gengrid_multi_select_set(eo, EINA_TRUE);
   evas_object_smart_callback_add(obj, "selected", _selection_add, NULL);
   evas_object_smart_callback_add(obj, "unselected", _selection_del, NULL);

   evas_object_smart_callback_add(eo, "clicked,double", _double_click, NULL);
   return eo;
}

EOLIAN static void
_elm_file_display_view_grid_elm_file_display_view_search(Eo *obj, Elm_File_Display_View_Grid_Data *pd, const char *needle)
{
   Eina_Iterator *itr;
   Elm_Widget_Item *it;
   Elm_Widget_Item *searched = NULL;
   const Eina_List *selected;
   Efm_File *file;
   const char *filename;
   int min = -1;

   if (!needle)
     return;

   itr = eina_hash_iterator_data_new(pd->files);

   selected = elm_gengrid_selected_items_get(obj);

   EINA_ITERATOR_FOREACH(itr, it)
     {
        char *f;
        file = elm_object_item_data_get(it);
        eo_do(file, filename = efm_file_filename_get());
        if ((f = strstr(filename, needle)))
          {
             int tmin = f - filename;
             if (min == -1)
               min = tmin;

             if (tmin > min)
               continue;
             min = tmin;
             searched = it;
          }
     }

   _item_select_swap(obj, selected, searched);
}

EOLIAN static void
_elm_file_display_view_grid_eo_base_destructor(Eo *obj, Elm_File_Display_View_Grid_Data *pd)
{
   elm_gengrid_clear(obj);

   if (pd->fm)
     eo_del(pd->fm);
   if (pd->files)
     eina_hash_free(pd->files);

   elm_gengrid_item_class_free(pd->gic);

   eo_do_super(obj, ELM_FILE_DISPLAY_VIEW_GRID_CLASS, eo_destructor());
}
#include "elm_file_display_view_grid.eo.x"