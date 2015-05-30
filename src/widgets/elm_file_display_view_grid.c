#define EFM_EO_NEED

#include "elm_file_display_priv.h"

typedef struct {
   Elm_Gengrid_Item_Class *gic;
   Efm_Monitor *fm;
   Eina_Hash *files;
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
   Efm_File *fm_file;

   it = elm_gengrid_at_xy_item_get(obj, x, y, NULL, NULL);
   if (!it)
     return NULL;
   fm_file = elm_object_item_data_get(it);

   return fm_file;
}

EOLIAN static void
_elm_file_display_view_grid_elm_file_display_view_items_select(Eo *obj EINA_UNUSED, Elm_File_Display_View_Grid_Data *pd EINA_UNUSED, int x, int y, int w, int h)
{
   int x1 = 0, y1 = 0;
   Elm_Object_Item *it;

   for (x1 = x; x < x+w; x += config->icon_size)
     {
        for (y1 = y; y < y+h; y += config->icon_size)
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
        Elm_File_Display_View_File *dnd;
        Evas_Object *content;

        content = elm_object_item_part_content_get(it, "elm.swallow.icon");
        evas_object_geometry_get(content, &x, &y, &w, &h);

        dnd = calloc(1, sizeof(Elm_File_Display_View_File));
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

   it = eina_hash_find(pd->files, &icon);
   elm_object_item_del(it);
   eina_hash_del(pd->files, &icon, it);

   return EINA_TRUE;
}

static Eina_Bool
_file_add(void *data, Eo *obj EINA_UNUSED, const Eo_Event_Description *desc EINA_UNUSED, void *event)
{
   Efm_File *icon = event;
   Elm_File_Display_View_Grid_Data *pd = eo_data_scope_get(data, ELM_FILE_DISPLAY_VIEW_GRID_CLASS);
   Elm_Object_Item *it;

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

   pd->files = eina_hash_pointer_new(NULL);

   elm_gengrid_clear(obj);
   elm_gengrid_item_size_set(obj, config->icon_size, config->icon_size);
   eo_do(EFM_MONITOR_CLASS, pd->fm = efm_monitor_obj_start(dir, config->hidden_files,
                              EINA_FALSE));

   eo_do(pd->fm, eo_event_callback_add(EFM_MONITOR_EVENT_FILE_ADD, _file_add, obj);
                  eo_event_callback_add(EFM_MONITOR_EVENT_FILE_DEL, _file_del, obj);
                  eo_event_callback_add(EFM_MONITOR_EVENT_ERROR, _error, obj);
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

EOLIAN static Eo *
_elm_file_display_view_grid_eo_base_constructor(Eo *obj, Elm_File_Display_View_Grid_Data *pd)
{
   Eo *eo;


   pd->gic = elm_gengrid_item_class_new();
   pd->gic->item_style = "default";
   pd->gic->func.content_get = _grid_content_get;

   eo_do_super_ret(obj, ELM_FILE_DISPLAY_VIEW_GRID_CLASS, eo, eo_constructor());

   elm_gengrid_item_size_set(eo, config->icon_size, config->icon_size);
   elm_gengrid_multi_select_mode_set(eo, ELM_OBJECT_MULTI_SELECT_MODE_WITH_CONTROL );
   elm_gengrid_multi_select_set(eo, EINA_TRUE);

   evas_object_smart_callback_add(eo, "clicked,double", _double_click, NULL);
   return eo;
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