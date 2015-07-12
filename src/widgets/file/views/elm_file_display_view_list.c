
#include "../elm_file_display_priv.h"

typedef struct {
   Elm_Genlist_Item_Class *gic;
   Efm_Monitor *fm;
   Eina_Hash *files;
   struct {
      Eina_Bool only_folder;
      Eina_Bool show_hidden;
      int icon_size;
   } config;
   Eina_List *selected;
} Elm_File_Display_View_List_Data;

EOLIAN static const char *
_elm_file_display_view_list_elm_file_display_view_name_get(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED)
{
   return "List";
}

EOLIAN static Elm_File_Icon *
_elm_file_display_view_list_elm_file_display_view_item_get(Eo *obj, Elm_File_Display_View_List_Data *pd EINA_UNUSED, int x, int y)
{
   Eina_List *items;
   Elm_Items_Item *item;
   Elm_File_Icon *fi;

   eo_do(obj, items = elm_items_display_item_search_xywh(x, y, 1, 1));

   if (!items)
     return NULL;

   item = eina_list_data_get(items);

   eo_do(item, fi = elm_items_item_content_get());

   return fi;
}

EOLIAN static void
_elm_file_display_view_list_elm_file_display_view_items_select(Eo *obj, Elm_File_Display_View_List_Data *pd EINA_UNUSED, int x, int y, int w, int h)
{
   Eina_List *items, *node;
   Elm_Items_Item *item;

   eo_do(obj, items = elm_items_display_item_search_xywh(x, y, w, h));

   EINA_LIST_FOREACH(items, node, item)
     {
        eo_do(item, elm_items_item_selected_set(EINA_TRUE));
     }
}

EOLIAN static Eina_List *
_elm_file_display_view_list_elm_file_display_view_selection_get(Eo *obj, Elm_File_Display_View_List_Data *pd)
{
    Eina_List *node, *result = NULL;
    Elm_Items_Item *item;


    EINA_LIST_FOREACH(pd->selected, node, item)
      {
         Elm_File_Display_View_DndFile *dnd;

         dnd = calloc(1, sizeof(Elm_File_Display_View_DndFile));

         eo_do(item, dnd->file_icon = elm_items_item_content_get());

         evas_object_geometry_get(item, &dnd->x, &dnd->y, &dnd->w, &dnd->h);

         result = eina_list_append(result, dnd);
      }
    return result;
}

static Eina_Bool
_realize(void *data, Eo *obj, const Eo_Event_Description2 *desc EINA_UNUSED, void *event EINA_UNUSED)
{
   Evas_Object *ic;
   ic = icon_create(obj, data);
   elm_object_style_set(ic, "line");
   eo_do(obj, elm_items_item_content_set(ic));

   return EO_CALLBACK_CONTINUE;
}

static Eina_Bool
_unrealize(void *data EINA_UNUSED, Eo *obj, const Eo_Event_Description2 *desc EINA_UNUSED, void *event EINA_UNUSED)
{
   Evas_Object *ic;

   eo_do(obj, ic = elm_items_item_content_get());
   evas_object_del(ic);
   eo_do(obj, elm_items_item_content_set(NULL));
   return EO_CALLBACK_CONTINUE;
}

static Eina_Bool
_selected(void *data, Eo *obj, const Eo_Event_Description2 *desc EINA_UNUSED, void *event EINA_UNUSED)
{
   Elm_File_Display_View_List_Data *pd;

   pd = eo_data_scope_get(data, ELM_FILE_DISPLAY_VIEW_LIST_CLASS);

   pd->selected = eina_list_append(pd->selected, obj);

   return EO_CALLBACK_CONTINUE;
}

static Eina_Bool
_unselected(void *data EINA_UNUSED, Eo *obj, const Eo_Event_Description2 *desc EINA_UNUSED, void *event EINA_UNUSED)
{
   Elm_File_Display_View_List_Data *pd;

   pd = eo_data_scope_get(data, ELM_FILE_DISPLAY_VIEW_LIST_CLASS);

   pd->selected = eina_list_remove(pd->selected, obj);

   return EO_CALLBACK_CONTINUE;
}

static Eina_Bool
_clicked(void *data, Eo *obj, const Eo_Event_Description2 *desc EINA_UNUSED, void *event EINA_UNUSED)
{
   Eo *file;

   eo_do(obj, file = eo_key_data_get("__file"));
   eo_do(data, eo_event_callback_call(ELM_FILE_DISPLAY_VIEW_EVENT_ITEM_SELECT_SIMPLE, file));
   return EO_CALLBACK_CONTINUE;
}

