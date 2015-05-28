#define EFL_BETA_API_SUPPORT
#define EFL_EO_API_SUPPORT
#include "elementary_ext_priv.h"


typedef struct
{
   Evas_Object *active;
   Evas_Object *content;
   Evas_Object *item;
   Evas_Object *widget;
} Item;

typedef struct
{
   Eina_List *items;
   Evas_Object *box;
   Evas_Object *scroller;
   Evas_Object *add;
   Item *focused;
} Elm_Tab_Pane_Data;

EOLIAN static Eo*
_elm_tab_pane_eo_base_constructor(Eo *obj, Elm_Tab_Pane_Data *pd EINA_UNUSED)
{

   eo_do_super(obj, ELM_TAB_PANE_CLASS, return eo_constructor());
}

EOLIAN static void
_elm_tab_pane_eo_base_destructor(Eo *obj, Elm_Tab_Pane_Data *pd EINA_UNUSED)
{
   eo_do_super(obj, ELM_TAB_PANE_CLASS, eo_destructor());

}

static void
_left_scroll_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, const char *s EINA_UNUSED, const char *e EINA_UNUSED)
{
  ERR("LEFT\n");
}

static void
_right_scroll_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, const char *s EINA_UNUSED, const char *e EINA_UNUSED)
{
  ERR("RIGHT\n");
}

static void
_add_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, const char *s EINA_UNUSED, const char *e EINA_UNUSED)
{
   eo_do(data,
         eo_event_callback_call(&_ELM_TAB_PANE_EVENT_ITEM_ADD,
                                NULL));
}

EOLIAN static void
_elm_tab_pane_evas_object_smart_add(Eo *obj, Elm_Tab_Pane_Data *pd)
{
   Evas_Object *sc, *o;
   eo_do_super(obj, ELM_TAB_PANE_CLASS, evas_obj_smart_add());

   if (!elm_layout_theme_set(obj, "tab_pane", "base", "default"))
     {
        CRI("Failed to set theme file\n");
        return;
     }

   elm_layout_signal_callback_add(obj, "scroll_left", "w",
                                  _left_scroll_cb, NULL);
   elm_layout_signal_callback_add(obj, "scroll_right", "w",
                                  _right_scroll_cb, NULL);

   pd->box = elm_box_add(obj);
   elm_box_align_set(pd->box, 0, 1.0);
   evas_object_size_hint_align_set(pd->box, 1.0, 1.0);
   elm_box_horizontal_set(pd->box, EINA_TRUE);
   evas_object_show(pd->box);

   //adding add icon
   pd->add = o = elm_layout_add(obj);
   if (!elm_layout_theme_set(o, "tab_pane", "add_item", "default"))
     {
        CRI("Failed to set theme file for the icon\n");
        return;
     }
   evas_object_size_hint_align_set(o, 0.0, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(o, 0.0, EVAS_HINT_EXPAND);
   elm_layout_signal_callback_add(o, "add", "w",
                                  _add_cb, obj);
   elm_box_pack_end(pd->box, o);
   evas_object_show(o);

   //adding scroller for the top box
   pd->scroller = sc = elm_scroller_add(obj);
   elm_scroller_policy_set(sc, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
   elm_scroller_movement_block_set(sc, ELM_SCROLLER_MOVEMENT_BLOCK_HORIZONTAL);
   elm_object_content_set(sc, pd->box);

   elm_object_part_content_set(obj, "tabs", sc);
}

static void
_highlight_item(Item *it)
{
   elm_layout_signal_emit(it->item, "active", "elm");
}

static void
_unhighlight_item(Item *it)
{
   elm_layout_signal_emit(it->item, "disable", "elm");
}

static void
_unfocus_item(Item *it)
{
   Evas_Object *obj, *t;

   obj = it->widget;
   _unhighlight_item(it);

   t = elm_object_part_content_unset(obj, "content");
   evas_object_hide(t);
}

static void
_focus_item(Item *it)
{
   Evas_Object *obj;

   obj = it->widget;
   _highlight_item(it);

   elm_object_part_content_set(obj, "content", it->content);
   evas_object_show(it->content);
}

static void
_item_clicked(void *data, Evas_Object *obj EINA_UNUSED, const char *s EINA_UNUSED, const char *e EINA_UNUSED)
{
  Item *it = data;
  Elm_Tab_Pane_Data *pd = eo_data_scope_get(it->widget, ELM_TAB_PANE_CLASS);

  if (pd->focused == it)
    return;

  _unfocus_item(pd->focused);
  _focus_item(it);

  pd->focused = it;
}

static void
_close_cb(void *data, Evas_Object *obj EINA_UNUSED, const char *s EINA_UNUSED, const char *e EINA_UNUSED)
{
   Item *it = data;
   Elm_Tab_Pane_Data *pd = eo_data_scope_get(it->widget, ELM_TAB_PANE_CLASS);


   eo_do(obj,
         eo_event_callback_call(&_ELM_TAB_PANE_EVENT_ITEM_DEL,
                                NULL));
   //FIXME this is just a hack

   elm_box_unpack(pd->box, it->item);

   evas_object_del(it->item);
   evas_object_del(it->active);
   evas_object_del(it->content);

   pd->items = eina_list_remove(pd->items, it);

   free(it);
}

static void
_check_size(Eo *obj, Elm_Tab_Pane_Data *pd)
{
  Evas_Object *sc = pd->scroller;
  Item *it;

  Evas_Coord w;
  Evas_Coord h;
  int x,y,wv,hv;

  elm_scroller_child_size_get(sc, &w, &h);

  evas_object_geometry_get(sc, &x, &y, &wv, &hv);

  if (w > wv)
    elm_object_signal_emit(obj, "scroll,show","elm");
  else
    elm_object_signal_emit(obj, "scroll,hide","elm");

  if (!pd->items) return;

  it = eina_list_data_get(pd->items);

  evas_object_size_hint_min_get(it->item, &w, &h);
  evas_object_size_hint_min_set(sc, -1, h);
}
EOLIAN static void
_elm_tab_pane_evas_object_smart_resize(Eo *obj, Elm_Tab_Pane_Data *pd, Evas_Coord w, Evas_Coord h)
{
   eo_do_super(obj, ELM_TAB_PANE_CLASS, evas_obj_smart_resize(w, h));
   _check_size(obj, pd);
}

EOLIAN static void
_elm_tab_pane_item_add(Eo *obj, Elm_Tab_Pane_Data *pd, Evas_Object *active, Evas_Object *content)
{
   Item *it = calloc(1, sizeof(Item));

   it->active = active;
   it->content = content;
   it->widget = obj;

   it->item = elm_layout_add(obj);
   elm_layout_signal_callback_add(it->item, "selected", "w",
                                  _item_clicked, it);
   elm_layout_signal_callback_add(it->item, "close", "w",
                                  _close_cb, it);
   if (!elm_layout_theme_set(it->item, "tab_pane", "item", "default"))
     {
        CRI("Failed to set theme file for the icon\n");
        return;
     }

   elm_object_part_content_set(it->item, "elm.content", it->active);
   evas_object_size_hint_align_set(it->item, 0.0, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(it->item, 0.0, EVAS_HINT_EXPAND);
   evas_object_show(it->item);

   elm_box_pack_before(pd->box, it->item, pd->add);

   if (!pd->items)
     {
        _focus_item(it);
        pd->focused = it;
     }
   else
     _unhighlight_item(it);

   pd->items = eina_list_append(pd->items, it);

   _check_size(obj, pd);
}

#include "elm_tab_pane.eo.x"
