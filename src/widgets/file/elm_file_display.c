#define INEEDWIDGET
#include "../elementary_ext_priv.h"

#define PRIV_DATA(o) Elm_File_Display_Data *pd = efl_data_scope_get(o, ELM_FILE_DISPLAY_CLASS);

typedef struct
{
   Evas_Object *detail;
   Evas_Object *bookmark;
   Evas_Object *selector;

   Eina_Bool preview_show;
   Eina_Bool bookmarks_show;
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

   elm_file_display_bookmarks_show_set(data, !pd->bookmarks_show);
}

static void
_ctx_preview_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event EINA_UNUSED)
{
   PRIV_DATA(data)

   elm_file_display_filepreview_show_set(data, !pd->preview_show);
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

   elm_menu_item_separator_add(menu, NULL);

   it = elm_menu_item_add(menu, NULL, "document-new", "Rename", _ctx_rename, data);
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
   elm_check_state_set(ck, pd->bookmarks_show);
   elm_object_text_set(ck, "Bookmarks");
   elm_object_item_content_set(it, ck);
   evas_object_show(ck);

   // Filepreview enable / disable
   it = elm_menu_item_add(menu, NULL, NULL, NULL, _ctx_preview_cb, data);
   ck = elm_check_add(menu);
   elm_check_state_set(ck, pd->preview_show);
   elm_object_text_set(ck, "File Details");
   elm_object_item_content_set(it, ck);
   evas_object_show(ck);
}

EOLIAN static Efl_Object *
_elm_file_display_efl_object_constructor(Eo *obj, Elm_File_Display_Data *pd EINA_UNUSED)
{
   Eo *eo;

   eo = efl_constructor(efl_super(obj, ELM_FILE_DISPLAY_CLASS));
   // XXX: take a config ?
   elm_file_display_bookmarks_show_set(obj, EINA_TRUE);
   elm_file_display_filepreview_show_set(obj, EINA_TRUE);

   return eo;
}

EOLIAN static void
_elm_file_display_efl_canvas_group_group_add(Eo *obj, Elm_File_Display_Data *pd)
{
   Evas_Object *o;
   Eo *cache;

   efl_canvas_group_add(efl_super(obj, ELM_FILE_DISPLAY_CLASS));

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


}

EOLIAN static void
_elm_file_display_bookmarks_show_set(Eo *obj EINA_UNUSED, Elm_File_Display_Data *pd, Eina_Bool bookmark)
{
   pd->bookmarks_show = bookmark;
   if (pd->bookmarks_show)
     elm_layout_signal_emit(obj, "bookmark,visible", "elm");
   else
     elm_layout_signal_emit(obj, "bookmark,invisible", "elm");
}

EOLIAN static Eina_Bool
_elm_file_display_bookmarks_show_get(Eo *obj EINA_UNUSED, Elm_File_Display_Data *pd)
{
   return pd->bookmarks_show;
}

EOLIAN static Eina_Bool
_elm_file_display_elm_widget_event(Eo *obj EINA_UNUSED, Elm_File_Display_Data *pd, Efl_Canvas_Object *source EINA_UNUSED, Evas_Callback_Type type, void *event_info)
{
   Evas_Event_Key_Down *ev;

   if (type != EVAS_CALLBACK_KEY_DOWN)
     return EINA_FALSE;

   ev = event_info;

   if (!strcmp(ev->key, "F2"))
     {
        elm_file_detail_rename(pd->detail);
        return EINA_TRUE;
     }
   return EINA_FALSE;
}

EOLIAN static void
_elm_file_display_filepreview_show_set(Eo *obj EINA_UNUSED, Elm_File_Display_Data *pd, Eina_Bool filepreview)
{
   pd->preview_show = filepreview;
   if (pd->preview_show)
     elm_layout_signal_emit(obj, "filepreview,visible", "elm");
   else
     elm_layout_signal_emit(obj, "filepreview,invisible", "elm");
}

EOLIAN static Eina_Bool
_elm_file_display_filepreview_show_get(Eo *obj EINA_UNUSED, Elm_File_Display_Data *pd)
{
   return pd->preview_show;
}

EOLIAN static Evas_Object *
_elm_file_display_filepreview_get(Eo *obj EINA_UNUSED, Elm_File_Display_Data *pd)
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
