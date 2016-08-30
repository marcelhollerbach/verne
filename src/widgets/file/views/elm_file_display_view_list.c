#include "View.h"

typedef struct {
   Elm_Genlist_Item_Class *gic;
   struct {
      int icon_size;
   } config;
   View_Common common;
} Elm_File_Display_View_List_Data;

static Eina_Bool
empty_check(Evas_Object *obj)
{
   if (!elm_genlist_selected_items_get(obj))
     {
        Elm_Gengrid_Item *it;

        it = elm_genlist_first_item_get(obj);

        elm_genlist_item_selected_set(it, EINA_TRUE);
        return EINA_FALSE;
     }
   return EINA_TRUE;
}

static void
_item_select_swap(Evas_Object *obj, const Eina_List *selected, Elm_Object_Item *it)
{
   Eina_List *sel = eina_list_clone(selected);
   Elm_Object_Item *mover;

   EINA_LIST_FREE(sel, mover)
     {
        elm_genlist_item_selected_set(mover, EINA_FALSE);
     }

   elm_genlist_item_selected_set(it, EINA_TRUE);
   empty_check(obj);
}

EOLIAN static const char *
_elm_file_display_view_list_elm_file_view_name_get(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED)
{
   return "List";
}

EOLIAN static Eina_List*
_elm_file_display_view_list_elm_file_view_search_items(Eo *obj, Elm_File_Display_View_List_Data *pd EINA_UNUSED, Eina_Rectangle *search)
{
   Elm_Object_Item *item;
   Eina_List *result = NULL;

   for (int x = search->x; x < search->x+search->w; x += 100)
     {
        for (int y = search->y; y < search->y+search->h; y += 100)
          {
             Evas_Object *icon;
             item = elm_genlist_at_xy_item_get(obj, x, y, 0);
             elm_genlist_item_selected_set(item, EINA_TRUE);


             icon = elm_object_item_part_content_get(item, "elm.swallow.icon");
             result = eina_list_append(result, icon);
          }
     }
   return result;
}

EOLIAN static Eina_List *
_elm_file_display_view_list_elm_file_view_selection_get(Eo *obj EINA_UNUSED, Elm_File_Display_View_List_Data *pd EINA_UNUSED)
{
   const Eina_List *sel_list, *node;
   Eina_List *result = NULL;
   Elm_Object_Item *it;

   sel_list = elm_genlist_selected_items_get(obj);

   EINA_LIST_FOREACH(sel_list, node, it)
     {
        Evas_Object *content;

        content = elm_object_item_part_content_get(it, "elm.swallow.icon");

        result = eina_list_append(result, content);
     }
     return result;
}

EOLIAN static void
_elm_file_display_view_list_elm_file_view_selection_set(Eo *obj EINA_UNUSED, Elm_File_Display_View_List_Data *pd EINA_UNUSED, Eina_List *file)
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


static void
_sel(void *data EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   efl_event_callback_call(obj, ELM_FILE_VIEW_EVENT_ITEM_SELECT_SIMPLE, data);
}

static void
_file_del(View_Common *common EINA_UNUSED, Elm_Object_Item *it)
{
   elm_object_item_del(it);
}

static Elm_Object_Item*
_file_add(View_Common *common EINA_UNUSED, Efm_File *file)
{
   Elm_File_Display_View_List_Data *pd = efl_data_scope_get(common->obj, ELM_FILE_DISPLAY_VIEW_LIST_CLASS);

   return elm_genlist_item_sorted_insert(common->obj, pd->gic, file, NULL, 0, sort_func, _sel, file);
}

static void
_error(View_Common *common EINA_UNUSED)
{
   //TODO something really bad happendhi
}

static void
_file_select(View_Common *common EINA_UNUSED, Elm_Object_Item *it, Eina_Bool sel)
{
   elm_genlist_item_selected_set(it, sel);
}

EOLIAN static void
_elm_file_display_view_list_elm_file_view_file_set(Eo *obj, Elm_File_Display_View_List_Data *pd, Efm_File *file)
{
  view_file_set(&pd->common, file);
  elm_genlist_clear(obj);
}

EOLIAN static void
_elm_file_display_view_list_elm_file_view_size_get(Eo *obj EINA_UNUSED, Elm_File_Display_View_List_Data *pd EINA_UNUSED, Eina_Rectangle *size)
{
   elm_interface_scrollable_content_viewport_geometry_get(obj, &size->x, &size->y, &size->w, &size->h);
}

EOLIAN static void
_elm_file_display_view_list_elm_file_view_iconsize_set(Eo *obj EINA_UNUSED, Elm_File_Display_View_List_Data *pd, int iconsize)
{
   pd->config.icon_size = iconsize;
}

