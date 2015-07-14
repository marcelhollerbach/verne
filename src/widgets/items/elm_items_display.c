#include "elm_items_priv.h"

typedef struct {
    Efl_Tree_Base *root;  //< the root item of the item node
    Eina_List *realized; //< list of realized items
    Evas_Object *pane;  //< the pane of the implementor
    Eina_Strbuf *search;
    Ecore_Idler *recalc_idler;
    Eina_List *selected; //< list of selected items
} Elm_Items_Display_Data;

EOLIAN static Efl_Tree_Base *
_elm_items_display_tree_get(Eo *obj EINA_UNUSED, Elm_Items_Display_Data *pd)
{
   return pd->root;
}

EOLIAN static Eina_Bool
_elm_items_display_elm_widget_event(Eo *obj, Elm_Items_Display_Data *pd, Evas_Object *source EINA_UNUSED, Evas_Callback_Type type, void *event_info)
{
   Evas_Event_Key_Down *ev;
   if (type != EVAS_CALLBACK_KEY_DOWN) return EINA_TRUE;

   ev = event_info;

   if (ev->key[1] == '\0' && isalnum(*ev->string))
     {
        if (!pd->search)
          pd->search = eina_strbuf_new();

        eina_strbuf_append(pd->search, ev->string);
        eo_do(obj, elm_items_display_search(eina_strbuf_string_get(pd->search)));
        return EINA_TRUE;
     }
   else if (!strcmp(ev->key, "BackSpace"))
     {
        char *oldstr;
        int oldlength;
        //shrink search string

        if (!pd->search)
          return EO_CALLBACK_STOP;

        oldlength = eina_strbuf_length_get(pd->search);
        oldstr = eina_strbuf_string_steal(pd->search);

        if (oldlength == 0)
          return EO_CALLBACK_STOP;

        //reset the string
        eina_strbuf_reset(pd->search);
        //cut off the last bit
        oldstr[oldlength - 1] = '\0';

        eina_strbuf_append(pd->search, oldstr);
        free(oldstr);
        eo_do(obj, elm_items_display_search(eina_strbuf_string_get(pd->search)));
        return EINA_TRUE;
     }
   else if (!strcmp(ev->key, "Right"))
     {
        //sent right
        eo_do(obj, elm_items_display_sel_move(ELM_ITEMS_MOVE_DIR_EAST));
        return EINA_TRUE;
     }
   else if (!strcmp(ev->key, "Left"))
     {
        //sent let
        eo_do(obj, elm_items_display_sel_move(ELM_ITEMS_MOVE_DIR_WEST));
        return EINA_TRUE;
     }
   else if (!strcmp(ev->key, "Up"))
     {
        //sent up
        eo_do(obj, elm_items_display_sel_move(ELM_ITEMS_MOVE_DIR_NORTH));
        return EINA_TRUE;
     }
   else if (!strcmp(ev->key, "Down"))
     {
        //sent down
        eo_do(obj, elm_items_display_sel_move(ELM_ITEMS_MOVE_DIR_SOUTH));
        return EINA_TRUE;
     }
   else if (!strcmp(ev->key, "Return"))
     {
        //select
        eo_do(obj, elm_items_display_select());
        return EINA_TRUE;
     }
   else if (!strcmp(ev->key, "Home"))
     {
        //first item
        eo_do(obj, elm_interface_scrollable_page_bring_in(0, 0));
        return EINA_TRUE;
     }
   else if (!strcmp(ev->key, "End"))
     {
        //last item
        int h,v;
        eo_do(obj, elm_interface_scrollable_last_page_get(&h, &v);
                    elm_interface_scrollable_page_bring_in(h, v));
        return EINA_TRUE;
     }
   else if (!strcmp(ev->key, "Next"))
     {
        //next page
        int h,v;
        eo_do(obj, elm_interface_scrollable_current_page_get(&h, &v);
                    elm_interface_scrollable_page_bring_in(h, v+1));
        return EINA_TRUE;
     }
   else if (!strcmp(ev->key, "Prior"))
     {
        //prior page
        int h,v;
        eo_do(obj, elm_interface_scrollable_current_page_get(&h, &v);
                    elm_interface_scrollable_page_bring_in(h, v-1));
        return EINA_TRUE;
     }
   return EO_CALLBACK_CONTINUE;
}

