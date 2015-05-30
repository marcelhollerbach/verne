#define EFM_EO_NEED

#include "elm_file_display_priv.h"

typedef struct
{
   Elm_Gengrid_Item_Class *gic;
   Efm_Monitor *fm;
   Eina_Hash *files;
} View_Context;

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

static Eina_Bool
_grid_state_get(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, const char *part EINA_UNUSED)
{
   return EINA_TRUE;
}

static void
_double_click(void *data EINA_UNUSED, Evas_Object *obj, void *event_info)
{
   Elm_Object_Item *it = event_info;
   Efm_File *fmm_file = elm_object_item_data_get(it);

   util_item_selected(elm_object_parent_widget_get(obj), fmm_file);
}

Elm_File_Display_View_Callbacks grid = {
   item_get, /* item_get */
   items_select, /* items_select */
   selections_get, /* selections_get */
   object_init, /* obj_get */
   dir_changed, /* dir_changed */
   size_get
};