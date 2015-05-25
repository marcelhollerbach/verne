#include "jesus.h"

#define SEP "<item relsize=15x15 vsize=full href=path/separator></item>"

struct tuple {
  const char *now;
  Evas_Object *obj;
};

static Eina_Bool planed_changed;
static Ecore_Idler *focus_idler;
static Eina_Bool unfocus_barrier;

static const char*
_path_transform(const char *text)
{
    Eina_Bool dir_break;
    char **parts;
    Eina_Strbuf *buf, *dir;
    int c = 0;
    const char *result;

    //split for /
    parts = eina_str_split(text, SEP, 0);

    //init result
    buf = eina_strbuf_new();
    dir = eina_strbuf_new();
    dir_break = EINA_FALSE;

    for(c = 0;parts[c]; c++)
      {
         char part[PATH_MAX];
         const char *dirr;

         if (*parts[c] == '\0')
           continue;

         //update durrect dir
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
   return result;
}

static void
_markup_filter(void *data EINA_UNUSED, Evas_Object *obj, char ** text)
{
   char *n = *text;
   int c = 0;
   Eina_Strbuf *buf;
   const char *existsing;

   existsing = elm_object_text_get(obj);

   buf = eina_strbuf_new();

   while(n[c] != '\0')
     {
        if (n[c] == '/')
          {
             char tmp;

             tmp = n[c];

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

   text = evas_object_data_get(data, "__orig_text");

   planed_changed = EINA_TRUE;
   elm_object_text_set(data, text);
   planed_changed = EINA_FALSE;

   free((char*)text);
   unfocus_barrier = EINA_TRUE;

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
    const char *text;


    if (!unfocus_barrier)
      return;

    unfocus_barrier = EINA_FALSE;
    //get the currect text
    text = elm_object_text_get(obj);

    //save the current state
    evas_object_data_set(obj, "__orig_text", strdup(text));

    planed_changed = EINA_TRUE;
    elm_object_text_set(obj, _path_transform(text));
    planed_changed = EINA_FALSE;
}

static Eina_Bool
_change_idle(void *data)
{
   struct tuple *h = data;

   planed_changed = EINA_TRUE;
   elm_object_text_set(h->obj, h->now);
   planed_changed = EINA_FALSE;

   free(h);

   return EINA_FALSE;
}

static void
_changed_cb(void *data EINA_UNUSED, Evas_Object *obj, void *event EINA_UNUSED)
{
   const char *text;
   const char *transform;

   if (planed_changed)
     return;
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
_anchor_clicked_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event)
{
   Elm_Entry_Anchor_Info *info = event;

   if (focus_idler)
     ecore_idler_del(focus_idler);

   change_path(info->name);
}

Evas_Object*
titlebar_add(Evas_Object *parent)
{
   Evas_Object *entry;

   planed_changed = EINA_FALSE;

   entry = elm_entry_add(parent);
   elm_entry_single_line_set(entry, EINA_TRUE);
   elm_entry_markup_filter_append(entry, _markup_filter, NULL);
   evas_object_smart_callback_add(entry, "focused", _focus_cb, NULL);
   evas_object_smart_callback_add(entry, "unfocused", _unfocus_cb, NULL);
   evas_object_smart_callback_add(entry, "changed", _changed_cb, NULL);
   evas_object_smart_callback_add(entry, "anchor,clicked", _anchor_clicked_cb, NULL);
   evas_object_show(entry);

   unfocus_barrier = EINA_FALSE;

   return entry;
}