#ifndef ELM_EXT_PRIV_H
#define ELM_EXT_PRIV_H

#include "config.h"

#include <Emous.h>
#include <Efl.h>
#include <Eio.h>
#include <Evas.h>
#include <Elementary.h>

#ifdef INEEDWIDGET
# define ELM_INTERNAL_API_ARGESFSDFEFC
# include <elm_widget.h>
#  undef ELM_INTERNAL_API_ARGESFSDFEFC
#endif

#include "./file/elm_file_config.h"
#include "Elementary_Ext.h"

#ifdef DEBUG
#  include "./file/views/elm_file_display_view_debug.eo.h"
#endif

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

extern int log_domain;

#define FILESEP "file://"
#define FILESEP_LEN sizeof(FILESEP) - 1

#define CRI(...) EINA_LOG_DOM_CRIT(log_domain, __VA_ARGS__)
#define ERR(...) EINA_LOG_DOM_ERR(log_domain, __VA_ARGS__)
#define INF(...) EINA_LOG_DOM_INFO(log_domain, __VA_ARGS__)
#define DBG(...) EINA_LOG_DOM_DBG(log_domain, __VA_ARGS__)

#endif
