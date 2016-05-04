#ifndef EFM_FM_H
#define EFM_FM_H

#include <Efl.h>
#include <Eina.h>
#include <Eo.h>

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

#include "efm.eot.h"
#include "efm_file.eo.h"
#include "efm_filter.eo.h"
#include "efm_monitor.eo.h"
#include "efm.eo.h"

#undef EAPI

#endif