EOLIAN static void
_elm_items_display_search(Eo *obj EINA_UNUSED, Elm_Items_Display_Data *pd, const char *search)
{
   Eina_List *node, *children;
   Efl_Tree_Base *tb;
   int min = -1;
   Elm_Items_Item *searched = NULL;

   eo_do(pd->root, children = efl_tree_base_children(EINA_TRUE));

   EINA_LIST_FOREACH(children, node, tb)
     {
        Elm_Items_Item *item;
        const char *searchable;
        char *f;
        eo_do(tb, item = efl_tree_base_carry_get());
        eo_do(item, searchable = elm_items_item_search_get());

        if (!searchable) continue;

        if ((f = strstr(searchable, search)))
          {
             int tmin = f - searchable;
             if (min == -1)
               min = tmin;

             if (tmin > min)
               continue;
             min = tmin;
             searched = item;
          }
     }
   eo_do(searched, elm_items_item_selected_set(EINA_TRUE));
}

static Eina_Bool
_selected(void *data, Eo *obj EINA_UNUSED, const Eo_Event_Description2 *desc EINA_UNUSED, void *event EINA_UNUSED)
{
   Elm_Items_Display_Data *pd;

   pd = eo_data_scope_get(data, ELM_ITEMS_DISPLAY_CLASS);
   pd->selected = eina_list_append(pd->selected, obj);
   return EO_CALLBACK_CONTINUE;
}

static Eina_Bool
_unselected(void *data, Eo *obj, const Eo_Event_Description2 *desc EINA_UNUSED, void *event EINA_UNUSED)
{
   Elm_Items_Display_Data *pd;

   pd = eo_data_scope_get(data, ELM_ITEMS_DISPLAY_CLASS);
   pd->selected = eina_list_remove(pd->selected, obj);
   return EO_CALLBACK_CONTINUE;
}

static Eina_Bool
_del(void *data, Eo *obj EINA_UNUSED, const Eo_Event_Description2 *desc EINA_UNUSED, void *event EINA_UNUSED)
{
   Elm_Items_Display_Data *pd;
   Eo *good;

   pd = eo_data_scope_get(data, ELM_ITEMS_DISPLAY_CLASS);

   eo_do(event, good = efl_tree_base_carry_get());
   eo_do(good, eo_event_callback_del(ELM_ITEMS_ITEM_EVENT_SELECTED, _selected, data);
               eo_event_callback_del(ELM_ITEMS_ITEM_EVENT_UNSELECTED, _unselected, data);
    );

   //remove from selected
   pd->selected = eina_list_remove(pd->selected, good);
   //remove from potential realized
   pd->realized = eina_list_remove(pd->realized, good);
   return EO_CALLBACK_CONTINUE;
}

static Eina_Bool
_add(void *data, Eo *obj EINA_UNUSED, const Eo_Event_Description2 *desc EINA_UNUSED, void *event EINA_UNUSED)
{
   Eo *good;

   //subscribe to events
   eo_do(event, good = efl_tree_base_carry_get());
   eo_do(good, eo_event_callback_add(ELM_ITEMS_ITEM_EVENT_SELECTED, _selected, data);
               eo_event_callback_add(ELM_ITEMS_ITEM_EVENT_UNSELECTED, _unselected, data);
    );
   //XXX: check if selected and add to list
   return EO_CALLBACK_CONTINUE;
}

EOLIAN static Eina_List *
_elm_items_display_selected_get(Eo *obj EINA_UNUSED, Elm_Items_Display_Data *pd)
{
   return pd->selected;
}

