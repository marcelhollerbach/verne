#define INEEDWIDGET
#define ELM_WIDGET_PROTECTED
#include "../elementary_ext_priv.h"

#define PRIV_DATA(o) Elm_File_Display_Data *pd = efl_data_scope_get(o, ELM_FILE_DISPLAY_CLASS);

typedef struct
{
   Evas_Object *detail;
   Evas_Object *bookmark;
   Evas_Object *selector;

   Eina_Bool hide_fileinfo;
   Eina_Bool hide_bookmarks;
} Elm_File_Display_Data;

static void
_selector_path_changed(void *data, const Efl_Event *event)
{
   const char *file;
   PRIV_DATA(data)

   file = efm_file_path_get(event->info);
   efl_file_set(pd->bookmark, file, NULL);
}

static void
_update_preview(void *data, const Efl_Event *event)
{
   Efm_File *f;
   PRIV_DATA(data)

   f = event->info;

   elm_file_detail_file_set(pd->detail, f);
}

static void
_bookmark_path_changed(void *data,  const Efl_Event *event)
{
   const char *file;
   Efm_File *f;
   PRIV_DATA(data)

   efl_file_get(event->object, &file, NULL);
   f = efm_file_get(EFM_CLASS, file);
   elm_file_selector_file_set(pd->selector, f);
}

static void
_ctx_bookmarks_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event EINA_UNUSED)
{
   PRIV_DATA(data)
   elm_file_display_hide_bookmarks_set(data, !pd->hide_bookmarks);
}

static void
_ctx_preview_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event EINA_UNUSED)
{
   PRIV_DATA(data)
   elm_file_display_hide_fileinfo_set(data, !pd->hide_fileinfo);
}

static void
_ctx_rename(void *data, Evas_Object *obj EINA_UNUSED, void *event EINA_UNUSED)
{
   PRIV_DATA(data)

   elm_file_detail_rename(pd->detail);
}

static void
_menu_start_cb(void *data, const Efl_Event *event)
{
   Elm_File_Selector_Menu_Hook *ev = event->info;
   Evas_Object *it;
   Evas_Object *menu = ev->menu;
   Efm_File_Stat *st;

   elm_menu_item_separator_add(menu, NULL);

   st = efm_file_stat_get(ev->file);
   it = elm_menu_item_add(menu, NULL, "document-new", "Rename", _ctx_rename, data);

   if (st)
     elm_object_item_disabled_set(it, !(getuid() == (uid_t)st->uid));
}

static void
_menu_end_cb(void *data, const Efl_Event *event)
{
   PRIV_DATA(data)
   Elm_File_Selector_Menu_Hook *ev = event->info;
   Evas_Object *it, *ck;
   Evas_Object *menu = ev->menu;

   // Bookmarks enable / disable
   it = elm_menu_item_add(menu, NULL, NULL, NULL, _ctx_bookmarks_cb, data);
   ck = elm_check_add(menu);
   elm_check_state_set(ck, pd->hide_bookmarks);
   elm_object_text_set(ck, "Bookmarks");
   elm_object_item_content_set(it, ck);
   evas_object_show(ck);

   // Filepreview enable / disable
   it = elm_menu_item_add(menu, NULL, NULL, NULL, _ctx_preview_cb, data);
   ck = elm_check_add(menu);
   elm_check_state_set(ck, pd->hide_fileinfo);
   elm_object_text_set(ck, "File Details");
   elm_object_item_content_set(it, ck);
   evas_object_show(ck);
}

