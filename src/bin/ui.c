#ifndef EFL_BETA_API_SUPPORT
#define EFL_BETA_API_SUPPORT
#endif
#ifndef EFL_EO_API_SUPPORT
#define EFL_EO_API_SUPPORT
#endif
#ifndef ELM_INTERNAL_API_ARGESFSDFEFC
#define ELM_INTERNAL_API_ARGESFSDFEFC
#endif
#include <Elementary.h>
#include "ui.h"
#if 0
static void
_pubs_free_cb(void *data, const Efl_Event *event EINA_UNUSED)
{
   free(data);
}

Open_With2_Main_Win_Widgets *
open_with2_main_win_create(Eo *__main_parent)
{
   Open_With2_Main_Win_Widgets *pub_widgets = calloc(1, sizeof(*pub_widgets));

   Eo *main_win;
   Eo *elm_win1_main_menu;
   Eo *elm_box1;
   Eo *elm_genlist1;
   Eo *elm_box2;
   Eo *open;
   Eo *asdefault;
   Eo *elm_box3;
   Eo *elm_label1;
   Eo *current_app;
   Eo *search;
   Eo *elm_icon1;
   Eo *elm_bg1;


   main_win = efl_add(EFL_UI_WIN_CLASS, __main_parent, efl_ui_win_type_set(efl_added, EFL_UI_WIN_BASIC));
   pub_widgets->main_win = main_win;
   efl_gfx_size_hint_weight_set (main_win, 1.000000, 1.000000);
   efl_gfx_size_set (main_win, 454, 379);
   efl_ui_win_autodel_set (main_win, EINA_TRUE);
   elm_win_title_set(main_win, "Open with");
   elm_win1_main_menu = efl_add(ELM_MENU_CLASS, main_win);

   elm_box1 = elm_box_add(main_win);
   efl_gfx_size_hint_weight_set (elm_box1, 1.000000, 1.000000);
   efl_gfx_visible_set (elm_box1, EINA_TRUE);
   efl_gfx_size_hint_align_set (elm_box1, -1.000000, -1.000000);
   efl_gfx_position_set (elm_box1, 72, 86);

   elm_genlist1 = efl_add(ELM_GENLIST_CLASS, elm_box1);
   pub_widgets->elm_genlist1 = elm_genlist1;
   efl_gfx_visible_set (elm_genlist1, EINA_TRUE);
   efl_gfx_size_set (elm_genlist1, 70, 60);
   efl_gfx_size_hint_weight_set (elm_genlist1, 1.000000, 1.000000);
   efl_gfx_size_hint_align_set (elm_genlist1, -1.000000, -1.000000);


   elm_box2 = elm_box_add(elm_box1);
   efl_gfx_visible_set (elm_box2, EINA_TRUE);
   elm_box_horizontal_set (elm_box2, EINA_TRUE);
   efl_gfx_size_hint_align_set (elm_box2, -1.000000, -1.000000);
   efl_gfx_size_hint_weight_set (elm_box2, 1.000000, 0.000000);

   open = efl_add(ELM_BUTTON_CLASS, elm_box2);
   pub_widgets->open = open;
   efl_gfx_size_hint_weight_set (open, 1.000000, 1.000000);
   efl_gfx_visible_set (open, EINA_TRUE);
   efl_gfx_size_set (open, 73, 30);
   efl_gfx_size_hint_align_set (open, -1.000000, -1.000000);
   elm_object_part_text_set (open, NULL, "Open");
   asdefault = efl_add(ELM_BUTTON_CLASS, elm_box2);
   pub_widgets->asdefault = asdefault;
   efl_gfx_size_hint_weight_set (asdefault, 1.000000, 1.000000);
   efl_gfx_visible_set (asdefault, EINA_TRUE);
   efl_gfx_size_set (asdefault, 73, 30);
   efl_gfx_size_hint_align_set (asdefault, -1.000000, -1.000000);
   elm_object_part_text_set (asdefault, NULL, "Set as default");
   elm_box_pack_end (elm_box2, asdefault);
   elm_box_pack_end (elm_box2, open);

   elm_box3 = elm_box_add(elm_box1);
   efl_gfx_visible_set (elm_box3, EINA_TRUE);
   efl_gfx_size_hint_align_set (elm_box3, -1.000000, 0.000000);
   elm_box_horizontal_set (elm_box3, EINA_TRUE);
   efl_gfx_size_hint_weight_set (elm_box3, 0.000000, 0.000000);

   elm_label1 = efl_add(ELM_LABEL_CLASS, elm_box3);
   efl_gfx_visible_set (elm_label1, EINA_TRUE);
   efl_gfx_size_set (elm_label1, 60, 30);
   efl_gfx_size_hint_weight_set (elm_label1, 1.000000, 0.000000);
   efl_gfx_size_hint_align_set (elm_label1, -1.000000, 0.500000);
   elm_object_part_text_set(elm_label1, NULL, "Current default:");

   current_app = efl_add(ELM_LABEL_CLASS, elm_box3);
   pub_widgets->current_app = current_app;
   efl_gfx_size_hint_weight_set (current_app, 1.000000, 1.000000);
   efl_gfx_visible_set (current_app, EINA_TRUE);
   elm_object_part_text_set (current_app, NULL, "Label");
   efl_gfx_size_set (current_app, 60, 30);
   elm_box_pack_end (elm_box3, elm_label1);
   elm_box_pack_end (elm_box3, current_app);

   search = efl_add(ELM_ENTRY_CLASS, elm_box1);
   pub_widgets->search = search;
   efl_gfx_visible_set (search, EINA_TRUE);
   efl_gfx_size_set (search, 60, 30);
   efl_gfx_size_hint_weight_set (search, 1.000000, 0.000000);
   efl_gfx_size_hint_align_set (search, -1.000000, 0.000000);
   elm_entry_icon_visible_set (search, EINA_FALSE);
   elm_entry_scrollable_set (search, EINA_TRUE);
   elm_entry_single_line_set (search, EINA_TRUE);
   elm_object_part_text_set (search, "guide", "search");
   elm_box_pack_end (elm_box1, elm_box3);
   elm_box_pack_end (elm_box1, search);
   elm_box_pack_end (elm_box1, elm_genlist1);
   elm_box_pack_end (elm_box1, elm_box2);

   elm_icon1 = elm_icon_add(search);
   efl_gfx_size_hint_weight_set (elm_icon1, 1.000000, 1.000000);
   efl_gfx_visible_set (elm_icon1, EINA_TRUE);
   efl_gfx_size_set (elm_icon1, 40, 40);
   elm_icon_standard_set (elm_icon1, "search");
   elm_object_part_content_set(search, "icon", elm_icon1);

   elm_bg1 = efl_add(ELM_BG_CLASS, main_win);
   efl_gfx_size_hint_weight_set (elm_bg1, 1.000000, 1.000000);
   efl_gfx_visible_set (elm_bg1, EINA_TRUE);
   elm_win_resize_object_add (main_win, elm_bg1);
   elm_win_resize_object_add (main_win, elm_box1);
   efl_gfx_visible_set (main_win, EINA_TRUE);
   efl_event_callback_add (main_win, EFL_EVENT_DEL, _pubs_free_cb, pub_widgets);

   return pub_widgets;
}

#endif
Open_With2_Main_Win_Widgets *
open_with2_main_win_create(Eo *__main_parent)
{
   printf("NOP\n");
   return NULL;
}
