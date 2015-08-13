
#include "../elm_file_display_priv.h"

typedef struct {
   Elm_Genlist_Item_Class *gic;
   Efm_Monitor *fm;
   Eina_Hash *files;
   Eina_List *sel_files;
   struct {
      Eina_Bool only_folder;
      Eina_Bool show_hidden;
      int icon_size;
   } config;
} Elm_File_Display_View_List_Data;

EOLIAN static const char *
_elm_file_display_view_list_elm_file_display_view_name_get(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED)
{
   return "List";
}

EOLIAN static Elm_File_Icon *
_elm_file_display_view_list_elm_file_display_view_item_get(Eo *obj, Elm_File_Display_View_List_Data *pd, int x, int y)
{
   Elm_Object_Item *item;
   Elm_File_Icon *ic;

   item = elm_genlist_at_xy_item_get(obj, x, y, 0);

   if (!item)
     return NULL;

   ic = elm_object_item_part_content_get(item, "elm.swallow.icon");

   return ic;
}

EOLIAN static void
_elm_file_display_view_list_elm_file_display_view_items_select(Eo *obj, Elm_File_Display_View_List_Data *pd, int xs, int ys, int w, int h)
{
   Elm_Object_Item *item;
   for (int x = xs; x < xs+w; x++)
     {
        for (int y = ys; y < ys+h; y++)
          {
             item = elm_genlist_at_xy_item_get(obj, x, y, 0);
             elm_genlist_item_selected_set(item, EINA_TRUE);
          }
     }
}

EOLIAN static Eina_List *
_elm_file_display_view_list_elm_file_display_view_selection_get(Eo *obj, Elm_File_Display_View_List_Data *pd)
{
    return NULL;
}

static Eina_Bool
_file_del(void *data, Eo *obj EINA_UNUSED, const Eo_Event_Description *desc EINA_UNUSED, void *event)
{
   //TODO need to delete
   return EINA_TRUE;
}

static void
_sel(void *data EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   eo_do(obj, eo_event_callback_call(ELM_FILE_DISPLAY_VIEW_EVENT_ITEM_SELECT_SIMPLE, data));
}

static Eina_Bool
_file_add(void *data, Eo *obj EINA_UNUSED, const Eo_Event_Description *desc EINA_UNUSED, void *event)
{
   Efm_File *icon = event;
   Elm_File_Display_View_List_Data *pd = eo_data_scope_get(data, ELM_FILE_DISPLAY_VIEW_LIST_CLASS);
   Elm_Object_Item *it;

   it = elm_genlist_item_append(data, pd->gic, icon, NULL, 0, NULL, NULL);
   it = elm_genlist_item_sorted_insert(data, pd->gic, icon, NULL, 0, sort_func, NULL, icon);
   eina_hash_add(pd->files, &icon, it);
   eo_do(icon, eo_event_callback_add(EO_BASE_EVENT_DEL, _file_del, data));
   return EINA_TRUE;
}

static Eina_Bool
_error(void *data, Eo *obj EINA_UNUSED, const Eo_Event_Description *desc EINA_UNUSED, void *event EINA_UNUSED)
{
   return EINA_TRUE;
}

EOLIAN static void
_elm_file_display_view_list_elm_file_display_view_path_set(Eo *obj, Elm_File_Display_View_List_Data *pd, const char *dir)
{
   // clear files
   if (pd->files)
     eina_hash_free(pd->files);

   // delete existing monitor
   if (pd->fm)
     eo_del(pd->fm);

   // free selected files
   eina_list_free(pd->sel_files);

   // emit a signal that there is a new selection, =>nothing
   eo_do(obj, eo_event_callback_call(ELM_FILE_DISPLAY_VIEW_EVENT_ITEM_SELECT_CHANGED, NULL));
   elm_genlist_clear(obj);

   eo_do(EFM_MONITOR_CLASS, pd->fm = efm_monitor_start(dir,pd->config.show_hidden,
                              pd->config.only_folder));

   pd->files = eina_hash_pointer_new(NULL);
   eo_do(pd->fm, eo_event_callback_add(EFM_MONITOR_EVENT_FILE_ADD, _file_add, obj);
                  eo_event_callback_add(EFM_MONITOR_EVENT_FILE_HIDE, _file_del, obj);
                  eo_event_callback_add(EFM_MONITOR_EVENT_ERROR, _error, obj);
        );
}

