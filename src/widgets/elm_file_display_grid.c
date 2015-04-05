#include "elm_file_display_priv.h"

typedef struct
{
   Elm_Gengrid_Item_Class *gic;
   EFM_Monitor *fm;
   Eina_Hash *files;
} View_Context;

static Evas_Object *
_grid_content_get(void *data, Evas_Object *obj, const char *part)
{
   Evas_Object *ic;
   EFM_File *file;

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
   EFM_File *fmm_file = elm_object_item_data_get(it);

   util_item_selected(elm_object_parent_widget_get(obj), fmm_file);
}

static void
_del(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *obj, void *event EINA_UNUSED)
{
   View_Context *ctx = evas_object_data_get(obj, "__ctx");

   elm_gengrid_clear(obj);

   if (ctx->fm)
     fm_monitor_stop(ctx->fm);

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

static EFM_File*
item_get(Evas_Object *grid_obj, int x, int y)
{
   Elm_Object_Item *it;
   EFM_File *fm_file;

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
   EFM_File *file;
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
        dnd->path = efm_file_path_get(file);

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

static void
add_cb(void *data EINA_UNUSED, EFM_Monitor *mon EINA_UNUSED, EFM_File *icon)
{
   View_Context *ctx = evas_object_data_get(data, "__ctx");
   Elm_Object_Item *it;

   it = elm_gengrid_item_sorted_insert(data, ctx->gic, icon, sort_func, _sel, icon);
   eina_hash_add(ctx->files, &icon, it);
}

static void
mime_cb(void *data EINA_UNUSED, EFM_Monitor *mon EINA_UNUSED, EFM_File *icon)
{
  View_Context *ctx = evas_object_data_get(data, "__ctx");
  Elm_Object_Item *it;
  Evas_Object *ic;

  it = eina_hash_find(ctx->files, &icon);
  if (!it)
    return;

  ic = elm_object_item_part_content_get(it, "elm.swallow.icon");
  if (!ic)
    return;
  //FIXME
  //eo_do(ic, elm_obj_file_icon_mime_set(icon->mime_type));
}

static void
del_cb(void *data EINA_UNUSED, EFM_Monitor *mon EINA_UNUSED, EFM_File *icon)
{
   View_Context *ctx = evas_object_data_get(data, "__ctx");
   Elm_Object_Item *it;

   it = eina_hash_find(ctx->files, &icon);
   elm_object_item_del(it);
}

static void
err_cb(void *data, EFM_Monitor *mon EINA_UNUSED)
{
   elm_gengrid_clear(data);
}

static void
sdel_cb(void *data EINA_UNUSED, EFM_Monitor *mon EINA_UNUSED)
{
   CRI("PANIC DELETE EVERYTHING ...\n");
   elm_gengrid_clear(data);
}

static void
dir_changed(Evas_Object *ww, const char *dir)
{
   View_Context *ctx = evas_object_data_get(ww, "__ctx");
   if (ctx->fm)
     fm_monitor_stop(ctx->fm);
   eina_hash_free(ctx->files);
   ctx->files = eina_hash_pointer_new(NULL);
   elm_gengrid_clear(ww);
   elm_gengrid_item_size_set(ww, config->icon_size, config->icon_size);
   ctx->fm = fm_monitor_start(dir, add_cb, del_cb,
                              mime_cb, err_cb, sdel_cb,
                              ww, config->hidden_files,
                              EINA_FALSE);
}

Elm_File_Display_View_Callbacks grid = {
   item_get, /* item_get */
   items_select, /* items_select */
   selections_get, /* selections_get */
   object_init, /* obj_get */
   dir_changed /* dir_changed */
};