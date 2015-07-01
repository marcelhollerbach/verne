#include "jesus.h"

static Eina_Bool
_open_cb(void *data EINA_UNUSED, Eo *obj EINA_UNUSED, const Eo_Event_Description *desc EINA_UNUSED, void *event)
{
    Efm_File *select;
    Eina_List *mime_types;
    Efreet_Desktop *icon;
    const char *mime_type = NULL;
    const char *command = NULL;

    select = event;

    eo_do(select, mime_type = efm_file_mimetype_get());

    //first check in config for a "special" open wish
    command = eina_hash_find(config->mime_type_open, mime_type);

    if (command)
      goto open;

    mime_types = efreet_util_desktop_mime_list(mime_type);

    if (!mime_types)
      {
         //todo error
         return EINA_TRUE;
      }

    icon = eina_list_data_get(mime_types);
    command = icon->exec;
open:
    exec_run(command, select);
    return EINA_TRUE;
}

static void
_open_cb2(void *data, Evas_Object *obj EINA_UNUSED, void *event EINA_UNUSED)
{
   _open_cb(NULL, NULL, NULL, data);
}

static void
_open_with_choosen(Efm_File *f, const char *cmd)
{
    exec_run(cmd, f);
}

static void
_open_with_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event EINA_UNUSED)
{
   exec_ui_open_with(data, _open_with_choosen);
}

static void
_copy_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event EINA_UNUSED)
{
   preview_copy();
}

static void
_move_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event EINA_UNUSED)
{
   preview_move();
}

static void
_remove_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event EINA_UNUSED)
{
   preview_remove();
}

static void
_paste_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event EINA_UNUSED)
{
   preview_paste();
}

static Eina_Bool
_menu_selector_start(void *data EINA_UNUSED, Eo *obj EINA_UNUSED, const Eo_Event_Description *desc EINA_UNUSED, void *event)
{
    Elm_File_Display_Menu_Hook *ev = event;
    Efm_File *file = ev->file;
    Eina_Bool dir;
    Elm_Object_Item *item;

    //open with entry
    if (!eo_do_ret(file, dir, efm_file_is_type(EFM_FILE_TYPE_DIRECTORY)))
      {
         elm_menu_item_add(ev->menu, NULL, NULL, "Open", _open_cb2, ev->file);
         elm_menu_item_add(ev->menu, NULL, NULL, "Open with", _open_with_cb, ev->file);
      }

    elm_menu_item_separator_add(ev->menu, NULL);
    elm_menu_item_add(ev->menu, NULL, NULL, "Copy", _copy_cb, ev->file);
    elm_menu_item_add(ev->menu, NULL, NULL, "Move", _move_cb, ev->file);
    elm_menu_item_add(ev->menu, NULL, NULL, "Remove", _remove_cb, ev->file);

    item = elm_menu_item_add(ev->menu, NULL, NULL, "Paste", _paste_cb, ev->file);
    if (clipboard_something_in())
      elm_object_item_disabled_set(item, EINA_TRUE);

    return EINA_TRUE;
}

static Eina_Bool
_menu_selector_end(void *data EINA_UNUSED, Eo *obj EINA_UNUSED, const Eo_Event_Description *desc EINA_UNUSED, void *event EINA_UNUSED)
{

    return EINA_TRUE;
}

static Eina_Bool
_menu_bookmarks_start(void *data EINA_UNUSED, Eo *obj EINA_UNUSED, const Eo_Event_Description *desc EINA_UNUSED, void *event EINA_UNUSED)
{

    return EINA_TRUE;
}

static Eina_Bool
_menu_bookmarks_end(void *data EINA_UNUSED, Eo *obj EINA_UNUSED, const Eo_Event_Description *desc EINA_UNUSED, void *event EINA_UNUSED)
{

    return EINA_TRUE;
}

static Eina_Bool
_menu_device_start(void *data EINA_UNUSED, Eo *obj EINA_UNUSED, const Eo_Event_Description *desc EINA_UNUSED, void *event EINA_UNUSED)
{

    return EINA_TRUE;
}

static Eina_Bool
_menu_device_end(void *data EINA_UNUSED, Eo *obj EINA_UNUSED, const Eo_Event_Description *desc EINA_UNUSED, void *event EINA_UNUSED)
{

    return EINA_TRUE;
}

void
hooks_init(void)
{
    eo_do(preview, eo_event_callback_add(ELM_FILE_DISPLAY_EVENT_HOOK_MENU_SELECTOR_START, _menu_selector_start, NULL);
        eo_event_callback_add(ELM_FILE_DISPLAY_EVENT_HOOK_MENU_SELECTOR_END, _menu_selector_end, NULL);
        eo_event_callback_add(ELM_FILE_DISPLAY_EVENT_HOOK_MENU_BOOKMARKS_START, _menu_bookmarks_start, NULL);
        eo_event_callback_add(ELM_FILE_DISPLAY_EVENT_HOOK_MENU_BOOKMARKS_END, _menu_bookmarks_end, NULL);
        eo_event_callback_add(ELM_FILE_DISPLAY_EVENT_HOOK_MENU_DEVICE_START, _menu_device_start, NULL);
        eo_event_callback_add(ELM_FILE_DISPLAY_EVENT_HOOK_MENU_DEVICE_END, _menu_device_end, NULL);
        eo_event_callback_add(ELM_FILE_DISPLAY_EVENT_ITEM_CHOOSEN, _open_cb, NULL);
        );
}