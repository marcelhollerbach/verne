#include "kvasir.h"

static void
_open(Efm_File *select)
{
    const char *fileending, *path;

    fileending = efm_file_fileending_get(select);
    path = efm_file_path_get(select);

    if (efm_file_is_type(select, EFM_FILE_TYPE_DIRECTORY))
      {
         elm_file_selector_file_set(selector, select);
      }
    else if (efm_archive_supported(EFM_CLASS, fileending))
      {
         select = efm_archive_get(EFM_CLASS, path, "/");
         elm_file_selector_file_set(selector, select);
      }
    else
      {
         exec_execute(select);
      }
}

static void
_open_cb(void *data EINA_UNUSED, const Eo_Event *event)
{
   _open(event->info);
}

static void
_open_cb2(void *data, Evas_Object *obj EINA_UNUSED, void *event EINA_UNUSED)
{
   _open(data);
}

static void
_open_with_choosen(Efm_File *f, Efreet_Desktop *name)
{
    exec_run(name, f);
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

#define ARCHIVE_FUNC_NAME(type) _create_##type

#define ARCHIVE_FUNC(type) \
static void \
ARCHIVE_FUNC_NAME(type)(void *data, Evas_Object *obj EINA_UNUSED, void *event EINA_UNUSED) \
{ \
   const char *path; \
   path = efm_file_path_get(data); \
   archive_create(path, type); \
}

ARCHIVE_FUNC(ARCHIVE_TYPE_ZIP)
ARCHIVE_FUNC(ARCHIVE_TYPE_TAR_GZ)
ARCHIVE_FUNC(ARCHIVE_TYPE_TAR_BZIP2)
ARCHIVE_FUNC(ARCHIVE_TYPE_TAR_XZ)
ARCHIVE_FUNC(ARCHIVE_TYPE_XZ)

static void
_extract(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event EINA_UNUSED)
{
   const char *path;
   path = efm_file_path_get(data);

   archive_extract(path);
}

static void
_menu_selector_start(void *data EINA_UNUSED, const Eo_Event *event)
{
    Elm_File_Selector_Menu_Hook *ev = event->info;
    Efm_File *file = ev->file;
    Elm_Object_Item *item;

    // open with entry
    if (file && !efm_file_is_type(file, EFM_FILE_TYPE_DIRECTORY))
      {
         elm_menu_item_add(ev->menu, NULL, NULL, "Open", _open_cb2, ev->file);
         elm_menu_item_add(ev->menu, NULL, NULL, "Open with", _open_with_cb, ev->file);
      }

    elm_menu_item_separator_add(ev->menu, NULL);
    elm_menu_item_add(ev->menu, NULL, NULL, "Copy", _copy_cb, ev->file);
    elm_menu_item_add(ev->menu, NULL, NULL, "Move", _move_cb, ev->file);
    elm_menu_item_add(ev->menu, NULL, NULL, "Remove", _remove_cb, ev->file);

    elm_menu_item_separator_add(ev->menu, NULL);

    if (file && !efm_file_is_type(file, EFM_FILE_TYPE_DIRECTORY))
      {
         elm_menu_item_add(ev->menu, NULL, NULL, "Extract here ", _extract, ev->file);
      }

    if (file && efm_file_is_type(file, EFM_FILE_TYPE_DIRECTORY))
      {
         item = elm_menu_item_add(ev->menu, NULL, NULL, "Create Archiv", NULL, ev->file);
         elm_menu_item_add(ev->menu, item, NULL, ".tar.gz", ARCHIVE_FUNC_NAME(ARCHIVE_TYPE_TAR_GZ), ev->file);
         elm_menu_item_add(ev->menu, item, NULL, ".tar.xz", ARCHIVE_FUNC_NAME(ARCHIVE_TYPE_TAR_XZ), ev->file);
         elm_menu_item_add(ev->menu, item, NULL, ".tar.bzip2", ARCHIVE_FUNC_NAME(ARCHIVE_TYPE_TAR_BZIP2), ev->file);
         elm_menu_item_add(ev->menu, item, NULL, ".zip", ARCHIVE_FUNC_NAME(ARCHIVE_TYPE_ZIP), ev->file);
         elm_menu_item_add(ev->menu, item, NULL, ".xz", ARCHIVE_FUNC_NAME(ARCHIVE_TYPE_XZ), ev->file);

      }

    elm_menu_item_separator_add(ev->menu, NULL);

    item = elm_menu_item_add(ev->menu, NULL, NULL, "Paste", _paste_cb, ev->file);
    if (clipboard_something_in())
      elm_object_item_disabled_set(item, EINA_TRUE);
}

static void
_menu_selector_end(void *data EINA_UNUSED, const Eo_Event *event EINA_UNUSED)
{

}

static void
_menu_bookmarks_start(void *data EINA_UNUSED, const Eo_Event *event EINA_UNUSED)
{

}

static void
_menu_bookmarks_end(void *data EINA_UNUSED, const Eo_Event *event EINA_UNUSED)
{

}

static void
_menu_device_start(void *data EINA_UNUSED, const Eo_Event *event EINA_UNUSED)
{

}

static void
_menu_device_end(void *data EINA_UNUSED, const Eo_Event *event EINA_UNUSED)
{

}

void
hooks_init(void)
{
   Evas_Object *bookmarks;

   bookmarks = elm_file_display_bookmarks_get(preview);

   efl_event_callback_add(selector, ELM_FILE_SELECTOR_EVENT_HOOK_MENU_SELECTOR_START, _menu_selector_start, NULL);
   efl_event_callback_add(selector, ELM_FILE_SELECTOR_EVENT_HOOK_MENU_SELECTOR_END, _menu_selector_end, NULL);
   efl_event_callback_add(selector, ELM_FILE_SELECTOR_EVENT_ITEM_CHOOSEN, _open_cb, NULL);

   efl_event_callback_add(bookmarks, ELM_FILE_BOOKMARKS_EVENT_HOOK_MENU_BOOKMARKS_START, _menu_bookmarks_start, NULL);
   efl_event_callback_add(bookmarks, ELM_FILE_BOOKMARKS_EVENT_HOOK_MENU_BOOKMARKS_END, _menu_bookmarks_end, NULL);
   efl_event_callback_add(bookmarks, ELM_FILE_BOOKMARKS_EVENT_HOOK_MENU_DEVICE_START, _menu_device_start, NULL);
   efl_event_callback_add(bookmarks, ELM_FILE_BOOKMARKS_EVENT_HOOK_MENU_DEVICE_END, _menu_device_end, NULL);
}