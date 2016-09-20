#define INEEDWIDGET
#include "../elementary_ext_priv.h"

typedef struct {
   Evas_Object *apply;
   Evas_Object *cancel;
} Elm_Dialog_Decision_Data;


static void
_cancel(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   efl_event_callback_call(data, ELM_DIALOG_DECISION_EVENT_CANCEL, NULL);
}


static void
_apply(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   efl_event_callback_call(data, ELM_DIALOG_DECISION_EVENT_APPLY , NULL);
}

EOLIAN static void
_elm_dialog_decision_efl_canvas_group_group_add(Eo *obj, Elm_Dialog_Decision_Data *pd)
{
   Evas_Object *bx, *o;
   efl_canvas_group_add(efl_super(obj, ELM_DIALOG_DECISION_CLASS));

   bx = elm_dialog_control_box_get(obj);

   pd->apply = o = elm_button_add(obj);
   elm_object_text_set(o, "Apply");
   evas_object_smart_callback_add(o, "clicked", _apply, obj);
   elm_box_pack_end(bx, o);
   evas_object_show(o);

   pd->cancel = o = elm_button_add(obj);
   elm_object_text_set(o, "Cancel");
   evas_object_smart_callback_add(o, "clicked", _cancel, obj);
   elm_box_pack_end(bx, o);
   evas_object_show(o);
}

#include "elm_dialog_decision.eo.x"