EOLIAN static void
_elm_file_display_view_list_elm_file_display_view_size_get(Eo *obj, Elm_File_Display_View_List_Data *pd, int *x, int *y, int *w, int *h)
{
   eo_do(obj, elm_interface_scrollable_content_viewport_geometry_get(x, y, w, h));
}

EOLIAN static void
_elm_file_display_view_list_elm_file_display_view_config_set(Eo *obj, Elm_File_Display_View_List_Data *pd, int iconsize, Eina_Bool only_folder, Eina_Bool hidden_files)
{
   pd->config.icon_size = iconsize;
   pd->config.only_folder = only_folder;
   pd->config.show_hidden = hidden_files;

   if (pd->fm)
      eo_do(pd->fm, efm_monitor_config_hidden_files_set(pd->config.show_hidden);
                    efm_monitor_config_only_folder_set(pd->config.only_folder);
            );
}

EOLIAN static void
_elm_file_display_view_list_elm_file_display_view_search(Eo *obj, Elm_File_Display_View_List_Data *pd, const char *needle)
{
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
   elm_object_style_set(ic, "line");

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
   Elm_File_Display_View_List_Data *pd = eo_data_scope_get(obj, ELM_FILE_DISPLAY_VIEW_GRID_CLASS);
   Efm_File *file;

   file = elm_object_item_data_get(event_info);

   pd->sel_files = eina_list_append(pd->sel_files, file);
   eo_do(obj, eo_event_callback_call(ELM_FILE_DISPLAY_VIEW_EVENT_ITEM_SELECT_CHANGED, pd->sel_files));
}

static void
_selection_del(void *data EINA_UNUSED, Evas_Object *obj, void *event_info)
{
   Elm_File_Display_View_List_Data *pd = eo_data_scope_get(obj, ELM_FILE_DISPLAY_VIEW_GRID_CLASS);
   Efm_File *file;

   file = elm_object_item_data_get(event_info);

   pd->sel_files = eina_list_remove(pd->sel_files, file);
   eo_do(obj, eo_event_callback_call(ELM_FILE_DISPLAY_VIEW_EVENT_ITEM_SELECT_CHANGED, pd->sel_files));
}

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
   const Eina_List *node;
   Elm_Object_Item *mover;

   EINA_LIST_FOREACH(selected, node, mover)
     {
        elm_genlist_item_selected_set(mover, EINA_FALSE);
     }

   elm_genlist_item_selected_set(it, EINA_TRUE);
   empty_check(obj);
}

