#include "elm_items_priv.h"


typedef struct {
    Evas_Object *rectangle;
    Evas_Object *content;
    Eina_Bool selected;
} Elm_Items_Item_Data;

EOLIAN static void
_elm_items_item_content_set(Eo *obj, Elm_Items_Item_Data *pd, Evas_Object *content)
{
   pd->content = content;
   elm_widget_resize_object_set(obj, content, EINA_TRUE);
}

EOLIAN static Evas_Object *
_elm_items_item_content_get(Eo *obj EINA_UNUSED, Elm_Items_Item_Data *pd)
{
   return pd->content;
}

EOLIAN static void
_elm_items_item_realize(Eo *obj EINA_UNUSED, Elm_Items_Item_Data *pd)
{
   //debug
   srand((unsigned int) obj);
   int r = rand() % 255;
   int g = rand() % 255;
   int b = rand() % 255;
   evas_object_color_set(pd->rectangle, r, g, b, 255);
   //call realize event
   eo_do(obj, eo_event_callback_call(ELM_ITEMS_ITEM_EVENT_REALIZE, NULL));
}

EOLIAN static void
_elm_items_item_unrealize(Eo *obj EINA_UNUSED, Elm_Items_Item_Data *pd)
{
   eo_do(pd->rectangle, efl_gfx_color_set(10, 10, 10, 255));
   eo_do(obj, eo_event_callback_call(ELM_ITEMS_ITEM_EVENT_UNREALIZE, NULL));
}

EOLIAN static void
_elm_items_item_selected_set(Eo *obj, Elm_Items_Item_Data *pd, Eina_Bool selected)
{
   if (pd->selected == selected) return;

   pd->selected = selected;

   if (selected)
     {
        eo_do(pd->rectangle, efl_gfx_color_set(255, 0, 0, 255));
        eo_do(obj, eo_event_callback_call(ELM_ITEMS_ITEM_EVENT_SELECTED, NULL));
     }
   else
     {
        srand((unsigned int) obj);
        int r = rand() % 255;
        int g = rand() % 255;
        int b = rand() % 255;
        evas_object_color_set(pd->rectangle, r, g, b, 255);
        eo_do(obj, eo_event_callback_call(ELM_ITEMS_ITEM_EVENT_UNSELECTED, NULL));
     }
}

EOLIAN static Eina_Bool
_elm_items_item_selected_get(Eo *obj EINA_UNUSED, Elm_Items_Item_Data *pd)
{
   return pd->selected;
}

EOLIAN static Eina_Bool
_elm_items_item_elm_widget_event(Eo *obj, Elm_Items_Item_Data *pd, Evas_Object *source EINA_UNUSED, Evas_Callback_Type type, void *event_info)
{
   if (type == EVAS_CALLBACK_MOUSE_DOWN)
     {
        //TODO check if those are the correct buttons
        eo_do(obj, elm_items_item_selected_set(!pd->selected));
        return EINA_FALSE;
     }

   return EINA_FALSE;
}

EOLIAN static void
_elm_items_item_evas_object_smart_add(Eo *obj, Elm_Items_Item_Data *pd)
{
   eo_do_super(obj,ELM_ITEMS_ITEM_CLASS,evas_obj_smart_add());

   pd->rectangle = evas_object_rectangle_add(evas_object_evas_get(obj));
   evas_object_color_set(pd->rectangle, 10, 10, 10, 255);
   evas_object_size_hint_min_set(pd->rectangle, 20, 20);
   evas_object_show(pd->rectangle);

   elm_widget_resize_object_set(obj, pd->rectangle, EINA_TRUE);
}

#include "elm_items_item.eo.x"