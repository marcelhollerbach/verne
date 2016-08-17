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

static void
_hover(void *data EINA_UNUSED, const Eo_Event *event) {
   Elm_File_Icon *icon = event->info;
   Efm_File *file;
   const char *path;

   file = elm_obj_file_icon_file_get(icon);
   elm_file_selector_file_set(selector, file);
   path = efm_file_path_get(file);
   titlebar_path_set(path);
}

static Eina_List*
_convert( Elm_Selection_Data *data)
{
   Eina_List *result = NULL;
   char **files;
   char *string;
   //first check if this is a string which ends with \0

   string = malloc(sizeof(char) * (data->len + 1));

   memcpy(string, data->data, data->len);
   string[data->len] = '\0';

   files = eina_str_split(string, "\n", 0);
   for (int i = 0; files[i]; i++)
     {
        if (ecore_file_exists(files[i]))
          result = eina_list_append(result, files[i]);
     }

   return result;
}

static void
_drop(void *data EINA_UNUSED, const Eo_Event *event) {
    Elm_File_Selector_Dnd_Drop_Event *ev;
    Elm_Selection_Data *dnd_data;
    Efm_File *file;
    Eina_List *passes = NULL;

    ev = event->info;
    dnd_data = ev->selection_data;

    file = elm_obj_file_icon_file_get(ev->file);

    passes = _convert(dnd_data);

    fs_operations_copy(passes, efm_file_path_get(file));
}

void
shortcuts_init()
{
   //init key shortcuts
   evas_object_event_callback_add(selector, EVAS_CALLBACK_KEY_DOWN, _search_key_down, NULL);

   //add dnd shortcut
   efl_event_callback_add(selector, ELM_FILE_SELECTOR_EVENT_DND_ITEM_HOVER, _hover, NULL);

   //add dnd shortcut
   efl_event_callback_add(selector, ELM_FILE_SELECTOR_EVENT_DND_ITEM_DROPED, _drop, NULL);

   //add dnd shortcut
   efl_event_callback_add(selector, ELM_FILE_SELECTOR_EVENT_DND_DROPED, _drop, NULL);
}

static void
window_close(void)
{
  elm_exit();
}