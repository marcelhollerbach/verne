#ifndef ELM_FILE_DISPLAY_PRIV_H
#define ELM_FILE_DISPLAY_PRIV_H

#define EFL_BETA_API_SUPPORT
#define EFL_EO_API_SUPPORT

#include <Efm.h>
#include <Emous.h>

#include <Elementary.h>
#include <Ecore.h>
#include <Evas.h>
#include <Eio.h>
#include "Elementary_Ext.h"
#include "elm_file_display.h"
#include "elm_file_icon.eo.h"
#include "elm_file_display.eo.h"

#define FILESEP "file://"
#define FILESEP_LEN sizeof(FILESEP) - 1

//== view stuff ==

typedef struct
{
   int x,y,w,h;
   const char *path;
   const char *thumb_path;
   const char *thumb_group;
} File_Display_View_DND;

typedef struct
{
   /* should return a pointer or NULL if or if not a item is below this coord */
   EFM_File* (*item_get)(Evas_Object *wid, int x, int y);
   /* should select the items within this region */
   void (*items_select)(Evas_Object *wid, int x1, int y1, int w, int h);
   /* should return a list of item which currently are selected*/
   Eina_List* (*selections_get)(Evas_Object *wid);
   /* returns the view which gets displayed also the first call to the view*/
   Evas_Object* (*obj_get)(Evas_Object *par);
   /* this is called when the standart dir has changed */
   void (*dir_changed)(Evas_Object *wid, const char *dir);
} Elm_File_Display_View_Callbacks;

typedef struct
{
   const char *name;
   Elm_File_Display_View_Callbacks cb;
} Elm_File_Display_View;

typedef struct
{
   const char *name;
   //Elm_File_Display_View_Callbacks cb;
} Elm_File_Display_View_Type;

typedef struct
{
   Eo *obj;
   Eina_List *views; //< a list of the View Types
   Elm_File_Display_View *view; //< current type which is displayed

   const char* current_path;

   Evas_Object *bookmarks;
   Evas_Object *cached_view;

   struct event {
     Evas_Object *rect, *selection;
     Evas_Point mouse_down;
     Eina_Bool on_item;
   }event;

   Eina_List *selections;
   void *drag_data;
   Eina_Bool show_bookmark;
   Eina_Bool show_filepreview;
   Evas_Object *preview;
   Evas_Object *bookmark;
} Elm_File_Display_Data;

typedef struct
{
   Eina_List *bookmarks;
   char display_gtk;
   const char *viewname;
   int icon_size;
   char hidden_files;
   struct{
      int folder_placement;
      int type;
      char reverse;
   }sort;
} Config ;

extern Config *config;


extern Elm_File_Display_View_Callbacks debug;
extern Elm_File_Display_View_Callbacks grid;
extern Elm_File_Display_View_Callbacks list;
extern Elm_File_Display_View_Callbacks tree;

//== calls which are just calling the cb of the view, but secure

Evas_Object* icon_create(Evas_Object *par, EFM_File *file);

/*
 * helper functions to interact with the view
 */
void             view_call_dir_changed(Elm_File_Display_Data *pd, const char *path);
Evas_Object*     view_call_obj_get(Elm_File_Display_Data *pd, Evas_Object *par);
void             view_call_items_select(Elm_File_Display_Data *pd, int x1, int y1, int x2, int y2);
EFM_File* view_call_item_get(Elm_File_Display_Data *pd, int x, int y);
Eina_List *      view_call_selectes_get(Elm_File_Display_Data *pd);
Eina_Bool        view_call_selected_get(Elm_File_Display_Data *pd, int x, int y);

/*
 * Utilfunction to call when a item is selected by a view
 * If f is a dir the dir will be set,
 *
 * The events CHOOSEN and PATH_CHANGE will be called
 */
void util_item_selected(Evas_Object *w, EFM_File *f);

/*
 *
 */
void util_item_select(Evas_Object *w, EFM_File *f);

/*
 * Helper function to add the bookmarklist
 */
Evas_Object* bookmark_add(Evas_Object *w);


void filepreview_file_set(Evas_Object *w, EFM_File *file);
Evas_Object* filepreview_add(Evas_Object *w);


/*
 * Sort helper function
 */
int sort_func(const void *data1, const void *data2);

/*
 * Return a list of gtk bookmarks
 *
 * @return a list of stringshares
 */
Eina_List* util_bookmarks_load_gtk(void);


/*
 * Helper function to add/del bookmark,
 * save is done.
 */
void helper_bookmarks_add(const char *ptr);
void helper_bookmarks_del(const char *ptr);

/*
 * init the config sutff
 */
void config_init(void);

/*
 * Save the config
 */
void config_save(void);

/*
 * Load the config
 */
void config_read(void);

/*
 * Shutdown the system, the config will be cleared
 */
void config_shutdown(void);
#endif