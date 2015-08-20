#ifndef ELM_EXT_H
#define ELM_EXT_H

#include <Evas.h>
#include <Efm.h>

#ifdef EAPI
# undef EAPI
#endif

#define EAPI

#include "./file/elm_file_mimetype_cache.eo.h"
#include "./file/elm_file_icon.h"
#include "./file/elm_file_display.h"
#include "./file/elm_file_bookmarks.eo.h"
#include "./file/elm_file_selector.eo.h"
#include "./file/elm_file_preview.eo.h"

EAPI void elm_ext_init();

#endif