#include "elm_items_priv.h"

//XXX: For now everything works just for strait items, no children or something will be displayed

typedef struct {
   Eo *root;
   Eina_Bool unselect_barrier;
   Evas_Object *pan;
   Eina_List *realitems;
   Ecore_Idler *flush_idle;
} Elm_Items_List_Data;

typedef struct {
   Evas_Box *box;
   Eo *obj;
   int w,h;
   int px,py;
   int y;
   int old_first;
   int old_last;
   Eina_List *realitems;
   Eina_List *realized;
} Elm_Items_List_Pan_Data;

#define HACKY_HEIGHT 30

EOLIAN static void
_elm_items_list_elm_items_display_select(Eo *obj, Elm_Items_List_Data *pd EINA_UNUSED)
{
   Eina_List *selection;

   eo_do(obj, selection = elm_items_display_selected_get());

   if (eina_list_count(selection) != 1)
     return;
   //emulate double click on this item XXX find a proper way
   eo_do(eina_list_data_get(selection), eo_event_callback_call(ELM_ITEMS_ITEM_EVENT_CLICKED_DOUBLE, NULL));
}

EOLIAN static void
_elm_items_list_elm_items_display_sel_move(Eo *obj, Elm_Items_List_Data *pd, Elm_Items_Move_Dir direction)
{

}

EOLIAN static void
_elm_items_list_evas_object_smart_add(Eo *obj, Elm_Items_List_Data *pd EINA_UNUSED)
{
   eo_do_super(obj, ELM_ITEMS_LIST_CLASS, evas_obj_smart_add());
}

static Eina_Bool
_selected(void *data, Eo *obj, const Eo_Event_Description *desc EINA_UNUSED, void *event EINA_UNUSED)
{
   Elm_Items_List_Data *pd;
   Elm_Items_Item *sel;
   const Evas_Modifier *mods;
   Eina_List *selection;

   pd = eo_data_scope_get(data, ELM_ITEMS_LIST_CLASS);
   eo_do(data, selection = elm_items_display_selected_get());

   mods = evas_key_modifier_get(evas_object_evas_get(obj));

   if (!evas_key_modifier_is_set(mods, "Control"))
     {
        Eina_List *selected;

        selected = eina_list_clone(selection);
        //stop unselecting work
        pd->unselect_barrier = EINA_TRUE;
        EINA_LIST_FREE(selected, sel)
          {
             if (sel == obj) continue;
             eo_do(sel, elm_items_item_selected_set(EINA_FALSE));
          }
        //restart unselecting work
        pd->unselect_barrier = EINA_FALSE;
     }
   return EO_CALLBACK_CONTINUE;
}

static Eina_Bool
_unselected(void *data, Eo *obj EINA_UNUSED, const Eo_Event_Description *desc EINA_UNUSED, void *event EINA_UNUSED)
{
   Elm_Items_List_Data *pd;
   const Evas_Modifier *mods;
   Eina_List *selection;

   pd = eo_data_scope_get(data, ELM_ITEMS_LIST_CLASS);
   eo_do(data, selection = elm_items_display_selected_get());
   if (pd->unselect_barrier) return EO_CALLBACK_CONTINUE;

   mods = evas_key_modifier_get(evas_object_evas_get(data));

   if (!evas_key_modifier_is_set(mods, "Control"))
     {
        if (eina_list_count(selection) > 1)
          eo_do(obj, elm_items_item_selected_set(EINA_TRUE));

     }
   return EO_CALLBACK_CONTINUE;
}

static Eina_Bool
_idle_tree_flush(void *data)
{
   Elm_Items_List_Data *pd;

   pd = eo_data_scope_get(data, ELM_ITEMS_LIST_CLASS);
   eo_do(pd->pan, elm_items_list_pan_realitems(pd->realitems));

   pd->flush_idle = NULL;

   return EINA_FALSE;
}