EOLIAN static Eo_Base *
_elm_items_display_eo_base_constructor(Eo *obj, Elm_Items_Display_Data *pd)
{
   Eo *eo;

   pd->root = eo_add(EFL_TREE_BASE_CLASS, NULL);

   eo_do(pd->root,
    eo_event_callback_add(EFL_TREE_BASE_EVENT_CHILDREN_DEL_RECURSIVE, _del, obj);
    eo_event_callback_add(EFL_TREE_BASE_EVENT_CHILDREN_DEL_DIRECT, _del, obj);
    eo_event_callback_add(EFL_TREE_BASE_EVENT_CHILDREN_ADD_RECURSIVE, _add, obj);
    eo_event_callback_add(EFL_TREE_BASE_EVENT_CHILDREN_ADD_DIRECT, _add, obj);
    );

   eo_do_super(obj, ELM_ITEMS_DISPLAY_CLASS, eo = eo_constructor());
   return eo;
}

EOLIAN static void
_elm_items_display_eo_base_destructor(Eo *obj, Elm_Items_Display_Data *pd)
{
   Elm_Items_Item *item;

   EINA_LIST_FREE(pd->realized, item)
     {
        eo_do(item, eo_event_callback_del(EO_BASE_EVENT_DEL, _del, obj));
     }

   eo_del(pd->root);

   eo_do_super(obj, ELM_ITEMS_DISPLAY_CLASS, eo_destructor());
}

EOLIAN static void
_elm_items_display_realizes_set(Eo *obj EINA_UNUSED, Elm_Items_Display_Data *pd, Eina_List *realized)
{
   Eina_List *node;
   Eina_List *realizes = NULL;

   Elm_Items_Item *item;
   //get the diff between the two sets of items
   EINA_LIST_FOREACH(realized, node, item)
     {
        if (eina_list_data_find_list(pd->realized, item))
          {
             //this item is allready realized
             pd->realized = eina_list_remove(pd->realized, item);
          }
        else
          {
             //this items needs to be realized
             realizes = eina_list_append(realizes, item);
          }
     }

   //the items which are
   EINA_LIST_FREE(pd->realized, item)
     {
        eo_do(item, elm_items_item_unrealize());
        evas_object_hide(item);
     }

   EINA_LIST_FREE(realizes, item)
     {
        eo_do(item, elm_items_item_realize());
        evas_object_show(item);
     }

   pd->realized = realized;
}

EOLIAN static Eina_List *
_elm_items_display_realizes_get(Eo *obj EINA_UNUSED, Elm_Items_Display_Data *pd)
{
   return pd->realized;
}

EOLIAN static Eina_List *
_elm_items_display_item_search_xywh(Eo *obj EINA_UNUSED, Elm_Items_Display_Data *pd, int xx, int yy, int ww, int hh)
{
   Eina_List *children, *node;
   Efl_Tree_Base *treeitem;
   Elm_Items_Item *item;
   Eina_List *items = NULL;
   Eina_Rectangle viewport;

   EINA_RECTANGLE_SET(&viewport, xx, yy, ww, hh);

   //get all the childs
   eo_do(pd->root, children = efl_tree_base_children(EINA_TRUE));

   //iterate throuw all the childs
   EINA_LIST_FOREACH(children, node, treeitem)
     {
        int x,y,w,h;
        Eina_Rectangle itemrect;

        eo_do(treeitem, item = efl_tree_base_carry_get());

        //get the geometry of a new item
        evas_object_geometry_get(item, &x, &y, &w, &h);
        EINA_RECTANGLE_SET(&itemrect, x, y, w, h);

        //check if the item is in the searched rectangle
        if (eina_rectangles_intersect(&viewport, &itemrect))
          {
             //printf("%d-%d-%d-%d %d-%d-%d-%d\n", vpx,vpy,vpw,vph,x,y,w,h);
             items = eina_list_append(items, item);
          }
     }
   return items;
}

#include "elm_items_display.eo.x"