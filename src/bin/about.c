#include "main.h"
static void
_end(void *data, Evas_Object *obj EINA_UNUSED, const char *emission EINA_UNUSED, const char *emitter EINA_UNUSED)
{
   elm_win_resize_object_del(win, data);
   evas_object_del(data);
}

void
about_show(void)
{
   Evas_Object *about;
   Eina_Strbuf *buf;

   about = elm_layout_add(layout);
   evas_object_size_hint_align_set(about, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(about, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_layout_file_set(about, THEME_PATH"/default.edc.edj", "verne.about");
   elm_layout_signal_emit(about, "begin", "");
   elm_layout_signal_callback_add(about, "end", "", _end, about);
   evas_object_show(about);

   buf = eina_strbuf_new();

   eina_strbuf_append(buf, "<b>Verne</b><br>"
                           "A little filemanager based on elementary.<br>"
                           "Authors:<br><br>");

   eina_strbuf_append(buf, AUTHORS);

   elm_object_part_text_set(about, "verne.authorship", eina_strbuf_string_get(buf));

   eina_strbuf_free(buf);
   buf = NULL;

   elm_win_resize_object_add(win, about);
}