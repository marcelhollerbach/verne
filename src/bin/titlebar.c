#include "main.h"

#define SEP "<item relsize=15x15 vsize=full href=path/separator></item>"

struct tuple {
  const char *now;
  Evas_Object *obj;
};

typedef struct {
  Eina_List *parts;
} Titlebar_Content;

static Evas_Object *_entry;

static void
titlebar_whipe(Titlebar_Content *content)
{
   Eina_Strbuf *buf;

   EINA_LIST_FREE(content->parts, buf)
     {
        eina_strbuf_free(buf);
     }
}

static void
titlebar_content_add(Titlebar_Content *content, unsigned int pos, const char *symbol)
{
   Eina_List *current_node;
   Eina_Strbuf *buf = NULL;
   int jump_in;

   if (!symbol) return;

   current_node = content->parts;
   if (!current_node)
     {
        buf = eina_strbuf_new();
        content->parts = eina_list_append(content->parts, buf);
        jump_in = 0;
     }
   else
     {
        jump_in = 0;
        for(unsigned int i = 0; i <= pos;i ++)
          {
             buf = eina_list_data_get(current_node);

             if (i + eina_strbuf_length_get(buf) > pos)
               {
                  //here we drop in our symbols
                  jump_in = pos - i;
                  break;
               }

             i += eina_strbuf_length_get(buf);

             current_node = eina_list_next(current_node);
          }
     }

   if (jump_in == 0)
     eina_strbuf_append(buf, symbol);
   else
     eina_strbuf_insert(buf, symbol, jump_in);

   //simplty split buf at the "/" and insert behind the buf
   {
      char **splits;

      splits = eina_str_split(eina_strbuf_string_get(buf), "/", 0);

      if (splits)
        {
           for(int i = 0; splits[i]; i ++)
             {
                Eina_Strbuf *new;

                new = eina_strbuf_new();
                eina_strbuf_append(new, splits[i]);

                content->parts = eina_list_prepend_relative(content->parts, new, buf);
             }
           content->parts = eina_list_remove(content->parts, buf);
        }
   }
}

static void
titlebar_del(Titlebar_Content *content, unsigned int pos, int length)
{
   Eina_Strbuf *buf;
   Eina_Strbuf *tmp;
   Eina_List *next_node, *last = NULL;
   int start_rest = -1;

   next_node = content->parts;


   tmp = eina_strbuf_new();

   for(unsigned int i = 0; i <= pos+ length;i ++)
     {
        Eina_List *prev;
        int i_tmp;
        buf = eina_list_data_get(next_node);
        i_tmp = i + eina_strbuf_length_get(buf);
        prev = next_node;
        next_node = eina_list_next(next_node);

        if (i >= pos || i + eina_strbuf_length_get(buf) >= pos)
          {
             if (start_rest != -1)
               {
                  eina_strbuf_append(tmp, "/");
               }
             eina_strbuf_append_buffer(tmp, buf);
             if (start_rest == -1)
               {
                  start_rest = pos - i;
                  last = eina_list_prev(prev);
               }

             content->parts = eina_list_remove(content->parts, buf);
             eina_strbuf_free(buf);
          }



        i = i_tmp;
     }

   eina_strbuf_remove(tmp, start_rest, start_rest + length);

   content->parts = eina_list_append_relative_list(content->parts, tmp, last);


}

#if 0
static void
_debug(Titlebar_Content *content)
{
   Eina_List *node;
   Eina_Strbuf *buf;

   EINA_LIST_FOREACH(content->parts, node, buf)
     {
        printf("DEBUG >%s<\n", eina_strbuf_string_get(buf));
     }
}
#endif

