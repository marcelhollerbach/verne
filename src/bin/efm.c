#include <Efm.h>

#define EFL_BETA_API_SUPPORT
#define EFL_EO_API_SUPPORT

#include <Elementary.h>
#include <Elementary_Ext.h>

/*
 * Seperator used to replace '/' characters
 */
#define SEP "<item relsize=15x15 vsize=full href=path/separator></item>"

/*
 * the text shown in the about text
 */
#define ABOUT_TEXT \
"<align=0.5><h2>EFM is a elementary based filebrowser</h2><br>\
<align=0.5>writen by:<br>\
<align=0.5>- bu5hm4n"

typedef struct {
   Evas_Object *label;
   Evas_Object *entry;
   Evas_Object *content;
   Evas_Object *back;
   Evas_Object *forb;
   const char *path;
   Eina_List *path_stack;
   unsigned int ptr;
} FM_Tab;

/*
 * The tab panels
 */
static Evas_Object *pane;
static Eina_List *list;

static void _open_tab(const char *path);
static void _open_about_tab(void);
static void _anchors_do(Evas_Object *obj, const char *path);
static void _anchors_undo(Evas_Object *obj, const char *path);

static void
_new_tab_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Efm_File *file;
   const char *path;

   file = data;
   path = efm_file_path_get(file);

   _open_tab(path);
}

static void
_about_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   _open_about_tab();
}

static Eina_Bool
_fm_tab_menu_hook_start(void *data EINA_UNUSED, Eo *obj EINA_UNUSED, const Eo_Event_Description *desc EINA_UNUSED, void *event)
{
   Elm_File_Selector_Menu_Hook_Event *ev = event;

   elm_menu_item_add(ev->menu, NULL, NULL, "Open in Tab",
                     _new_tab_cb, ev->file);
   return EINA_TRUE;
}

static void
_dir_changed(FM_Tab *tab, const char *dir)
{
   eina_stringshare_del(tab->path);
   tab->path = eina_stringshare_add(dir);

   _anchors_do(tab->entry, tab->path);
   elm_object_text_set(tab->label, tab->path);
}

static void
_fm_tab_stack_push(FM_Tab *tab)
{
   if (eina_list_count(tab->path_stack) - 1 > tab->ptr)
     {
        unsigned int i = tab->ptr;
        // unstack
        for (i = eina_list_count(tab->path_stack) - 1; i > tab->ptr; i--)
          {
             const char *path;

             path = eina_list_data_get(eina_list_last(tab->path_stack));
             tab->path_stack = eina_list_remove_list(tab->path_stack, eina_list_last(tab->path_stack));
             eina_stringshare_del(path);
          }
     }

   eina_stringshare_ref(tab->path);
   tab->path_stack = eina_list_append(tab->path_stack, tab->path);
   tab->ptr = eina_list_count(tab->path_stack) - 1;
#if 0
   {
     Eina_List *node;
     const char *n;

     printf("DUMP \n");

     EINA_LIST_FOREACH(tab->path_stack, node, n)
       {
          printf("%s\n", n);
       }
   }
#endif
}

static void
_fm_tab_up(void *data, Evas_Object *obj EINA_UNUSED, void *event EINA_UNUSED)
{
   FM_Tab *tab;
   const char *path;

   tab = data;

   path = ecore_file_dir_get(tab->path);
   if (!strcmp(path, tab->path))
     return;
   _dir_changed(tab, path);
   _fm_tab_stack_push(tab);
   eo_do(tab->content, efl_file_set(path, NULL));
   free((void*)path);
}

static void
_fm_tab_back(void *data, Evas_Object *obj EINA_UNUSED, void *event EINA_UNUSED)
{
   FM_Tab *tab;
   const char *path;

   tab = data;
   if (tab->ptr == 0)
     return;

   tab->ptr --;

   path = eina_list_nth(tab->path_stack, tab->ptr);
   _dir_changed(tab, path);
   eo_do(tab->content, efl_file_set(path, NULL));
}

static void
_fm_tab_for(void *data, Evas_Object *obj EINA_UNUSED, void *event EINA_UNUSED)
{
   FM_Tab *tab;
   const char *path;

   tab = data;
   if (eina_list_count(tab->path_stack) -1 == tab->ptr)
     return;

   tab->ptr ++;

   path = eina_list_nth(tab->path_stack, tab->ptr);
   eo_do(tab->content, efl_file_set(path, NULL));
   _dir_changed(tab, path);
}

