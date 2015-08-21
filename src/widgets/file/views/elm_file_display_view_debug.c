#include "View.h"

typedef struct {

} Elm_File_Display_View_Debug_Data;

EOLIAN static const char *
_elm_file_display_view_debug_elm_file_display_view_name_get(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED)
{
    return "Debug";
}

EOLIAN static Efm_File *
_elm_file_display_view_debug_elm_file_display_view_item_get(Eo *obj EINA_UNUSED, Elm_File_Display_View_Debug_Data *pd EINA_UNUSED, int x EINA_UNUSED, int y EINA_UNUSED)
{
   return NULL;
}

EOLIAN static void
_elm_file_display_view_debug_elm_file_display_view_items_select(Eo *obj EINA_UNUSED, Elm_File_Display_View_Debug_Data *pd EINA_UNUSED, int x EINA_UNUSED, int y EINA_UNUSED, int w EINA_UNUSED, int h EINA_UNUSED)
{

}

EOLIAN static Eina_List *
_elm_file_display_view_debug_elm_file_display_view_selection_get(Eo *obj EINA_UNUSED, Elm_File_Display_View_Debug_Data *pd EINA_UNUSED)
{
    return NULL;
}

EOLIAN static void
_elm_file_display_view_debug_elm_file_display_view_path_set(Eo *obj, Elm_File_Display_View_Debug_Data *pd EINA_UNUSED, const char *dir)
{
    elm_object_text_set(obj, dir);
}

EOLIAN static void
_elm_file_display_view_debug_elm_file_display_view_size_get(Eo *obj, Elm_File_Display_View_Debug_Data *pd EINA_UNUSED, int *x, int *y, int *w, int *h)
{
    evas_object_geometry_get(obj, x, y, w, h);
}
#include "elm_file_display_view_debug.eo.x"