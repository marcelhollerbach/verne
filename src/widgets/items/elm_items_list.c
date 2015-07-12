#include "elm_items_priv.h"

//XXX: For now everything works just for strait items, no children or something will be displayed

typedef struct {
   //Eina_Hash *boxes; //< hash map of all the boxes which are appended to
   Eo *root; //< lazy root node -of the display
   Evas_Object *box; //< tmp box should be solved with hash tables
   Eina_List *selected; //< a list of currently selected items
   Eina_Bool unselect_barrier;
} Elm_Items_List_Data;

EOLIAN static Evas_Object *
_elm_items_list_elm_items_display_child_pane_get(Eo *obj EINA_UNUSED, Elm_Items_List_Data *pd)
{
   if (!pd->box)
     {
        //create root box
        pd->box = elm_box_add(obj);
     }

   return pd->box;
}

EOLIAN static void
_elm_items_list_elm_items_display_sel_move(Eo *obj EINA_UNUSED, Elm_Items_List_Data *pd, Elm_Items_Move_Dir direction)
{

   if (eina_list_count(pd->selected) == 1)
     {
        //just select the next
        Eo *item;
        Eo *sel;

        eo_do(eina_list_data_get(pd->selected), item = elm_items_item_get());
        if (direction == ELM_ITEMS_MOVE_DIR_SOUTH || direction == ELM_ITEMS_MOVE_DIR_EAST)
          eo_do(item, sel = efl_tree_base_next_get());
        else
          eo_do(item, sel = efl_tree_base_prev_get());
        if (sel)
          {
             Eo *good;

             eo_do(eina_list_data_get(pd->selected), elm_items_item_selected_set(EINA_FALSE));
             eo_do(sel, good = efl_tree_base_carry_get());
             eo_do(good, elm_items_item_selected_set(EINA_TRUE));
          }
     }
   else if (pd->selected == NULL)
     {
        Eo *first;
        Eo *item;

        eo_do(pd->root, first = eina_list_data_get(efl_tree_base_children(EINA_FALSE)));
        eo_do(first, item = efl_tree_base_carry_get());
        eo_do(item, elm_items_item_selected_set(EINA_TRUE));
     }
   else
     {
        //XXX:
        if (direction == ELM_ITEMS_MOVE_DIR_SOUTH || direction == ELM_ITEMS_MOVE_DIR_EAST)
          {

          }
        else
          {

          }
     }
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

   pd = eo_data_scope_get(data, ELM_ITEMS_LIST_CLASS);

   mods = evas_key_modifier_get(evas_object_evas_get(data));
   if (!evas_key_modifier_is_set(mods, "Control"))
     {
        Eina_List *selected;

        selected = eina_list_clone(pd->selected);

        EINA_LIST_FREE(selected, sel)
          {
             eo_do(sel, elm_items_item_selected_set(EINA_FALSE));
          }
        pd->selected = NULL;
     }
   pd->selected = eina_list_append(pd->selected, obj);
   return EO_CALLBACK_CONTINUE;
}

static Eina_Bool
_unselected(void *data, Eo *obj EINA_UNUSED, const Eo_Event_Description *desc EINA_UNUSED, void *event EINA_UNUSED)
{
   Elm_Items_List_Data *pd;
   const Evas_Modifier *mods;

   pd = eo_data_scope_get(data, ELM_ITEMS_LIST_CLASS);
   if (pd->unselect_barrier) return EO_CALLBACK_CONTINUE;

   mods = evas_key_modifier_get(evas_object_evas_get(data));
   if (!evas_key_modifier_is_set(mods, "Control"))
     {
        pd->unselect_barrier = EINA_TRUE;
        Eina_List *selected;
        Elm_Items_Item *sel;

        selected = eina_list_clone(pd->selected);

        EINA_LIST_FREE(selected, sel)
          {
             eo_do(sel, elm_items_item_selected_set(EINA_FALSE));
          }
        pd->selected = NULL;
        pd->unselect_barrier = EINA_FALSE;
     }
   else
     pd->selected = eina_list_remove(pd->selected, obj);

   return EO_CALLBACK_CONTINUE;
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
   evas_object_size_hint_min_set(good, 1, 30);
   evas_object_size_hint_align_set(good, EVAS_HINT_FILL, 1.0);
   evas_object_size_hint_weight_set(good, EVAS_HINT_EXPAND, 0.0);
   //show the item
   evas_object_show(good);
   //check if we can prepend / append or need to pack and the end
   if (prev)
     {
        Eo *prevgood;

        eo_do(prev, prevgood = efl_tree_base_carry_get());
        elm_box_pack_after(pd->box, good, prevgood);
     }
   else if (next)
     {
        Eo *nextgood;

        eo_do(next, nextgood = efl_tree_base_carry_get());
        elm_box_pack_before(pd->box, good, nextgood);
     }
   else
     elm_box_pack_end(pd->box, good);
   elm_box_recalculate(pd->box);

   return EO_CALLBACK_CONTINUE;
}

static Eina_Bool
_del(void *data, Eo *obj EINA_UNUSED, const Eo_Event_Description *desc EINA_UNUSED, void *event)
{
   Eo *good;
   Elm_Items_List_Data *pd;

   pd = eo_data_scope_get(data, ELM_ITEMS_LIST_CLASS);

   eo_do(event, good = efl_tree_base_carry_get());

   elm_box_unpack(pd->box, good);
   pd->selected = eina_list_remove(pd->selected, good);
   return EO_CALLBACK_CONTINUE;
}

EOLIAN static Eo_Base *
_elm_items_list_eo_base_constructor(Eo *obj, Elm_Items_List_Data *pd)
{
   Eo *eo;

   eo_do_super_ret(obj, ELM_ITEMS_LIST_CLASS, eo, eo_constructor());
   if (!eo)
     return eo;

   //get the root
   eo_do(obj, pd->root = elm_items_display_tree_get());

   //monitor changes on the tree
   eo_do(pd->root,
            eo_event_callback_add(EFL_TREE_BASE_EVENT_CHILDREN_ADD_DIRECT, _add, obj);
            eo_event_callback_add(EFL_TREE_BASE_EVENT_CHILDREN_DEL_DIRECT, _del, obj);
        );

   return eo;
}

EOLIAN static void
_elm_items_list_eo_base_destructor(Eo *obj, Elm_Items_List_Data *pd EINA_UNUSED)
{
   eo_do_super(obj, ELM_ITEMS_LIST_CLASS, eo_destructor());
}

#include "elm_items_list.eo.x"