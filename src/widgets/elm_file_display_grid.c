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

static void
_del(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *obj, void *event EINA_UNUSED)
{
   View_Context *ctx = evas_object_data_get(obj, "__ctx");

   elm_gengrid_clear(obj);

   if (ctx->fm)
     eo_del(ctx->fm);

   eina_hash_free(ctx->files);
   elm_gengrid_item_class_free(ctx->gic);
   free(ctx);
}

static Evas_Object*
object_init(Evas_Object *par)
{
   Evas_Object *grid_obj;
   View_Context *ctx;

   ctx = calloc(1, sizeof(View_Context));

   grid_obj = elm_gengrid_add(par);
   evas_object_event_callback_add(grid_obj, EVAS_CALLBACK_DEL, _del, NULL);
   evas_object_data_set(grid_obj, "__ctx", ctx);
   elm_gengrid_item_size_set(grid_obj, config->icon_size, config->icon_size);
   elm_gengrid_multi_select_mode_set(grid_obj, ELM_OBJECT_MULTI_SELECT_MODE_WITH_CONTROL );
   elm_gengrid_multi_select_set(grid_obj, EINA_TRUE);
   evas_object_smart_callback_add(grid_obj, "clicked,double", _double_click, NULL);

   ctx->files = eina_hash_pointer_new(NULL);

   ctx->gic = elm_gengrid_item_class_new();
   ctx->gic->item_style = "default";
   ctx->gic->func.text_get = NULL;
   ctx->gic->func.content_get = _grid_content_get;
   ctx->gic->func.state_get = _grid_state_get;

   return grid_obj;
}

static Efm_File*
item_get(Evas_Object *grid_obj, int x, int y)
{
   Elm_Object_Item *it;
   Efm_File *fm_file;

   it = elm_gengrid_at_xy_item_get(grid_obj, x, y, NULL, NULL);
   if (!it)
     return NULL;
   fm_file = elm_object_item_data_get(it);
   return fm_file;
}

static void
items_select(Evas_Object *grid_obj, int x1, int y1, int w, int h)
{
   int x = 0, y = 0;
   Elm_Object_Item *it;

   for (x = x1; x < x1+w; x += config->icon_size)
   {
      for (y = y1; y < y1+h; y += config->icon_size)
      {
         it = elm_gengrid_at_xy_item_get(grid_obj, x, y, NULL, NULL);
         elm_gengrid_item_selected_set(it, EINA_TRUE);
      }
   }
}

static Eina_List*
selections_get(Evas_Object *grid_obj)
{
   const Eina_List *sel_list, *node;
   Eina_List *result = NULL;
   Elm_Object_Item *it;
   Efm_File *file;
   File_Display_View_DND *dnd;
   Evas_Object *content;
   int x = 0, y = 0, w = 20, h = 20;

   sel_list = elm_gengrid_selected_items_get(grid_obj);

   EINA_LIST_FOREACH(sel_list, node, it)
     {
        content = elm_object_item_part_content_get(it, "elm.swallow.icon");

        evas_object_geometry_get(content, &x, &y, &w, &h);

        dnd = calloc(1, sizeof(File_Display_View_DND));
        dnd->x = x;
        dnd->y = y;
        dnd->w = w;
        dnd->h = h;

        file = elm_object_item_data_get(it);
        eo_do(file, dnd->path = efm_file_obj_path_get());

        dnd->thumb_path = NULL;
        dnd->thumb_group = NULL;
        eo_do(content, elm_obj_file_icon_fill_sample(&dnd->thumb_group, &dnd->thumb_path));
        result = eina_list_append(result, dnd);
     }
     return result;
}

static void
_sel(void *data EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   util_item_select(elm_object_parent_widget_get(obj), data);
}

static Eina_Bool
_file_del(void *data, Eo *obj EINA_UNUSED, const Eo_Event_Description *desc EINA_UNUSED, void *event)
{
   Efm_File *icon = event;
   View_Context *ctx = evas_object_data_get(data, "__ctx");
   Elm_Object_Item *it;

   it = eina_hash_find(ctx->files, &icon);
   elm_object_item_del(it);
   eina_hash_del(ctx->files, &icon, it);

   return EINA_TRUE;
}

static Eina_Bool
_file_add(void *data, Eo *obj EINA_UNUSED, const Eo_Event_Description *desc EINA_UNUSED, void *event)
{
   Efm_File *icon = event;
   View_Context *ctx = evas_object_data_get(data, "__ctx");
   Elm_Object_Item *it;

   it = elm_gengrid_item_sorted_insert(data, ctx->gic, icon, sort_func, _sel, icon);
   eina_hash_add(ctx->files, &icon, it);

   return EINA_TRUE;
}

static Eina_Bool
_error(void *data, Eo *obj EINA_UNUSED, const Eo_Event_Description *desc EINA_UNUSED, void *event EINA_UNUSED)
{
   View_Context *ctx = evas_object_data_get(data, "__ctx");
   elm_gengrid_clear(data);
   ctx->fm = NULL;

   return EINA_TRUE;
}

static void
dir_changed(Evas_Object *ww, const char *dir)
{
   View_Context *ctx = evas_object_data_get(ww, "__ctx");
   if (ctx->fm)
     eo_del(ctx->fm);
   eina_hash_free(ctx->files);
   ctx->files = eina_hash_pointer_new(NULL);
   elm_gengrid_clear(ww);
   elm_gengrid_item_size_set(ww, config->icon_size, config->icon_size);
   eo_do(EFM_MONITOR_CLASS, ctx->fm = efm_monitor_obj_start(dir, config->hidden_files,
                              EINA_FALSE));
   eo_do(ctx->fm, eo_event_callback_add(EFM_MONITOR_EVENT_FILE_ADD, _file_add, ww);
                  eo_event_callback_add(EFM_MONITOR_EVENT_FILE_DEL, _file_del, ww);
                  eo_event_callback_add(EFM_MONITOR_EVENT_ERROR, _error, ww);
        );
}

static void
size_get(Evas_Object *wid, int *x, int *y, int *w, int *h)
{
  eo_do(wid,
    elm_interface_scrollable_content_viewport_geometry_get(x, y, w, h));
}

Elm_File_Display_View_Callbacks grid = {
   item_get, /* item_get */
   items_select, /* items_select */
   selections_get, /* selections_get */
   object_init, /* obj_get */
   dir_changed, /* dir_changed */
   size_get
};