static Eina_Strbuf*
titlebar_anchor_get(Titlebar_Content *content)
{
   Eina_List *node;
   Eina_Strbuf *buf;
   Eina_Strbuf *result;
   Eina_Strbuf *link;
   Eina_Bool dir_break = EINA_FALSE;

   result = eina_strbuf_new();
   link = eina_strbuf_new();

   EINA_LIST_FOREACH(content->parts, node, buf)
     {

        eina_strbuf_append_buffer(link, buf);
        if (eina_list_next(node))
          eina_strbuf_append_char(link, '/');

        if (!ecore_file_exists(eina_strbuf_string_get(link)))
          {
             dir_break = EINA_TRUE;
          }

        if (eina_strbuf_length_get(buf))
          {
             if (!dir_break)
               {
                  eina_strbuf_append_printf(result,
                    "<a href=%s>%s</a>",
                    eina_strbuf_string_get(link),
                    eina_strbuf_string_get(buf));
               }
             else
               {
                  eina_strbuf_append_printf(result,
                    "%s",
                    eina_strbuf_string_get(buf));
               }

          }

        if (eina_list_next(node))
          eina_strbuf_append(result, SEP);

     }

   return result;
}

static Eina_Strbuf*
titlebar_link_get(Titlebar_Content *content)
{
   Eina_List *node;
   Eina_Strbuf *buf;
   Eina_Strbuf *result;

   result = eina_strbuf_new();

   EINA_LIST_FOREACH(content->parts, node, buf)
     {

        eina_strbuf_append_buffer(result, buf);
        if (eina_list_next(node))
          eina_strbuf_append(result, "/");
     }

   return result;
}

static void
_content_refresh(Evas_Object *entry)
{
   Titlebar_Content *content = evas_object_data_get(entry, "__content");
   Eina_Strbuf *buf;
   int cursor_pos = 0;

   buf = titlebar_anchor_get(content);
   cursor_pos = elm_entry_cursor_pos_get(entry);
   elm_object_text_set(entry, eina_strbuf_string_get(buf));
   elm_entry_cursor_pos_set(entry, cursor_pos);
   eina_strbuf_string_free(buf);
}

static void
_link_flush(Evas_Object *entry)
{
   Titlebar_Content *content = evas_object_data_get(entry, "__content");
   Eina_Strbuf *link;
   Efm_File *file = NULL;

   link = titlebar_link_get(content);
   file = efm_file_get(EFM_CLASS, eina_strbuf_string_get(link));

   if (file)
     {
        elm_file_selector_file_set(selector, file);
     }

   eina_strbuf_free(link);
}

static void
_unfocus_cb(void *data EINA_UNUSED, Evas_Object *obj, void *event EINA_UNUSED)
{
   _link_flush(obj);
}

static void
_changed_cb(void *data EINA_UNUSED, Evas_Object *obj, void *event)
{
   Elm_Entry_Change_Info *info = event;
   Titlebar_Content *content = evas_object_data_get(obj, "__content");

   if (info->insert)
     {
        titlebar_content_add(content, info->change.insert.pos, info->change.insert.content);
     }
   else
     {
        int pos = -1, length = -1;
        if (info->change.del.start < info->change.del.end)
          {
             pos = info->change.del.start;
             length = info->change.del.end - pos;
          }
        else
          {
             pos = info->change.del.end;
             length = info->change.del.start - pos;
          }

        titlebar_del(content,
          pos, length);
     }

   _content_refresh(obj);
}

static void
_anchor_clicked_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event)
{
   Elm_Entry_Anchor_Info *info = event;
   Efm_File *file;

   elm_object_focus_set(obj, EINA_FALSE);
   elm_object_text_set(obj, NULL);
   elm_entry_entry_append(obj, info->name);
   file = efm_file_get(EFM_CLASS, info->name);
   elm_file_selector_file_set(selector, file);
}

void
titlebar_init(void)
{
   Titlebar_Content *content = calloc(1, sizeof(Titlebar_Content));

   _entry = elm_entry_add(layout);
   evas_object_data_set(_entry, "__content", content);
   elm_entry_single_line_set(_entry, EINA_TRUE);
   evas_object_smart_callback_add(_entry, "unfocused", _unfocus_cb, NULL);
   evas_object_smart_callback_add(_entry, "changed,user", _changed_cb, NULL);
   evas_object_smart_callback_add(_entry, "anchor,clicked", _anchor_clicked_cb, NULL);
   evas_object_show(_entry);

   elm_object_part_content_set(layout, "verne.textbar", _entry);
}

void
titlebar_path_set(const char *path)
{
   Titlebar_Content *content = evas_object_data_get(_entry, "__content");

   titlebar_whipe(content);

   titlebar_content_add(content, elm_entry_cursor_pos_get(_entry), path);

   _content_refresh(_entry);
}