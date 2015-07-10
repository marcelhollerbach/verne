#ifndef ELM_FILE_DISPLAY_H
#define ELM_FILE_DISPLAY_H

#include <Elementary.h>

#include <Efm.h>
#include "elm_file_display_view.eo.h"
#include "views/elm_file_display_view_debug.eo.h"
#include "views/elm_file_display_view_grid.eo.h"
#include "views/elm_file_display_view_list.eo.h"

#ifdef EFL_EO_API_SUPPORT
#   include "elm_file_display.eo.h"
#else
#   include "elm_file_display.eo.legacy.h"
#endif

#endif