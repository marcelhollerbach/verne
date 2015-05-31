#ifndef ELM_FILE_DISPLAY_H
#define ELM_FILE_DISPLAY_H

#include <Elementary.h>

#include <Efm.h>
#include "elm_file_display_view.eo.h"
#include "elm_file_display_view_debug.eo.h"
#include "elm_file_display_view_grid.eo.h"

typedef struct{
   Evas_Object *menu;
   Efm_File *file;
} Elm_File_Display_Menu_Hook_Event;

#ifdef EFL_EO_API_SUPPORT
#   include "elm_file_display.eo.h"
#else
#   include "elm_file_display.eo.legacy.h"
#endif

#endif