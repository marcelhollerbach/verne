#define INEEDWIDGET
#include "../elementary_ext_priv.h"

typedef struct {
    Evas_Object *table;
    Evas_Object *control_box;
    Evas_Object *icon;
    Evas_Object *text;

    Eina_Stringshare *icon_text, *main;
} Elm_Dialog_Data;

EOLIAN static void
_elm_dialog_icon_set(Eo *obj EINA_UNUSED, Elm_Dialog_Data *pd, const char *icon)
{
   eina_stringshare_replace(&pd->icon_text, icon);
   elm_icon_standard_set(pd->icon, pd->icon_text);
}

EOLIAN static const char *
_elm_dialog_icon_get(Eo *obj EINA_UNUSED, Elm_Dialog_Data *pd)
{
   return pd->icon_text;
}

EOLIAN static Efl_Ui_Box *
_elm_dialog_control_box_get(Eo *obj EINA_UNUSED, Elm_Dialog_Data *pd)
{
   return pd->control_box;
}

static void
on_done(void *data EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   evas_object_del(obj);
}

static void
_internal_constructing(Eo *obj, Elm_Dialog_Data *pd)
{
/*
EOLIAN static Efl_Object*
_elm_dialog_efl_object_constructor(Eo *obj, Elm_Dialog_Data *pd)
{
   Eo *eo;
 */  Evas_Object *o;

   //eo = efl_constructor(efl_super(obj, ELM_DIALOG_CLASS));

   evas_object_smart_callback_add(obj, "delete,request", on_done, NULL);

   pd->table = o = elm_table_add(obj);
   elm_table_homogeneous_set(o, EINA_FALSE);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(o);

   o = elm_bg_add(obj);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(o);
   elm_table_pack(pd->table, o, 0, 0, 2, 3);

   pd->icon = o = elm_icon_add(obj);
   elm_icon_standard_set(o, pd->icon_text);
   evas_object_size_hint_align_set(o, 0.0, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(o, 0.0, EVAS_HINT_EXPAND);
   evas_object_show(o);
   evas_object_size_hint_min_set(o, 50, 50);
   elm_table_pack(pd->table, o, 0, 0, 1, 1);

   pd->text = o = elm_entry_add(obj);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_object_text_set(o, pd->main);
   evas_object_show(o);
   elm_table_pack(pd->table, o, 1, 0, 1, 1);

   o = elm_separator_add(obj);
   elm_separator_horizontal_set(o, EINA_TRUE);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(o);
   elm_table_pack(pd->table, o, 0, 1, 2, 1);

   pd->control_box = o = elm_box_add(obj);
   elm_box_horizontal_set(o, EINA_TRUE);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, 0.0);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, 0.0);
   evas_object_show(o);
   elm_table_pack(pd->table, o, 0, 2, 2, 1);

   //return eo;
}

EOLIAN static Efl_Object *
_elm_dialog_efl_object_finalize(Eo *obj, Elm_Dialog_Data *pd)
{
   efl_finalize(efl_super(obj, ELM_DIALOG_CLASS));

   _internal_constructing(obj, pd);
   elm_win_resize_object_add(obj, pd->table);

   return obj;
}

EOLIAN static void
_elm_dialog_elm_widget_part_text_set(Eo *obj EINA_UNUSED, Elm_Dialog_Data *pd, const char * part, const char *label)
{
   if (!part)
     {
        eina_stringshare_replace(&pd->main, label);
        elm_object_text_set(pd->text, pd->main);
     }
}


EOLIAN static const char *
_elm_dialog_elm_widget_part_text_get(Eo *obj EINA_UNUSED, Elm_Dialog_Data *pd, const char * part)
{
  if (!part) return pd->main;
  return NULL;
}

#include "elm_dialog.eo.x"