static Eina_Bool
_clicked_double(void *data, Eo *obj EINA_UNUSED, const Eo_Event_Description *desc EINA_UNUSED, void *event)
{
   Eo *file;

   eo_do(obj, file = eo_key_data_get("__file"));
   eo_do(data, eo_event_callback_call(ELM_FILE_DISPLAY_VIEW_EVENT_ITEM_SELECT_CHOOSEN, file));
   return EO_CALLBACK_CONTINUE;
}

static Eina_Bool
_file_del(void *data, Eo *obj EINA_UNUSED, const Eo_Event_Description *desc EINA_UNUSED, void *event)
{
   Elm_File_Display_View_List_Data *pd;

   pd = eo_data_scope_get(data, ELM_FILE_DISPLAY_VIEW_LIST_CLASS);
   eina_hash_del_by_key(pd->files, event);
   return EINA_TRUE;
}

static Eina_Bool
_file_add(void *data, Eo *obj EINA_UNUSED, const Eo_Event_Description *desc EINA_UNUSED, void *event)
{
   Efl_Tree_Base *it, *root;
   Elm_Items_Item *item;
   Elm_File_Display_View_List_Data *pd;

   pd = eo_data_scope_get(data, ELM_FILE_DISPLAY_VIEW_LIST_CLASS);
   eo_do(data, root = elm_items_display_tree_get());

   //create new object
   item = eo_add(ELM_ITEMS_ITEM_CLASS, data);
   eo_do(item, eo_key_data_set("__file", event);
               it = elm_items_item_get();
               eo_event_callback_add(ELM_ITEMS_ITEM_EVENT_REALIZE, _realize, event);
               eo_event_callback_add(ELM_ITEMS_ITEM_EVENT_UNREALIZE, _unrealize, event);
               eo_event_callback_add(ELM_ITEMS_ITEM_EVENT_SELECTED, _selected, data);
               eo_event_callback_add(ELM_ITEMS_ITEM_EVENT_UNSELECTED, _unselected, data);
               eo_event_callback_add(ELM_ITEMS_ITEM_EVENT_CLICKED, _clicked, data);
               eo_event_callback_add(ELM_ITEMS_ITEM_EVENT_CLICKED_DOUBLE, _clicked_double, data);
               );

   eo_do(event, eo_key_data_set("__object", item));

   evas_object_size_hint_align_set(item, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(item, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(item);

   eo_do(root, efl_tree_base_append(it, NULL));

   eina_hash_add(pd->files, &event, item);

   return EINA_TRUE;
}

static Eina_Bool
_error(void *data, Eo *obj EINA_UNUSED, const Eo_Event_Description *desc EINA_UNUSED, void *event EINA_UNUSED)
{
   Elm_File_Display_View_List_Data *pd;

   pd = eo_data_scope_get(data, ELM_FILE_DISPLAY_VIEW_LIST_CLASS);

   eina_hash_free(pd->files);
   pd->files = NULL;

   ERR("The world ended up in flames!");

   return EINA_TRUE;
}

static void
_free_file(void *data)
{
   Elm_Items_Item *item = data;

   eo_del(item);
}

EOLIAN static void
_elm_file_display_view_list_elm_file_display_view_path_set(Eo *obj, Elm_File_Display_View_List_Data *pd, const char *dir)
{
   //clear files
   if (pd->files)
     eina_hash_free(pd->files);

   //delete existing monitor
   if (pd->fm)
     eo_del(pd->fm);

   //emit a signal that there is a new selection, =>nothing
   eo_do(obj, eo_event_callback_call(ELM_FILE_DISPLAY_VIEW_EVENT_ITEM_SELECT_CHANGED, NULL));

   pd->files = eina_hash_pointer_new(_free_file);
   eo_do(EFM_MONITOR_CLASS, pd->fm = efm_monitor_start(dir,pd->config.show_hidden,
                              pd->config.only_folder));

   eo_do(pd->fm, eo_event_callback_add(EFM_MONITOR_EVENT_FILE_ADD, _file_add, obj);
                  eo_event_callback_add(EFM_MONITOR_EVENT_FILE_DEL, _file_del, obj);
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

EOLIAN static Eo_Base *
_elm_file_display_view_list_eo_base_constructor(Eo *obj, Elm_File_Display_View_List_Data *pd)
{
   Eo *eo;

   eo_do_super_ret(obj, ELM_FILE_DISPLAY_VIEW_LIST_CLASS, eo, eo_constructor());
   return eo;
}

EOLIAN static void
_elm_file_display_view_list_eo_base_destructor(Eo *obj, Elm_File_Display_View_List_Data *pd)
{
   eo_do_super(obj, ELM_FILE_DISPLAY_VIEW_LIST_CLASS, eo_destructor());
}

#include "elm_file_display_view_list.eo.x"