static Eina_Bool
_add(void *data, Eo *obj EINA_UNUSED, const Eo_Event_Description *desc EINA_UNUSED, void *event)
{
   Eo *good;
   Eo *prev;
   Eo *next;
   Elm_Items_List_Data *pd;

   pd = eo_data_scope_get(data, ELM_ITEMS_LIST_CLASS);

   eo_do(event, good = efl_tree_base_carry_get();
                prev = efl_tree_base_prev_get();
                next = efl_tree_base_next_get();
        );
   //subscribe to item events
   eo_do(good, eo_event_callback_add(ELM_ITEMS_ITEM_EVENT_SELECTED, _selected, data);
               eo_event_callback_add(ELM_ITEMS_ITEM_EVENT_UNSELECTED, _unselected, data);
        );
   //set hints
   evas_object_size_hint_min_set(good, 1, HACKY_HEIGHT);
   evas_object_size_hint_align_set(good, EVAS_HINT_FILL, 1.0);
   evas_object_size_hint_weight_set(good, EVAS_HINT_EXPAND, 0.0);
   //show the item
   evas_object_show(good);

   pd->realitems = eina_list_append(pd->realitems, good);

   if (!pd->flush_idle)
     pd->flush_idle = ecore_idler_add(_idle_tree_flush, data);

   return EO_CALLBACK_CONTINUE;
}

static Eina_Bool
_del(void *data, Eo *obj EINA_UNUSED, const Eo_Event_Description *desc EINA_UNUSED, void *event)
{
   Eo *good;
   Elm_Items_List_Data *pd;

   pd = eo_data_scope_get(data, ELM_ITEMS_LIST_CLASS);

   eo_do(event, good = efl_tree_base_carry_get());

   pd->realitems = eina_list_remove(pd->realitems, good);

   if (!pd->flush_idle)
     pd->flush_idle = ecore_idler_add(_idle_tree_flush, data);

   return EO_CALLBACK_CONTINUE;
}

EOLIAN static Eo_Base *
_elm_items_list_eo_base_constructor(Eo *obj, Elm_Items_List_Data *pd)
{
   Eo *eo;

   eo_do_super(obj, ELM_ITEMS_LIST_CLASS, eo = eo_constructor());

   if (!eo)
     return eo;

   //get the root
   eo_do(obj, pd->root = elm_items_display_tree_get());

   //monitor changes on the tree
   eo_do(pd->root,
            eo_event_callback_add(EFL_TREE_BASE_EVENT_CHILDREN_ADD_DIRECT, _add, obj);
            eo_event_callback_add(EFL_TREE_BASE_EVENT_CHILDREN_DEL_DIRECT, _del, obj);
        );

   pd->pan = eo_add(ELM_ITEMS_LIST_PAN_CLASS, obj);
   {
      Elm_Items_List_Pan_Data *pdpan = eo_data_scope_get(pd->pan, ELM_ITEMS_LIST_PAN_CLASS);
      pdpan->obj = obj;
      pdpan->box = evas_object_box_add(evas_object_evas_get(obj));
      evas_object_box_padding_set(pdpan->box, 0, 0);
      evas_object_box_align_set(pdpan->box, 0.0, 0.0);
      evas_object_box_layout_set(pdpan->box, evas_object_box_layout_vertical, pdpan->box, NULL);
      evas_object_show(pdpan->box);
   }

   eo_do(obj, elm_interface_scrollable_extern_pan_set(pd->pan));
   return eo;
}

//===== pan element =====

static void
_pan_update(Eo *obj, Elm_Items_List_Pan_Data *pd)
{
   int first_item, last_item;
   int gap;
   Eina_Bool realize_dir;
   Eina_List *realizes = NULL;

   gap = pd->y % (HACKY_HEIGHT);
   first_item = pd->y / HACKY_HEIGHT;
   last_item = ((pd->y + pd->h) / HACKY_HEIGHT);

   //remove all children from the box
   evas_object_box_remove_all(pd->box, EINA_FALSE);

   //move the box to the new position
   evas_object_move(pd->box, pd->px, pd->py - (gap));
   evas_object_resize(pd->box, pd->w, (last_item - first_item) * HACKY_HEIGHT);

   //add the new items
   for (int i = first_item; i <= last_item; i++)
     {
        Eo *item;

        item = eina_list_nth(pd->realitems, i);
        evas_object_box_append(pd->box, item);
        realizes = eina_list_append(realizes, item);
     }

   eo_do(pd->obj, elm_items_display_realizes_set(realizes));
}

