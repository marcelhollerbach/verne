#ifndef ELM_EXT_H
#define ELM_EXT_H

#include <Elementary.h>
#include <Efm.h>

#ifdef EAPI
# undef EAPI
#endif

#define EAPI

#include "./file/elm_file_mimetype_cache.eo.h"
#include "./file/elm_file_icon.eo.h"
#include "./file/elm_file_bookmarks.eo.h"
#include "./file/elm_file_selector.eo.h"
#include "./file/elm_file_detail.eo.h"
#include "./file/elm_file_display.eo.h"
#include "./file/elm_file_view.eo.h"
#include "./file/views/elm_file_display_view_grid.eo.h"
#include "./file/views/elm_file_display_view_list.eo.h"

EAPI void elm_ext_init();

#undef EAPI

#endif