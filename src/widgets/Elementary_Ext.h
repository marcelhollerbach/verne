#ifndef ELM_EXT_H
#define ELM_EXT_H

#include <Elementary.h>
#include <Efm.h>

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef EFL_ECORE_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif /* ! DLL_EXPORT */
# else
#  define EAPI __declspec(dllimport)
# endif /* ! EFL_ECORE_BUILD */
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EAPI __attribute__ ((visibility("default")))
#  else
#   define EAPI
#  endif
# else
#  define EAPI
# endif
#endif /* ! _WIN32 */

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