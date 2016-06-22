#include "View.h"

typedef struct {
    Evas_Object *search;
    Evas_Object *size;
    Evas_Object *working;
    Evas_Object *selectable;
} Elm_File_Display_View_Debug_Data;

EOLIAN static const char *
_elm_file_display_view_debug_elm_file_view_name_get(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED)
{
   return "Debug";
}

EOLIAN static Eina_List *
_elm_file_display_view_debug_elm_file_view_search_items(Eo *obj EINA_UNUSED, Elm_File_Display_View_Debug_Data *pd EINA_UNUSED, Eina_Rectangle *select EINA_UNUSED)
{
   return NULL;
}

EOLIAN static Eina_List *
_elm_file_display_view_debug_elm_file_view_selection_get(Eo *obj EINA_UNUSED, Elm_File_Display_View_Debug_Data *pd EINA_UNUSED)
{
   return NULL;
}

EOLIAN static void
_elm_file_display_view_debug_elm_file_view_selection_set(Eo *obj EINA_UNUSED, Elm_File_Display_View_Debug_Data *pd EINA_UNUSED, Eina_List *selection EINA_UNUSED)
{

}

EOLIAN static void
_elm_file_display_view_debug_elm_file_view_iconsize_set(Eo *obj EINA_UNUSED, Elm_File_Display_View_Debug_Data *pd, int iconsize)
{
   char buf[PATH_MAX];

   snprintf(buf, sizeof(buf), "%d", iconsize);

   elm_object_text_set(pd->size, buf);
}

EOLIAN static void
_elm_file_display_view_debug_elm_file_view_filter_set(Eo *obj EINA_UNUSED, Elm_File_Display_View_Debug_Data *pd EINA_UNUSED, Efm_Filter *filter EINA_UNUSED)
{

}

EOLIAN static void
_elm_file_display_view_debug_elm_file_view_size_get(Eo *obj EINA_UNUSED, Elm_File_Display_View_Debug_Data *pd, Eina_Rectangle *size)
{
   evas_object_geometry_get(pd->selectable, &size->x, &size->y, &size->w, &size->h);
}

EOLIAN static Eina_Bool
_elm_file_display_view_debug_elm_file_view_search(Eo *obj EINA_UNUSED, Elm_File_Display_View_Debug_Data *pd, const char *needle)
{
   elm_object_text_set(pd->search, needle);

   return EINA_FALSE;
}

EOLIAN static void
_changed(void *data EINA_UNUSED, const Eo_Event *event)
{
   Eina_Bool state;

   state = elm_check_state_get(event->object);

   if (state)
     eo_event_callback_call(data, ELM_FILE_VIEW_EVENT_WORKING_START, NULL);
   else
     eo_event_callback_call(data, ELM_FILE_VIEW_EVENT_WORKING_DONE, NULL);
}

EOLIAN static void
_elm_file_display_view_debug_evas_object_smart_add(Eo *obj, Elm_File_Display_View_Debug_Data *pd)
{
   Evas_Object *o;

   evas_obj_smart_add(eo_super(obj, ELM_FILE_DISPLAY_VIEW_DEBUG_CLASS));

   evas_object_size_hint_weight_set(obj, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(obj, EVAS_HINT_FILL, EVAS_HINT_FILL);

   pd->selectable = evas_object_rectangle_add(evas_object_evas_get(obj));
   evas_object_repeat_events_set(pd->selectable, EINA_TRUE);
   evas_object_size_hint_weight_set(pd->selectable, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(pd->selectable, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_color_set(pd->selectable, 255, 0, 0, 255);
   elm_table_pack(obj, pd->selectable, 2, 0, 1, 4);
   evas_object_show(pd->selectable);

   o = elm_label_add(obj);
   elm_object_text_set(o, "Size: ");
   elm_table_pack(obj, o, 0, 0, 1, 1);
   evas_object_show(o);

   pd->size = elm_label_add(obj);
   elm_table_pack(obj, pd->size, 1, 0, 1, 1);
   evas_object_show(pd->size);

   o = elm_label_add(obj);
   elm_object_text_set(o, "Search: ");
   elm_table_pack(obj, o, 0, 1, 1, 1);
   evas_object_show(o);

   pd->search = elm_label_add(obj);
   elm_table_pack(obj, pd->search, 1, 1, 1, 1);
   evas_object_show(pd->search);

   o = elm_label_add(obj);
   elm_object_text_set(o, "Loading animation: ");
   elm_table_pack(obj, o, 0, 2, 1, 1);
   evas_object_show(o);

   pd->working = elm_check_add(obj);
   elm_table_pack(obj, pd->working, 1, 2, 1, 1);
   eo_do(pd->working, eo_event_callback_add(ELM_CHECK_EVENT_CHANGED, _changed, obj));
   evas_object_show(pd->working);

   o = evas_object_rectangle_add(evas_object_evas_get(obj));
   evas_object_repeat_events_set(o, EINA_TRUE);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_pack(obj, o, 0, 3, 2, 1);
   evas_object_show(o);
}


#include "elm_file_display_view_debug.eo.x"