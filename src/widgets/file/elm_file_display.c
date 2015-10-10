#define EFL_BETA_API_SUPPORT
#include "../elementary_ext_priv.h"

#define PRIV_DATA(o) Elm_File_Display_Data *pd = eo_data_scope_get(o, ELM_FILE_DISPLAY_CLASS);

typedef struct
{
   Evas_Object *preview;
   Evas_Object *bookmark;
   Evas_Object *selector;

   Eina_Bool preview_show;
   Eina_Bool bookmarks_show;
} Elm_File_Display_Data;

static Eina_Bool
_selector_path_changed(void *data, Eo *obj, const Eo_Event_Description2 *desc EINA_UNUSED, void *event_info EINA_UNUSED)
{
   const char *file;
   PRIV_DATA(data)

   eo_do(obj, efl_file_get(&file, NULL));
   eo_do(pd->bookmark, efl_file_set(file, NULL));

   return EO_CALLBACK_CONTINUE;
}

static Eina_Bool
_update_preview(void *data, Eo *obj EINA_UNUSED, const Eo_Event_Description2 *desc EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Efm_File *f;
   PRIV_DATA(data)

   if (desc == ELM_FILE_SELECTOR_EVENT_ITEM_SELECTED)
     f = event_info;
   else
     eo_do(EFM_FILE_CLASS, f = efm_file_generate(event_info));

   eo_do(pd->preview, elm_file_preview_file_set(f));
   return EO_CALLBACK_CONTINUE;
}

static Eina_Bool
_bookmark_path_changed(void *data, Eo *obj, const Eo_Event_Description2 *desc EINA_UNUSED, void *event_info EINA_UNUSED)
{
   const char *file;
   PRIV_DATA(data)

   eo_do(obj, efl_file_get(&file, NULL));
   eo_do(pd->selector, efl_file_set(file, NULL));

   return EO_CALLBACK_CONTINUE;
}

static void
_ctx_bookmarks_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event EINA_UNUSED)
{
   PRIV_DATA(data)

   eo_do(data, elm_file_display_bookmarks_show_set(!pd->bookmarks_show));
}

static void
_ctx_preview_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event EINA_UNUSED)
{
   PRIV_DATA(data)

   eo_do(data, elm_file_display_filepreview_show_set(!pd->preview_show));
}

static Eina_Bool
_menu_cb(void *data, Eo *obj EINA_UNUSED, const Eo_Event_Description2 *desc EINA_UNUSED, void *event_info EINA_UNUSED)
{
   PRIV_DATA(data)
   Elm_File_Selector_Menu_Hook *ev = event_info;
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
   elm_object_text_set(ck, "Preview");
   elm_object_item_content_set(it, ck);
   evas_object_show(ck);

   return EO_CALLBACK_CONTINUE;
}

EOLIAN static void
_elm_file_display_eo_base_destructor(Eo *obj, Elm_File_Display_Data *pd)
{
   eo_do_super(obj, ELM_FILE_DISPLAY_CLASS, eo_destructor());
   eo_del(pd->preview);
   eo_del(pd->selector);
   eo_del(pd->bookmark);
}

EOLIAN static Eo_Base *
_elm_file_display_eo_base_constructor(Eo *obj, Elm_File_Display_Data *pd EINA_UNUSED)
{
   Eo *eo;

   eo_do_super(obj, ELM_FILE_DISPLAY_CLASS, eo = eo_constructor());

   // XXX: take a config ?
   eo_do(obj, elm_file_display_bookmarks_show_set(EINA_TRUE));
   eo_do(obj, elm_file_display_filepreview_show_set(EINA_TRUE));

   return eo;
}

EOLIAN static void
_elm_file_display_evas_object_smart_add(Eo *obj, Elm_File_Display_Data *pd)
{
   Evas_Object *o;
   Eo *cache;

   eo_do_super(obj, ELM_FILE_DISPLAY_CLASS, evas_obj_smart_add());

   if (!elm_layout_theme_set(obj, "file_display", "base", "default"))
     {
        CRI("Failed to set theme file\n");
     }

   pd->selector = o = eo_add(ELM_FILE_SELECTOR_CLASS, obj);
   eo_do(o,
    cache = elm_file_selector_cache_get();
    eo_event_callback_add(ELM_FILE_SELECTOR_EVENT_HOOK_MENU_SELECTOR_END, _menu_cb, obj);
    eo_event_callback_add(ELM_FILE_SELECTOR_EVENT_PATH_CHANGED_USER, _selector_path_changed, obj);
    eo_event_callback_add(ELM_FILE_SELECTOR_EVENT_PATH_CHANGED_USER, _update_preview, obj);
    eo_event_callback_add(ELM_FILE_SELECTOR_EVENT_ITEM_SELECTED, _update_preview, obj);
    );
   elm_object_part_content_set(obj, "content", o);
   evas_object_show(o);

   pd->bookmark = o = eo_add(ELM_FILE_BOOKMARKS_CLASS, obj);
   eo_do(o, eo_event_callback_add(ELM_FILE_BOOKMARKS_EVENT_PATH_SELECTED, _bookmark_path_changed, obj));
   elm_object_part_content_set(obj, "bookmark", o);
   evas_object_show(o);

   pd->preview = o = eo_add(ELM_FILE_PREVIEW_CLASS, obj);
   eo_do(o,
    elm_file_preview_cache_set(cache)
   );
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
  return pd->preview;
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