static Eina_Bool
_fm_tab_dir_changed(void *data EINA_UNUSED, Eo *obj EINA_UNUSED, const Eo_Event_Description *desc EINA_UNUSED, void *event)
{
   FM_Tab *tab;
   const char *dir = event;

   tab = data;

   _dir_changed(tab, dir);

   _fm_tab_stack_push(tab);

   return EINA_FALSE;
}

static void
_open_about_tab(void)
{
   Evas_Object *a, *c;

   a = elm_label_add(pane);
   elm_object_text_set(a, "About");

   c = elm_entry_add(pane);
   elm_entry_entry_set(c, ABOUT_TEXT);

   eo_do(pane, elm_obj_tab_pane_item_add(a, c));
}

static void
_anchors_do(Evas_Object *obj, const char *path)
{
   char **tok, buf[PATH_MAX * 3];
   int i, j;

   if (!path)
     return;

   buf[0] = '\0';
   tok = eina_str_split(path, "/", 0);

   eina_strlcat(buf, "<a href='/'>root</a>", sizeof(buf));
   for (i = 0; tok[i]; i++)
     {
        if ((!tok[i]) || (!tok[i][0])) continue;
        eina_strlcat(buf, SEP, sizeof(buf));
        eina_strlcat(buf, "<a href=", sizeof(buf));
        for (j = 0; j <= i; j++)
          {
             if (strlen(tok[j]) < 1) continue;
             eina_strlcat(buf, "/", sizeof(buf));
             eina_strlcat(buf, tok[j], sizeof(buf));
          }
        eina_strlcat(buf, ">", sizeof(buf));
        eina_strlcat(buf, tok[i], sizeof(buf));
        eina_strlcat(buf, "</a>", sizeof(buf));
     }
   free(tok[0]);
   free(tok);

   elm_object_text_set(obj, buf);
}

static void
_anchors_undo(Evas_Object *obj, const char *path)
{
   if (!path)
     return;

   elm_object_text_set(obj, path);
}

static void
_fm_tab_entry_focused(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   FM_Tab *tab;

   tab = data;
   _anchors_undo(obj, tab->path);
}

static void
_fm_tab_entry_unfocused(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   FM_Tab *tab;

   tab = data;
   _anchors_do(obj, tab->path);
}


static Eina_Bool
_fm_tab_item_open(void *data EINA_UNUSED, Eo *obj EINA_UNUSED, const Eo_Event_Description *desc EINA_UNUSED, void *event)
{
   const char *file = event;
   char cmd[PATH_MAX];

   snprintf(cmd, sizeof(cmd), "xdg-open \"%s\"", file);
   printf("Executing %s\n", cmd);

   ecore_exe_run(cmd, NULL);

   return EINA_FALSE;
}

