#include "jesus.h"

static Eina_Bool
_open_cb(void *data EINA_UNUSED, Eo *obj EINA_UNUSED, const Eo_Event_Description *desc EINA_UNUSED, void *event)
{
    Efm_File *select;
    Eina_Bool b;
    const char *fileending, *path;

    select = event;
    eo_do(select, fileending = efm_file_fileending_get();
                  path = efm_file_path_get());

    if (eo_do_ret(select, b, efm_file_is_type(EFM_FILE_TYPE_DIRECTORY)))
      {
         eo_do(selector, elm_file_selector_file_set(select));
      }
    else if (eo_do_ret(EFM_CLASS, b, efm_archive_supported(fileending)))
      {
         eo_do(EFM_CLASS, select = efm_archive_get(path, "/"));
         eo_do(selector, elm_file_selector_file_set(select));
         return EO_CALLBACK_CONTINUE;
      }
    else
      {
         exec_execute(select);
      }
    return EINA_TRUE;
}

static void
_open_cb2(void *data, Evas_Object *obj EINA_UNUSED, void *event EINA_UNUSED)
{
   _open_cb(NULL, NULL, NULL, data);
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
   eo_do(data, path = efm_file_path_get()); \
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
   eo_do(data, path = efm_file_path_get());

   archive_extract(path);
}

static Eina_Bool
_menu_selector_start(void *data EINA_UNUSED, Eo *obj EINA_UNUSED, const Eo_Event_Description *desc EINA_UNUSED, void *event)
{
    Elm_File_Selector_Menu_Hook *ev = event;
    Efm_File *file = ev->file;
    Eina_Bool dir;
    Elm_Object_Item *item;

    // open with entry
    if (file && !eo_do_ret(file, dir, efm_file_is_type(EFM_FILE_TYPE_DIRECTORY)))
      {
         elm_menu_item_add(ev->menu, NULL, NULL, "Open", _open_cb2, ev->file);
         elm_menu_item_add(ev->menu, NULL, NULL, "Open with", _open_with_cb, ev->file);
      }

    elm_menu_item_separator_add(ev->menu, NULL);
    elm_menu_item_add(ev->menu, NULL, NULL, "Copy", _copy_cb, ev->file);
    elm_menu_item_add(ev->menu, NULL, NULL, "Move", _move_cb, ev->file);
    elm_menu_item_add(ev->menu, NULL, NULL, "Remove", _remove_cb, ev->file);

    elm_menu_item_separator_add(ev->menu, NULL);

    if (file && !eo_do_ret(file, dir, efm_file_is_type(EFM_FILE_TYPE_DIRECTORY)))
      {
         elm_menu_item_add(ev->menu, NULL, NULL, "Extract here ", _extract, ev->file);
      }

    if (file && eo_do_ret(file, dir, efm_file_is_type(EFM_FILE_TYPE_DIRECTORY)))
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
   Evas_Object *bookmarks;

   eo_do(preview, bookmarks = elm_file_display_bookmarks_get());
   eo_do(selector,
        eo_event_callback_add(ELM_FILE_SELECTOR_EVENT_HOOK_MENU_SELECTOR_START, _menu_selector_start, NULL);
        eo_event_callback_add(ELM_FILE_SELECTOR_EVENT_HOOK_MENU_SELECTOR_END, _menu_selector_end, NULL);
        eo_event_callback_add(ELM_FILE_SELECTOR_EVENT_ITEM_CHOOSEN, _open_cb, NULL););

   eo_do(bookmarks,
        eo_event_callback_add(ELM_FILE_BOOKMARKS_EVENT_HOOK_MENU_BOOKMARKS_START, _menu_bookmarks_start, NULL);
        eo_event_callback_add(ELM_FILE_BOOKMARKS_EVENT_HOOK_MENU_BOOKMARKS_END, _menu_bookmarks_end, NULL);
        eo_event_callback_add(ELM_FILE_BOOKMARKS_EVENT_HOOK_MENU_DEVICE_START, _menu_device_start, NULL);
        eo_event_callback_add(ELM_FILE_BOOKMARKS_EVENT_HOOK_MENU_DEVICE_END, _menu_device_end, NULL););
}