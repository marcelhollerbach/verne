#ifndef _ui_h_
#define _ui_h_
#include <Eo.h>
#include <Elementary.h>

typedef struct
{
   Eo *open_with;
   Eo *elm_genlist1;
   Eo *open;
   Eo *set_as_default;
} Executorui_Open_With_Widgets;


Executorui_Open_With_Widgets *executorui_open_with_create(Eo *parent);

#endif
