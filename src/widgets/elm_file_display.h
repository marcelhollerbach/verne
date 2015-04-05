#ifndef ELM_FILE_DISPLAY_H
#define ELM_FILE_DISPLAY_H

#include <Elementary.h>

#include <Efm.h>

typedef enum{
   SORT_TYPE_SIZE = 0,
   SORT_TYPE_DATE = 1,
   SORT_TYPE_NAME = 2,
   SORT_TYPE_EXTENSION = 3,
} Elm_File_Display_Sort_Type;

typedef enum{
  FOLDER_FIRST, FOLDER_LAST, NOTHING
} Elm_File_Display_Folder_Placement;

typedef struct{
   Evas_Object *menu;
   EFM_File *file;
} Elm_File_Display_Menu_Hook_Event;

#ifdef EFL_EO_API_SUPPORT
#   include "elm_file_display.eo.h"
#else
#   include "elm_file_display.eo.legacy.h"
#endif

#endif