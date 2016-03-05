#include "jesus.h"

static Eina_List *list = NULL;
static unsigned int pointer;
static Eina_Bool barrier = EINA_FALSE;

static void
_back_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, const char *emission EINA_UNUSED, const char *source EINA_UNUSED)
{
    const char *path;
    Efm_File *file;

    if (pointer < 1)
      return;

    pointer --;

    path = eina_list_nth(list, pointer);
    barrier = EINA_TRUE;
    file = efm_file_get(EFM_CLASS, path);
    elm_file_selector_file_set(selector, file);
    barrier = EINA_FALSE;
}

static void
_forward_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, const char *emission EINA_UNUSED, const char *source EINA_UNUSED)
{
   const char *pointed;
   Efm_File *file;

   if (pointer == eina_list_count(list) + 1)
     return;

   pointer ++;

   pointed = eina_list_nth(list, pointer);

   file = efm_file_get(EFM_CLASS, pointed);
   elm_file_selector_file_set(selector, file);
}

static Eina_Bool
_path_changed_cb(void *data EINA_UNUSED, const Eo_Event *event)
{
   const char *pointed;
   const char *share;
   const char *path;

   path = efm_file_path_get(event->event_info);

   if (barrier)
     return EINA_TRUE;

   if (eina_list_count(list) > pointer +1)
     {
        pointed = eina_list_nth(list, pointer + 1);

        if (pointed && !strcmp(pointed , path))
          {
             //
             pointer ++;
             return EINA_TRUE;
          }
        // unpop the upper items
        while(eina_list_count(list) > pointer + 1)
          {
             const char *item;

             item = eina_list_data_get(eina_list_last(list));
             list = eina_list_remove(list, item);
             eina_stringshare_del(item);
          }
     }

   share = eina_stringshare_add(path);
   list = eina_list_append(list, share);

   pointer = eina_list_count(list) - 1;

   return EINA_TRUE;
}

void
history_init(void)
{
   elm_layout_signal_callback_add(layout, "jesus.history.back", "theme", _back_cb, NULL);
   elm_layout_signal_callback_add(layout, "jesus.history.forward", "theme", _forward_cb, NULL);
   eo_event_callback_add(selector, ELM_FILE_SELECTOR_EVENT_PATH_CHANGED, _path_changed_cb, NULL);
}