static Eina_Bool
_key_down(void *data, Eo *obj EINA_UNUSED, const Eo_Event_Description2 *desc EINA_UNUSED, void *event)
{
   Evas_Event_Key_Down *ev = event;
   const Eina_List *selected, *node;
   Eo *list = data;
   Elm_Object_Item *mover;
   Elm_Object_Item *next;
   Elm_Object_Item *sel;
   Evas_Object *track;
   int x,y,w,h;

   selected = elm_genlist_selected_items_get(list);
   EINA_LIST_FOREACH(selected, node, sel)
     {
        Evas_Object *icon;
        Eina_Bool rename;

        icon = elm_object_item_part_content_get(sel, "elm.swallow.icon");
        eo_do(icon, rename = elm_obj_file_icon_rename_get());
        if (rename)
          return EO_CALLBACK_CONTINUE;
     }

   if (!strcmp(ev->key, "Up"))
     {
        if (!empty_check(list))
          return EO_CALLBACK_STOP;

        selected = elm_genlist_selected_items_get(list);
        mover =  eina_list_data_get((selected));
        track = elm_object_item_track(mover);

        evas_object_geometry_get(track, &x, &y, &w, &h);

        y -= w/2;
        x += w/2;

        next = elm_genlist_at_xy_item_get(list, x, y, NULL);

        _item_select_swap(list, eina_list_clone(selected), next);

        return EO_CALLBACK_STOP;
     }
   else if (!strcmp(ev->key, "Down"))
     {
        if (!empty_check(list))
          return EO_CALLBACK_STOP;

        selected = elm_genlist_selected_items_get(list);
        mover =  eina_list_data_get(eina_list_last(selected));
        track = elm_object_item_track(mover);

        evas_object_geometry_get(track, &x, &y, &w, &h);

        y += w+w/2;
        x += w/2;

        next = elm_genlist_at_xy_item_get(list, x, y, NULL);

        _item_select_swap(list, eina_list_clone(selected), next);

        return EO_CALLBACK_STOP;
     }
   else if (!strcmp(ev->key, "Return"))
     {
        Efm_File *fmm_file;
        selected = elm_genlist_selected_items_get(list);

        if (eina_list_count(selected) > 1)
          return EO_CALLBACK_STOP;

        mover =  eina_list_data_get(eina_list_last(selected));

        fmm_file = elm_object_item_data_get(mover);
        eo_do(list, eo_event_callback_call(ELM_FILE_DISPLAY_VIEW_EVENT_ITEM_SELECT_CHOOSEN, fmm_file));

        return EO_CALLBACK_STOP;
     }
   else if (!strcmp(ev->key, "Home"))
     {
        // first item
        eo_do(list, elm_interface_scrollable_page_bring_in(0, 0));
        return EO_CALLBACK_STOP;
     }
   else if (!strcmp(ev->key, "End"))
     {
        // last item
        int h,v;
        eo_do(list, elm_interface_scrollable_last_page_get(&h, &v);
                    elm_interface_scrollable_page_bring_in(h, v));
        return EO_CALLBACK_STOP;
     }
   else if (!strcmp(ev->key, "Next"))
     {
        // next page
        int h,v;
        eo_do(list, elm_interface_scrollable_current_page_get(&h, &v);
                    elm_interface_scrollable_page_bring_in(h, v+1));
        return EO_CALLBACK_STOP;
     }
   else if (!strcmp(ev->key, "Prior"))
     {
        // prior page
        int h,v;
        eo_do(list, elm_interface_scrollable_current_page_get(&h, &v);
                    elm_interface_scrollable_page_bring_in(h, v-1));
        return EO_CALLBACK_STOP;
     }

   return EO_CALLBACK_CONTINUE;
}

EOLIAN static Eo_Base *
_elm_file_display_view_list_eo_base_constructor(Eo *obj, Elm_File_Display_View_List_Data *pd)
{
   Eo *eo;
   Eo *parent;

   pd->gic = elm_genlist_item_class_new();
   pd->gic->item_style = "default";
   pd->gic->func.content_get = _grid_content_get;

   eo_do_super_ret(obj, ELM_FILE_DISPLAY_VIEW_LIST_CLASS, eo, eo_constructor());

   eo_do(obj, elm_interface_scrollable_page_relative_set(1.0, 0.9));
   eo_do(obj, parent = eo_parent_get());

   eo_do(parent, eo_event_callback_add(EVAS_OBJECT_EVENT_KEY_DOWN, _key_down, obj));
   evas_object_smart_callback_add(obj, "selected", _selection_add, NULL);
   evas_object_smart_callback_add(obj, "unselected", _selection_del, NULL);

   evas_object_smart_callback_add(eo, "clicked,double", _double_click, NULL);

   return obj;
}

EOLIAN static void
_elm_file_display_view_list_eo_base_destructor(Eo *obj, Elm_File_Display_View_List_Data *pd)
{
   eo_do_super(obj, ELM_FILE_DISPLAY_VIEW_LIST_CLASS, eo_destructor());
}

#include "elm_file_display_view_list.eo.x"