#ifndef _ui_h_
#define _ui_h_
#include <Eo.h>
#include <Elementary.h>

typedef struct
{
   Eo *main_win;
   Eo *current_app;
   Eo *search;
   Eo *desktop_list;
   Eo *asdefault;
   Eo *open;
} Open_With2_Main_Win_Widgets;


Open_With2_Main_Win_Widgets *open_with2_main_win_create(Eo *parent);

#endif