static void
_open_tab(const char *path)
{
   Evas_Object *lb1, *lb2, *o, *o2, *bx,*content;
   FM_Tab *tab;

   tab = calloc(1, sizeof(FM_Tab));
   tab->path = eina_stringshare_add(path);

   list = eina_list_append(list, tab);

   eina_stringshare_ref(tab->path);
   tab->path_stack = eina_list_append(tab->path_stack, tab->path);
   tab->ptr = eina_list_count(tab->path_stack) - 1;

   bx = elm_box_add(pane);
   evas_object_size_hint_align_set(bx, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

   lb1 = elm_box_add(pane);
   evas_object_size_hint_align_set(lb1, EVAS_HINT_FILL, 0);
   evas_object_size_hint_weight_set(lb1, EVAS_HINT_EXPAND, 0);
   elm_box_horizontal_set(lb1, EINA_TRUE);
   elm_box_pack_end(bx, lb1);
   evas_object_show(lb1);

   o = elm_icon_add(pane);
   evas_object_size_hint_min_set(o, 12, 12);
   elm_icon_standard_set(o, "arrow-left");
   evas_object_show(o);

   tab->back = o2 = elm_button_add(pane);
   elm_object_part_content_set(o2, "icon", o);
   evas_object_smart_callback_add(o2, "clicked", _fm_tab_back, tab);
   elm_box_pack_end(lb1, o2);
   evas_object_show(o2);

   o = elm_icon_add(pane);
   evas_object_size_hint_min_set(o, 12, 12);
   elm_icon_standard_set(o, "arrow-right");
   evas_object_show(o);

   tab->forb = o2 = elm_button_add(pane);
   elm_object_part_content_set(o2, "icon", o);
   evas_object_smart_callback_add(o2, "clicked", _fm_tab_for, tab);
   elm_box_pack_end(lb1, o2);
   evas_object_show(o2);

   o = elm_icon_add(pane);
   evas_object_size_hint_min_set(o, 12, 12);
   elm_icon_standard_set(o, "arrow-up");
   evas_object_show(o);

   o2 = elm_button_add(pane);
   elm_object_part_content_set(o2, "icon", o);
   evas_object_smart_callback_add(o2, "clicked", _fm_tab_up, tab);
   elm_box_pack_end(lb1, o2);
   evas_object_show(o2);

   tab->entry = o = elm_entry_add(pane);
   evas_object_smart_callback_add(o, "focused", _fm_tab_entry_focused, tab);
   evas_object_smart_callback_add(o, "unfocused", _fm_tab_entry_unfocused, tab);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, 0);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, 0);
   elm_entry_single_line_set(o, EINA_TRUE);
   _anchors_do(o, tab->path);
   elm_box_pack_end(lb1, o);
   evas_object_show(o);

   o = elm_icon_add(pane);
   evas_object_size_hint_min_set(o, 12, 12);
   elm_icon_standard_set(o, "search");
   evas_object_show(o);

   o2 = elm_button_add(pane);
   elm_object_part_content_set(o2, "icon", o);
   evas_object_smart_callback_add(o2, "clicked", _about_cb, NULL);
   elm_box_pack_end(lb1, o2);
   evas_object_show(o2);

   tab->content = content = efl_add(ELM_FILE_DISPLAY_CLASS, pane);
   evas_object_size_hint_align_set(content, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(content, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   eo_do(content,
         efl_event_callback_add(&_ELM_FILE_DISPLAY_EVENT_PATH_CHANGED,
                               _fm_tab_dir_changed, tab);
         efl_event_callback_add(&_ELM_FILE_DISPLAY_EVENT_ITEM_CHOOSEN,
                               _fm_tab_item_open, NULL);
         efl_event_callback_add(&_ELM_FILE_DISPLAY_EVENT_HOOK_MENU_SELECTOR_START,
                               _fm_tab_menu_hook_start, NULL);
         efl_file_set(path, NULL);
     );
   evas_object_show(content);

   elm_box_pack_end(bx, content);
   efl_unref(content);

   tab->label = lb2 = elm_label_add(pane);
   elm_object_text_set(lb2, tab->path);
   evas_object_show(lb2);

   eo_do(pane, elm_obj_tab_pane_item_add(lb2, bx));

}
static Eina_Bool
_item_add(void *data EINA_UNUSED, Eo *obj EINA_UNUSED, const Eo_Event_Description *desc EINA_UNUSED, void *event EINA_UNUSED)
{
   const char *home;
   if ((home = getenv("HOME")))
      _open_tab(home);
   return EINA_TRUE;
}

static void
on_done(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   // quit the mainloop (elm_run function will return)
   elm_exit();
}


EAPI_MAIN int
elm_main(int argc EINA_UNUSED, char **argv EINA_UNUSED)
{
   const char *home;
   Evas_Object *win, *bx;

   elm_ext_init();

   elm_need_ethumb();
   elm_need_efreet();

   win = elm_win_util_standard_add("efm", "efm - Kvasir");
   evas_object_smart_callback_add(win, "delete,request", on_done, NULL);

   bx = elm_box_add(win);
   evas_object_size_hint_align_set(bx, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bx);

   pane = efl_add(ELM_TAB_PANE_CLASS, win);
   evas_object_size_hint_align_set(pane, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(pane, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(pane);
      eo_do(pane,
     efl_event_callback_add(&_ELM_TAB_PANE_EVENT_ITEM_ADD,
                           _item_add, pane);
     );
   elm_box_pack_end(bx, pane);


   if ((home = getenv("HOME")))
     _open_tab(home);

   elm_win_resize_object_add(win, bx);
   evas_object_resize(win, 200,200);
   evas_object_show(win);
   efl_unref(pane);
   elm_run();
   return 0;
}
ELM_MAIN()
