#ifndef _ui_h_
#define _ui_h_
#include <Eo.h>
#include <Elementary.h>

typedef struct
{
   Eo *main_win;
   Eo *elm_genlist1;
   Eo *open;
   Eo *asdefault;
   Eo *current_app;
   Eo *search;
} Open_With2_Main_Win_Widgets;


Open_With2_Main_Win_Widgets *open_with2_main_win_create(Eo *parent);

#endif
