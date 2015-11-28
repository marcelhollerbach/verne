#include "jesus.h"

#define SEP "<item relsize=15x15 vsize=full href=path/separator></item>"

struct tuple {
  const char *now;
  Evas_Object *obj;
};

static Ecore_Idler *focus_idler;

static Eina_Bool unfocus_barrier;
static Evas_Object *entry;

static const char*
_path_transform(const char *text)
{
    Eina_Bool dir_break;
    char **parts;
    Eina_Strbuf *buf, *dir;
    int c = 0;
    const char *result;

    // split for /
    parts = eina_str_split(text, SEP, 0);

    // init result
    buf = eina_strbuf_new();
    dir = eina_strbuf_new();
    dir_break = EINA_FALSE;

    for(c = 0;parts[c]; c++)
      {
         char part[PATH_MAX];
         const char *dirr;

         if (*parts[c] == '\0')
           continue;

         // update durrect dir
         eina_strbuf_append(dir, "/");
         eina_strbuf_append(dir, parts[c]);
         dirr = eina_strbuf_string_get(dir);

         if (!dir_break && ecore_file_exists(dirr))
           {
              snprintf(part, sizeof(part), "<a href=%s>%s</a>", dirr, parts[c]);
           }
         else
           {
              snprintf(part, sizeof(part), "%s", parts[c]);
              dir_break = EINA_TRUE;
           }

         eina_strbuf_append(buf, SEP);
         eina_strbuf_append(buf, part);
      }

   result = (eina_strbuf_string_steal(buf));
   eina_strbuf_free(buf);
   eina_strbuf_free(dir);
   free(parts);

   return result;
}

static void
_markup_filter(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, char ** text)
{
   char *n = *text;
   int c = 0;
   Eina_Strbuf *buf;

   buf = eina_strbuf_new();

   while(n[c] != '\0')
     {
        if (n[c] == '/')
          {
             n[c] = '\0';

             eina_strbuf_append(buf, SEP);
          }
        else
          eina_strbuf_append_char(buf, n[c]);
        c ++;
     }
   free(*text);
   *text = eina_strbuf_string_steal(buf);
   eina_strbuf_free(buf);
}

static Eina_Bool
_focus_idler(void *data)
{
   const char *text;


   text = evas_object_data_del(data, "__orig_text");

   elm_object_text_set(data, text);

   free((char*)text);
   unfocus_barrier = EINA_TRUE;

   return EINA_FALSE;
}

static void
_trigger_change(const char *text) {
    char **parts;
    char path[PATH_MAX];

    parts = eina_str_split(text, SEP, 0);

    path[0] = '\0';

    for(int i = 0; parts[i]; i++) {
      strcat(path, "/");
      strcat(path, parts[i]);
    }

    eo_do(selector, efl_file_set(path, NULL));
}

static Eina_Bool
_unfocus_idler(void *data)
{
    const char *text;

    if (!unfocus_barrier)
      return EINA_FALSE;

    unfocus_barrier = EINA_FALSE;
    // get the currect text
    text = elm_object_text_get(data);

    _trigger_change(text);

    // save the current state
    evas_object_data_set(data, "__orig_text", strdup(text));

    elm_object_text_set(data, _path_transform(text));


    return EINA_FALSE;
}

static void
_focus_cb(void *data EINA_UNUSED, Evas_Object *obj, void *event EINA_UNUSED)
{
   focus_idler = ecore_idler_add(_focus_idler, obj);
}

static void
_unfocus_cb(void *data EINA_UNUSED, Evas_Object *obj, void *event EINA_UNUSED)
{
   ecore_idler_add(_unfocus_idler, obj);
}

static Eina_Bool
_change_idle(void *data)
{
   struct tuple *h = data;

   elm_object_text_set(h->obj, h->now);

   free(h);

   return EINA_FALSE;
}

static void
_entry_transform(Evas_Object *obj)
{
   const char *text;
   const char *transform;


   if (elm_object_focus_get(obj))
     return;

   text = elm_object_text_get(obj);

   evas_object_data_set(obj, "__orig_text", strdup(text));

   transform = _path_transform(text);

   struct tuple *h;

   h = calloc(1, sizeof(struct tuple));
   h->obj = obj;
   h->now = transform;

   ecore_idler_add(_change_idle, h);
}

static void
_changed_cb(void *data EINA_UNUSED, Evas_Object *obj, void *event EINA_UNUSED)
{
   _entry_transform(obj);
}

static void
_anchor_clicked_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event)
{
   Elm_Entry_Anchor_Info *info = event;

   if (focus_idler)
     ecore_idler_del(focus_idler);

   elm_object_focus_set(obj, EINA_FALSE);
   elm_object_text_set(obj, NULL);
   elm_entry_entry_append(obj, info->name);
   eo_do(selector, efl_file_set(info->name, NULL));
}

void
titlebar_init(void)
{
   entry = elm_entry_add(layout);
   elm_entry_single_line_set(entry, EINA_TRUE);
   elm_entry_markup_filter_append(entry, _markup_filter, NULL);
   evas_object_smart_callback_add(entry, "focused", _focus_cb, NULL);
   evas_object_smart_callback_add(entry, "unfocused", _unfocus_cb, NULL);
   evas_object_smart_callback_add(entry, "changed,user", _changed_cb, NULL);
   evas_object_smart_callback_add(entry, "anchor,clicked", _anchor_clicked_cb, NULL);
   evas_object_show(entry);

   elm_object_part_content_set(layout, "jesus.textbar", entry);

   unfocus_barrier = EINA_FALSE;
}

void
titlebar_path_set(const char *path)
{
   elm_entry_entry_set(entry, NULL);
   elm_entry_entry_append(entry, path);
   _entry_transform(entry);
}