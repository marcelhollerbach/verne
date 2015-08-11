#ifndef ELM_FILE_DISPLAY_PRIV_H
#define ELM_FILE_DISPLAY_PRIV_H

#ifndef EFM_EO_NEED
# include <Efm.h>
#endif

#define EFL_BETA_API_SUPPORT
#define EFL_EO_API_SUPPORT

#include <Emous.h>

#ifdef EFM_EO_NEED
# include <Efm.h>
#endif

#include <Elementary.h>
#include <Ecore.h>
#include <Evas.h>
#include <Eio.h>
#include "../elementary_ext_priv.h"
#include "elm_file_config.h"

#define FILESEP "file://"
#define FILESEP_LEN sizeof(FILESEP) - 1

extern Elm_File_MimeType_Cache *cache;

typedef struct
{
   Eo *obj;

   const Eo_Class *view_klass;
   Evas_Object *cached_view;

   const char* current_path;

   Evas_Object *bookmarks;

   struct event {
     Evas_Object *rect, *selection;
     Evas_Point mouse_down;
     Eina_Bool on_item;
   }event;

   Eina_List *selection;
   void *drag_data;
   Eina_Bool show_bookmark;
   Eina_Bool show_filepreview;
   Evas_Object *preview;
   Evas_Object *bookmark;

   struct search {
     Ecore_Timer *clear_timer;
     Eina_Strbuf *searchpart;
     const char *pass;
   } search;

} Elm_File_Display_Data;

// == calls which are just calling the cb of the view, but secure

Evas_Object* icon_create(Evas_Object *par, Efm_File *file);

// event helper for selection
Eina_Bool _util_item_select_simple(void *data, Eo *obj, const Eo_Event_Description *desc, void *event);
Eina_Bool _util_item_select_choosen(void *data, Eo *obj, const Eo_Event_Description *desc, void *event);
/*
 * Helper function to add the bookmarklist
 */
Evas_Object* bookmark_add(Evas_Object *w);


void filepreview_file_set(Evas_Object *w, Efm_File *file);
Evas_Object* filepreview_add(Evas_Object *w);


/*
 * Sort helper function
 */
int sort_func(const void *data1, const void *data2);


#endif