EOLIAN static void
_elm_items_list_pan_realitems(Eo *obj, Elm_Items_List_Pan_Data *pd, Eina_List *list)
{
   //unrealize all items
   {
      Eo *realized;
      //make the box empty
      evas_object_box_remove_all(pd->box, EINA_FALSE);

      //free all realized items
      EINA_LIST_FREE(pd->realized, realized)
        {
           eo_do(realized, elm_items_item_unrealize());
           evas_object_hide(realized);
        }

      pd->old_first = -1;
      pd->old_last = -1;
   }
   //set new list
   pd->realized = NULL;
   pd->realitems = list;
   //update pan
   _pan_update(obj, pd);
   eo_do(obj, eo_event_callback_call(ELM_PAN_EVENT_CHANGED, NULL));
}

EOLIAN static void
_elm_items_list_pan_elm_pan_gravity_set(Eo *obj, Elm_Items_List_Pan_Data *pd, double x, double y)
{

}

EOLIAN static void
_elm_items_list_pan_elm_pan_gravity_get(Eo *obj, Elm_Items_List_Pan_Data *pd, double *x, double *y)
{
   if (x)
     *x = 0;
   if (y)
     *y = 0;
}

EOLIAN static void
_elm_items_list_pan_elm_pan_pos_set(Eo *obj, Elm_Items_List_Pan_Data *pd, Evas_Coord x, Evas_Coord y)
{
   pd->y = y;

   _pan_update(obj, pd);
}

EOLIAN static void
_elm_items_list_pan_elm_pan_pos_get(Eo *obj EINA_UNUSED, Elm_Items_List_Pan_Data *pd, Evas_Coord *x, Evas_Coord *y)
{
   if (x)
     *x = 0;
   if (y)
     *y = pd->y;
}

EOLIAN static void
_elm_items_list_pan_elm_pan_content_size_get(Eo *obj EINA_UNUSED, Elm_Items_List_Pan_Data *pd, Evas_Coord *w, Evas_Coord *h)
{
   if (w)
     *w = 10;
   if (h)
     *h = eina_list_count(pd->realitems) * HACKY_HEIGHT;
}

EOLIAN static void
_elm_items_list_pan_elm_pan_pos_min_get(Eo *obj EINA_UNUSED, Elm_Items_List_Pan_Data *pd EINA_UNUSED, Evas_Coord *x, Evas_Coord *y)
{
   if (x)
     *x = 0;
   if (y)
     *y = 0;
}

EOLIAN static void
_elm_items_list_pan_elm_pan_pos_max_get(Eo *obj EINA_UNUSED, Elm_Items_List_Pan_Data *pd, Evas_Coord *x, Evas_Coord *y)
{
   if (x)
     *x = 0;
   if (y)
     *y = eina_list_count(pd->realitems) * HACKY_HEIGHT - pd->h;
}

EOLIAN static void
_elm_items_list_pan_evas_object_smart_resize(Eo *obj EINA_UNUSED, Elm_Items_List_Pan_Data *pd, Evas_Coord w, Evas_Coord h)
{
   pd->w = w;
   pd->h = h;
   _pan_update(obj, pd);
   eo_do_super(obj, ELM_ITEMS_LIST_PAN_CLASS, evas_obj_smart_resize(w, h));
}

EOLIAN static void
_elm_items_list_pan_evas_object_smart_move(Eo *obj, Elm_Items_List_Pan_Data *pd, Evas_Coord x, Evas_Coord y)
{
   pd->px = x;
   pd->py = y;
   _pan_update(obj, pd);
   eo_do_super(obj, ELM_ITEMS_LIST_PAN_CLASS, evas_obj_smart_move(x, y));
}

#include "elm_items_list.eo.x"
#include "elm_items_list_pan.eo.x"