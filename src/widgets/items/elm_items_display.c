#include "elm_items_priv.h"

typedef struct {
    Efl_Tree_Base *root;  //< the root item of the item node
    Eina_List *realized; //< list of realized items
    Evas_Object *pane;  //< the pane of the implementor
    Eina_Strbuf *search;
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
        //TODO do something usefull
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
        //TODO do something usefull
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

static Eina_Bool
_del(void *data, Eo *obj, const Eo_Event_Description2 *desc EINA_UNUSED, void *event EINA_UNUSED)
{
   Elm_Items_Display_Data *pd;

   pd = eo_data_scope_get(data, ELM_ITEMS_DISPLAY_CLASS);

   pd->realized = eina_list_remove(pd->realized, obj);

   return EINA_TRUE;
}

static void
_viewport_recheck(Evas_Object *obj, Elm_Items_Display_Data *pd)
{
   int vpx,vpy,vpw,vph;
   Eina_Rectangle viewport;
   Eina_List *children, *node, *realizes = NULL;
   Eina_List *realized = NULL;
   Elm_Items_Item *item;
   Efl_Tree_Base *tree_item;

   //get the geometry of the viewport
   eo_do(obj, elm_interface_scrollable_content_viewport_geometry_get(&vpx, &vpy, &vpw, &vph));
   EINA_RECTANGLE_SET(&viewport, vpx, vpy, vpw, vph);

   //get all the childs
   eo_do(pd->root, children = efl_tree_base_children(EINA_TRUE));
   //iterate throuw all the childs
   EINA_LIST_FOREACH(children, node, tree_item)
     {
        int x,y,w,h;
        Eina_Rectangle itemrect;

        eo_do(tree_item, item = efl_tree_base_carry_get());
        //get the geometry of a new item
        evas_object_geometry_get(item, &x, &y, &w, &h);
        EINA_RECTANGLE_SET(&itemrect, x, y, w, h);

        //check if the item is in the viewport
        if (eina_rectangles_intersect(&viewport, &itemrect))
          {
             //printf("%d-%d-%d-%d %d-%d-%d-%d\n", vpx,vpy,vpw,vph,x,y,w,h);

             if (eina_list_data_find(pd->realized, item))
               {
                  /*
                   * No need to subscribe to deletion for the item
                   * it allready is
                   */
                  //if the item is allready realized add it
                  realized = eina_list_append(realized, item);
                  //and remove it from the realized list
                  pd->realized = eina_list_remove(pd->realized, item);
               }
             else
               {
                  //if the item is not realized add it to the realizes
                  realizes = eina_list_append(realizes, item);
               }
          }
     }

   //items in realized list needs to be unrealized, free them
   EINA_LIST_FREE(pd->realized, item)
     {
        //unrealize the listed items
        eo_do(item, elm_items_item_unrealize());
        //not interested in deletion anymore
        eo_do(item, eo_event_callback_del(EO_BASE_EVENT_DEL, _del, obj));
     }

   //set the new list to the "still" realized items
   pd->realized = realized;

   //Iterate throuw all the realizes
   EINA_LIST_FREE(realizes, item)
     {
        //realize them
        eo_do(item, elm_items_item_realize());
        //subscribe to possible deletion
        eo_do(item, eo_event_callback_add(EO_BASE_EVENT_DEL, _del, obj));
        //append them to the new realized items
        pd->realized = eina_list_append(pd->realized, item);
     }

}

EOLIAN static Eo_Base *
_elm_items_display_eo_base_constructor(Eo *obj, Elm_Items_Display_Data *pd)
{
   Eo *eo;

   pd->root = eo_add(EFL_TREE_BASE_CLASS, NULL);

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

static void
_scroll_cb2(void *data EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   //recheck the realized items
   _viewport_recheck(obj, data);
}

EOLIAN static void
_elm_items_display_evas_object_smart_add(Eo *obj, Elm_Items_Display_Data *pd)
{
   eo_do_super(obj, ELM_ITEMS_DISPLAY_CLASS, evas_obj_smart_add());
   //take pane of the implementor
   eo_do(obj, pd->pane = elm_items_display_child_pane_get());
   //subscribe to scroll events
   evas_object_smart_callback_add(obj, "scroll", _scroll_cb2, pd);
   //give the pane to the scroller
   elm_object_content_set(obj, pd->pane);
   evas_object_size_hint_align_set(pd->pane, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(pd->pane, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   //show the scroller panel
   evas_object_show(pd->pane);
}

EOLIAN static void
_elm_items_display_evas_object_smart_resize(Eo *obj, Elm_Items_Display_Data *pd, Evas_Coord w, Evas_Coord h)
{
   eo_do_super(obj, ELM_ITEMS_DISPLAY_CLASS, evas_obj_smart_resize(w,h));
   //recheck the realized items
   _viewport_recheck(obj, pd);
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