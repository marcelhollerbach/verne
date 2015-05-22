#include "elm_file_display_priv.h"

static Eina_List *lst;
static Evas_Object *entry;

static Efm_File*
debug_item_get(Evas_Object *ww EINA_UNUSED, int x, int y)
{
   int x2, y2, w, h;
   Eina_List *node;
   Evas_Object *rect;
   char buf[PATH_MAX];

   EINA_LIST_FOREACH(lst, node, rect)
   {
      evas_object_geometry_get(rect, &x2, &y2, &w, &h);
      if (x > x2 && x < x2 + h &&
          y > y2 && y < y2 + w)
        {
           snprintf(buf, PATH_MAX, "item_get: %d:%d:%p<br>", x, y, rect);
           elm_entry_entry_append(entry, buf);
           return NULL;
        }
   }
   snprintf(buf, PATH_MAX, "item_get: %d:%d:%p<br>", x, y, rect);
   elm_entry_entry_append(entry, buf);
   return NULL;
}

static void
debug_items_get(Evas_Object *ww EINA_UNUSED, int x, int y, int w, int h)
{
   int x2, y2, w2, h2;
   Eina_List *node;
   Evas_Object *rect;
   char buf[PATH_MAX];

   snprintf(buf, PATH_MAX, "items_select: %d:%d:%d:%d<br>", x, y, w, h);
   elm_entry_entry_append(entry, buf);
   EINA_LIST_FOREACH(lst, node, rect)
   {
      evas_object_geometry_get(rect, &x2, &y2, &w2, &h2);
      if ((((x2 > x && x2 < x+w) || (x2 + w2 > x && x2 + w2 < x+w)) || (x > x2 && x < x2 + w2)) &&
          (((y2 > y && y2 < y+h) || (y2 + h2 > y && y2 + h2 < y+h)) || (y > y2 && y < y2 + h2)))
        {
           evas_object_data_set(rect, "selected", (void*) 1);
           evas_object_color_set(rect, 220, 20, 20, 50);
           evas_object_lower(rect);
        }
      else
        {
           evas_object_data_del(rect, "selected");
           evas_object_color_set(rect, 50, 50, 50, 50);
           evas_object_lower(rect);
        }
   }
}

static Eina_List*
debug_selections_get(Evas_Object *w EINA_UNUSED)
{
  return NULL;
}

#define SIZE 20
#define STEP 40
#define OFFSET 10

static void
_resize(void *data, Evas *e EINA_UNUSED, Evas_Object *obj, void *event EINA_UNUSED)
{
   Evas_Object *rect;
   int x = 0, y = 0, ox, oy;
   Evas *evas = data;

   EINA_LIST_FREE(lst, rect)
   {
      evas_object_del(rect);
   }

   evas_object_geometry_get(obj, &ox, &oy, NULL, NULL);

   for (x = 0; x< 3*STEP; x += STEP)
     {
        for (y = 0; y< 3*STEP; y += STEP)
          {
            rect = evas_object_rectangle_add(evas);
            evas_object_move(rect, ox + x + 10, oy + y + 10);
            evas_object_resize(rect, SIZE, SIZE);
            evas_object_color_set(rect, 250, 50, 50, 50);
            evas_object_layer_set(rect, EVAS_LAYER_MAX - 1);
            evas_object_show(rect);

            lst = eina_list_append(lst, rect);
          }
     }
}

static Evas_Object*
debug_object_get(Evas_Object *par)
{
   Evas *evas;

   entry = elm_entry_add(par);
   evas = evas_object_evas_get(entry);
   evas_object_event_callback_add(entry, EVAS_CALLBACK_RESIZE, _resize, evas);
   elm_entry_entry_set(entry, "Debug output<br>");
   elm_entry_editable_set(entry, EINA_FALSE);

   return entry;
}

static void
debug_dir_changed(Evas_Object *w EINA_UNUSED, const char *dir EINA_UNUSED)
{

}

Elm_File_Display_View_Callbacks debug = {
   debug_item_get,
   debug_items_get,
   debug_selections_get,
   debug_object_get,
   debug_dir_changed
};