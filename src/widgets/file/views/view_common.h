#ifndef VIEW_COMMON_H
#define VIEW_COMMON_H

#include <Elementary.h>
#include <Efm.h>
typedef struct _View_Common View_Common;

typedef Elm_Object_Item* (*Item_Add_Cb)(View_Common *common, Efm_File *f);
typedef void (*Item_Del_Cb)(View_Common *common, Elm_Object_Item* it);
typedef void (*Error_Cb)(View_Common *common);
typedef void (*Item_Select_Cb)(View_Common *common, Elm_Object_Item *it, Eina_Bool direction);

struct _View_Common{
    Efm_Monitor *monitor;
    Eina_List *selection;
    Eina_Hash *files;
    Efm_Filter *f;
    Eo *obj;
    Item_Add_Cb add;
    Item_Del_Cb del;
    Error_Cb err;
    Item_Select_Cb sel;
};

void view_common_init(View_Common *common, Eo *obj, Item_Add_Cb add, Item_Del_Cb del, Error_Cb error, Item_Select_Cb sel);
void view_file_set(View_Common *common, Efm_File *file);
void view_filter_set(View_Common *common, Efm_Filter *filter);
void view_file_select(View_Common *common, Efm_File *f);
void view_file_unselect(View_Common *common, Efm_File *f);
Elm_Object_Item* view_search(View_Common *common, const char *search);
#endif