EOLIAN static Eina_Bool
_elm_file_display_view_list_elm_file_view_search(Eo *obj, Elm_File_Display_View_List_Data *pd, const char *needle)
{
   Elm_Object_Item *searched;
   const Eina_List *selected;

   if (!needle) return EINA_TRUE;

   searched = view_search(&pd->common, needle);

   if (!searched) return EINA_FALSE;
   selected = elm_genlist_selected_items_get(obj);
   _item_select_swap(obj, selected, searched);

   return EINA_TRUE;
}

static Evas_Object *
_grid_content_get(void *data, Evas_Object *obj, const char *part)
{
   Evas_Object *ic;
   Evas_Object *parent;
   Efm_File *file;

   parent = efl_parent_get(obj);

   if (!!strcmp(part, "elm.swallow.icon")) return NULL;

   file = data;

   ic = elm_file_selector_icon_generate(parent, file);
   elm_object_style_set(ic, "line");

   return ic;
}

static void
_double_click(void *data EINA_UNUSED, Evas_Object *obj, void *event_info)
{
   Elm_Object_Item *it = event_info;
   Efm_File *fmm_file = elm_object_item_data_get(it);

   efl_event_callback_call(obj, ELM_FILE_VIEW_EVENT_ITEM_SELECT_CHOOSEN, fmm_file);
}

static void
_selection_add(void *data EINA_UNUSED, Evas_Object *obj, void *event_info)
{
   Elm_File_Display_View_List_Data *pd = efl_data_scope_get(obj, ELM_FILE_DISPLAY_VIEW_LIST_CLASS);
   Efm_File *file;

   file = elm_object_item_data_get(event_info);

   view_file_select(&pd->common, file);
}

static void
_selection_del(void *data EINA_UNUSED, Evas_Object *obj, void *event_info)
{
   Elm_File_Display_View_List_Data *pd = efl_data_scope_get(obj, ELM_FILE_DISPLAY_VIEW_LIST_CLASS);
   Efm_File *file;

   file = elm_object_item_data_get(event_info);

   view_file_unselect(&pd->common, file);
}

static void
_key_down(void *data, const Eo_Event *event)
{
   Efl_Input_Key *ev = event->info;
   const Eina_List *selected;
   Eo *list = data;
   Elm_Object_Item *mover;

   if (!strcmp(efl_input_key_get(ev), "Return"))
     {
        Efm_File *fmm_file;
        selected = elm_genlist_selected_items_get(list);

        if (eina_list_count(selected) > 1)
          efl_event_callback_stop(event->object);

        mover =  eina_list_data_get(eina_list_last(selected));

        fmm_file = elm_object_item_data_get(mover);
        efl_event_callback_call(list, ELM_FILE_VIEW_EVENT_ITEM_SELECT_CHOOSEN, fmm_file);

        efl_event_callback_stop(event->object);
     }
}

EOLIAN static Efl_Object *
_elm_file_display_view_list_efl_object_constructor(Eo *obj, Elm_File_Display_View_List_Data *pd)
{
   Eo *eo;
   Eo *parent;

   pd->gic = elm_genlist_item_class_new();
   pd->gic->item_style = "default";
   pd->gic->func.content_get = _grid_content_get;

   eo = efl_constructor(efl_super(obj, ELM_FILE_DISPLAY_VIEW_LIST_CLASS));
   elm_genlist_homogeneous_set(obj, EINA_TRUE);

   parent = efl_parent_get(obj);

   efl_event_callback_add(parent, EFL_EVENT_KEY_DOWN, _key_down, obj);
   evas_object_smart_callback_add(obj, "selected", _selection_add, NULL);
   evas_object_smart_callback_add(obj, "unselected", _selection_del, NULL);

   evas_object_smart_callback_add(eo, "clicked,double", _double_click, NULL);

   view_common_init(&pd->common, obj, _file_add, _file_del, _error, _file_select);

   return obj;
}

EOLIAN static void
_elm_file_display_view_list_efl_object_destructor(Eo *obj, Elm_File_Display_View_List_Data *pd)
{
   efl_destructor(efl_super(obj, ELM_FILE_DISPLAY_VIEW_LIST_CLASS));
   elm_genlist_item_class_free(pd->gic);
}

EOLIAN static void
_elm_file_display_view_list_elm_file_view_filter_set(Eo *obj EINA_UNUSED, Elm_File_Display_View_List_Data *pd, Efm_Filter *filter)
{
   view_filter_set(&pd->common, filter);
}


#include "elm_file_display_view_list.eo.x"