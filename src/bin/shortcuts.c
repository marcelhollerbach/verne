#include "jesus.h"

typedef struct {
    const char *key; // the key which needs to be set
    struct modifier {
        Eina_Bool ctrl;
        Eina_Bool alt;
        Eina_Bool shift;
        Eina_Bool win;
        Eina_Bool meta;
        Eina_Bool hyper;
    } mods;
    void (*callback)(void); // callback which will get called
} Shortcut;

#define ONLY_CTRL {EINA_TRUE, EINA_FALSE, EINA_FALSE, EINA_FALSE, EINA_FALSE, EINA_FALSE}

#define CHECK(v, k) \
    if (v && !evas_key_modifier_is_set(ev->modifiers, k)) \
      return EINA_FALSE;

static void window_close(void);

// the list of shortcuts
static Shortcut shortcuts[] = {
    {"c", ONLY_CTRL, preview_copy},
    {"v", ONLY_CTRL, preview_paste},
    {"x", ONLY_CTRL, preview_move},
    {"q", ONLY_CTRL, window_close},
    {NULL, ONLY_CTRL, NULL}
};

static Eina_Bool
_mods_check(Shortcut *sc, Evas_Event_Key_Down *ev)
{
    Eina_Bool meta;

    CHECK(sc->mods.ctrl, "Control");
    CHECK(sc->mods.alt, "Alt");
    CHECK(sc->mods.shift, "Shift");
    CHECK(sc->mods.win, "Super");
    CHECK(sc->mods.hyper, "hyper")

    meta = evas_key_modifier_is_set(ev->modifiers, "Meta") ||
           evas_key_modifier_is_set(ev->modifiers, "AltGr") ||
           evas_key_modifier_is_set(ev->modifiers, "ISO_Level3_Shift");

    if (sc->mods.meta && !meta)
      return EINA_FALSE;

    return EINA_TRUE;

}

static void
_search_key_down(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
    Evas_Event_Key_Down *ev = event_info;
    for (int i = 0; shortcuts[i].key ; i++)
      {
         if (!!strcmp(ev->key, shortcuts[i].key))
           continue;
         if (!_mods_check(&shortcuts[i], ev))
           continue;

         shortcuts[i].callback();
      }

}

static Eina_Bool
_hover(void *data EINA_UNUSED, Eo *obj EINA_UNUSED, const Eo_Event_Description2 *desc EINA_UNUSED, void *event_info) {
   Elm_File_Icon *icon = event_info;
   Efm_File *file;
   const char *path;

   eo_do(icon, file = elm_obj_file_icon_file_get());
   eo_do(selector, elm_file_selector_file_set(file));
   eo_do(file, path = efm_file_path_get());
   titlebar_path_set(path);

   return EO_CALLBACK_CONTINUE;
}

void
shortcuts_init()
{
   //init key shortcuts
   evas_object_event_callback_add(selector, EVAS_CALLBACK_KEY_DOWN, _search_key_down, NULL);

   //add dnd shortcut
   eo_do(selector,
    eo_event_callback_add(ELM_FILE_SELECTOR_EVENT_DND_ITEM_HOVER, _hover, NULL));
}

static void
window_close(void)
{
  elm_exit();
}