EOLIAN static Efl_Object *
_elm_file_display_efl_object_constructor(Eo *obj, Elm_File_Display_Data *pd)
{
   Eo *eo;
   Evas_Object *o;
   Eo *cache;

   elm_ext_config_init();

   if (config->hide_fileinfo != 0)
     {
        pd->hide_fileinfo = EINA_TRUE;
     } else {
        pd->hide_fileinfo = EINA_FALSE;
     }

   if (config->hide_bookmarks != 0)
     {
        pd->hide_bookmarks = EINA_TRUE;
     } else {
        pd->hide_bookmarks = EINA_FALSE;
     }

   eo = efl_constructor(efl_super(obj, ELM_FILE_DISPLAY_CLASS));

   if (!elm_layout_theme_set(obj, "file_display", "base", "default"))
     {
        CRI("Failed to set theme file\n");
     }

   pd->selector = o = efl_add(ELM_FILE_SELECTOR_CLASS, obj);

   cache = elm_file_selector_cache_get(o);
   efl_event_callback_add(o, ELM_FILE_SELECTOR_EVENT_HOOK_MENU_SELECTOR_START, _menu_start_cb, obj);
   efl_event_callback_add(o, ELM_FILE_SELECTOR_EVENT_HOOK_MENU_SELECTOR_END, _menu_end_cb, obj);
   efl_event_callback_add(o, ELM_FILE_SELECTOR_EVENT_PATH_CHANGED, _selector_path_changed, obj);
   efl_event_callback_add(o, ELM_FILE_SELECTOR_EVENT_PATH_CHANGED, _update_preview, obj);
   efl_event_callback_add(o, ELM_FILE_SELECTOR_EVENT_ITEM_SELECTED, _update_preview, obj);

   elm_object_part_content_set(obj, "content", o);
   evas_object_show(o);

   pd->bookmark = o = efl_add(ELM_FILE_BOOKMARKS_CLASS, obj, elm_file_bookmarks_cache_set(efl_added, cache));
   efl_event_callback_add(o, ELM_FILE_BOOKMARKS_EVENT_PATH_SELECTED, _bookmark_path_changed, obj);
   elm_object_part_content_set(obj, "bookmark", o);
   evas_object_show(o);

   pd->detail = o = efl_add(ELM_FILE_DETAIL_CLASS, obj);
   elm_file_detail_cache_set(o, cache);
   elm_object_part_content_set(obj, "filepreview", o);
   evas_object_show(o);


   elm_file_display_hide_bookmarks_set(obj, pd->hide_bookmarks);
   elm_file_display_hide_fileinfo_set(obj, pd->hide_fileinfo);

   return eo;
}

EOLIAN static void
_elm_file_display_hide_bookmarks_set(Eo *obj EINA_UNUSED, Elm_File_Display_Data *pd, Eina_Bool hide)
{
   config->hide_bookmarks = hide;
   elm_ext_config_save();
   pd->hide_bookmarks = hide;
   if (!pd->hide_bookmarks)
     elm_layout_signal_emit(obj, "bookmark,visible", "elm");
   else
     elm_layout_signal_emit(obj, "bookmark,invisible", "elm");
}

EOLIAN static Eina_Bool
_elm_file_display_hide_bookmarks_get(Eo *obj EINA_UNUSED, Elm_File_Display_Data *pd)
{
   return pd->hide_bookmarks;
}


EOLIAN static Eina_Bool
_elm_file_display_elm_widget_widget_event(Eo *obj, Elm_File_Display_Data *pd, const Efl_Event *eo_event, Efl_Canvas_Object *source EINA_UNUSED)
{
   Efl_Input_Key *ev;

   if (eo_event->desc != EFL_EVENT_KEY_DOWN)
     return EINA_FALSE;

   ev = eo_event->info;

   if (!strcmp(efl_input_key_get(ev), "F2"))
     {
        elm_file_detail_rename(pd->detail);
        return EINA_TRUE;
     }
   return EINA_FALSE;
}

EOLIAN static void
_elm_file_display_hide_fileinfo_set(Eo *obj EINA_UNUSED, Elm_File_Display_Data *pd, Eina_Bool hide)
{
   config->hide_fileinfo = hide;
   elm_ext_config_save();
   pd->hide_fileinfo = hide;
   if (!pd->hide_fileinfo)
     elm_layout_signal_emit(obj, "filepreview,visible", "elm");
   else
     elm_layout_signal_emit(obj, "filepreview,invisible", "elm");
}

EOLIAN static Eina_Bool
_elm_file_display_hide_fileinfo_get(Eo *obj EINA_UNUSED, Elm_File_Display_Data *pd)
{
   return pd->hide_fileinfo;
}

EOLIAN static Evas_Object *
_elm_file_display_fileinfo_get(Eo *obj EINA_UNUSED, Elm_File_Display_Data *pd)
{
  return pd->detail;
}

EOLIAN static Evas_Object *
_elm_file_display_bookmarks_get(Eo *obj EINA_UNUSED, Elm_File_Display_Data *pd)
{
   return pd->bookmark;
}

EOLIAN static Evas_Object *
_elm_file_display_selector_get(Eo *obj EINA_UNUSED, Elm_File_Display_Data *pd)
{
   return pd->selector;
}

#include "elm_file_display.eo.x"
