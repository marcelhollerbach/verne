#include "elm_items_priv.h"

typedef struct {
    Evas_Object *edje;
    Evas_Object *content;
    Eina_Bool selected;
    Efl_Tree_Base *item;
    Ecore_Timer *double_timer;
    Ecore_Idler *realize_idle;
    const char * searchable;
} Elm_Items_Item_Data;

EOLIAN static void
_elm_items_item_content_set(Eo *obj EINA_UNUSED, Elm_Items_Item_Data *pd, Evas_Object *content)
{
   pd->content = content;
   elm_object_part_content_unset(pd->edje, "content");
   elm_object_part_content_set(pd->edje, "content", pd->content);
}

EOLIAN static Evas_Object *
_elm_items_item_content_get(Eo *obj EINA_UNUSED, Elm_Items_Item_Data *pd)
{
   return pd->content;
}

EOLIAN static void
_elm_items_item_realize(Eo *obj EINA_UNUSED, Elm_Items_Item_Data *pd EINA_UNUSED)
{
   //call realize event
  if (!pd->content)
    eo_do(obj, eo_event_callback_call(ELM_ITEMS_ITEM_EVENT_REALIZE, NULL));
}

EOLIAN static void
_elm_items_item_unrealize(Eo *obj EINA_UNUSED, Elm_Items_Item_Data *pd EINA_UNUSED)
{
   if (!pd->realize_idle)
     eo_do(obj, eo_event_callback_call(ELM_ITEMS_ITEM_EVENT_UNREALIZE, NULL));
   else
     ecore_idler_del(pd->realize_idle);
}

EOLIAN static void
_elm_items_item_selected_set(Eo *obj, Elm_Items_Item_Data *pd, Eina_Bool selected)
{
   if (pd->selected == selected) return;

   pd->selected = selected;

   if (selected)
     {
        elm_layout_signal_emit(pd->edje, "elm,state,selected,on", "elm");
        eo_do(obj, eo_event_callback_call(ELM_ITEMS_ITEM_EVENT_SELECTED, NULL));
     }
   else
     {
        elm_layout_signal_emit(pd->edje, "elm,state,selected,off", "elm");
        eo_do(obj, eo_event_callback_call(ELM_ITEMS_ITEM_EVENT_UNSELECTED, NULL));
     }
}

EOLIAN static Eina_Bool
_elm_items_item_selected_get(Eo *obj EINA_UNUSED, Elm_Items_Item_Data *pd)
{
   return pd->selected;
}

static Eina_Bool
_timer_cb(void *data)
{
   Elm_Items_Item_Data *pd;

   pd = eo_data_scope_get(data, ELM_ITEMS_ITEM_CLASS);

   if (!pd)
     {
        ERR("timer is not corrected correct /o\\");
        return EINA_FALSE;
     }

   pd->double_timer = NULL;

   return EINA_FALSE;
}

static void
_mouse_down(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Evas_Event_Mouse_Up *ev;
   Eina_Bool selected;
   Elm_Items_Item_Data *pd;
   Eo *sobj;

   pd = eo_data_scope_get(obj, ELM_ITEMS_ITEM_CLASS);
   ev = event_info;

   if (ev->button != 1) return;

   //get a self reference, if one of the callbacks leads to a deletion of this object, we can detect it and return
   eo_do(obj, eo_wref_add(&sobj));

   eo_do(sobj, selected = elm_items_item_selected_get());

   if (selected && pd->double_timer)
     {
        //this is a double click! nasty!
        ecore_timer_del(pd->double_timer);
        pd->double_timer = NULL;
        eo_do(sobj, eo_event_callback_call(ELM_ITEMS_ITEM_EVENT_CLICKED_DOUBLE, NULL));
        return;
     }

   eo_do(sobj, eo_event_callback_call(ELM_ITEMS_ITEM_EVENT_CLICKED, NULL));
   if (!sobj)
     return;

   eo_do(sobj, elm_items_item_selected_set(!selected));
   if (!sobj)
     return;

   if (!selected)
     pd->double_timer = ecore_timer_add(1.0, _timer_cb, obj);

}

EOLIAN static void
_elm_items_item_evas_object_smart_add(Eo *obj, Elm_Items_Item_Data *pd)
{
   eo_do_super(obj,ELM_ITEMS_ITEM_CLASS,evas_obj_smart_add());

   evas_object_event_callback_add(obj, EVAS_CALLBACK_MOUSE_UP, _mouse_down, NULL);

   pd->edje = elm_layout_add(obj);

   if (!elm_layout_theme_set(pd->edje, "icons", "item", "default"))
     {
        ERR("Failed to set theme");
     }

   elm_widget_resize_object_set(obj, pd->edje, EINA_TRUE);
}

EOLIAN static Efl_Tree_Base *
_elm_items_item_item_get(Eo *obj, Elm_Items_Item_Data *pd)
{
   if (pd->item)
     return pd->item;

   pd->item = eo_add(EFL_TREE_BASE_CLASS, obj);
   eo_do(pd->item, efl_tree_base_carry_set(obj));
   return pd->item;
}

EOLIAN static void
_elm_items_item_eo_base_destructor(Eo *obj, Elm_Items_Item_Data *pd)
{
   if (pd->item)
     eo_del(pd->item);
   ecore_timer_del(pd->double_timer);
   pd->double_timer = NULL;
   eo_do_super(obj, ELM_ITEMS_ITEM_CLASS, eo_destructor());
}

EOLIAN static void
_elm_items_item_search_set(Eo *obj EINA_UNUSED, Elm_Items_Item_Data *pd, const char *string)
{
   pd->searchable = string;
}

EOLIAN static const char *
_elm_items_item_search_get(Eo *obj EINA_UNUSED, Elm_Items_Item_Data *pd)
{
   return pd->searchable;
}


#include "elm_items_item.eo.x"