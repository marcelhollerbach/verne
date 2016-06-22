#include "jesus.h"

static Eina_List *list = NULL;
static unsigned int pointer = 0;
static Eina_Bool barrier = EINA_FALSE;

static void
_history_push(Efm_File *file)
{
   Eina_Stringshare *path;

   path = eina_stringshare_add(
    efm_file_path_get(file)
   );

   list = eina_list_append(list, path);
}

static void
_history_pop(void)
{
   Eina_List *last;

   last = eina_list_last(list);

   list = eina_list_remove_list(list, last);

}

static const char*
_history_get(void)
{
   return eina_list_last_data_get(list);
}

static const char*
_history_neg(int i)
{
   return eina_list_nth(list, eina_list_count(list) - (i + 1));
}

static void
_flush(void)
{
    const char *path;
    Efm_File *file;

    path = _history_neg(pointer);
    file = efm_file_get(EFM_CLASS, path);

    barrier = EINA_TRUE;
    elm_file_selector_file_set(selector, file);
    barrier = EINA_FALSE;
}

static void
_back_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, const char *emission EINA_UNUSED, const char *source EINA_UNUSED)
{
   if (pointer + 1 == eina_list_count(list)) return;

   pointer++;
   _flush();
}

static void
_forward_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, const char *emission EINA_UNUSED, const char *source EINA_UNUSED)
{
   if (pointer == 0) return;

   pointer --;
   _flush();
}

static void
_path_changed_cb(void *data EINA_UNUSED, const Eo_Event *event)
{
   if (barrier) return;
   if (event->info == _history_get()) return;

   while(pointer > 0)
     {
        pointer --;
        _history_pop();
     }

   _history_push(event->info);
}

void
history_init(void)
{
   elm_layout_signal_callback_add(layout, "jesus.history.back", "theme", _back_cb, NULL);
   elm_layout_signal_callback_add(layout, "jesus.history.forward", "theme", _forward_cb, NULL);
   eo_event_callback_add(selector, ELM_FILE_SELECTOR_EVENT_PATH_CHANGED, _path_changed_cb, NULL);
}