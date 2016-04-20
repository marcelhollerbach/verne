#ifndef EMOUS_PRIV_H
#define EMOUS_PRIV_H

#include "config.h"
#include "Emous.h"

#include <Eio.h>
#include <Ecore.h>
#include <Ecore_File.h>

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

extern int _emous_domain;

#define CRIT(...)     EINA_LOG_DOM_CRIT(_emous_domain, __VA_ARGS__)
#define ERR(...)      EINA_LOG_DOM_ERR(_emous_domain, __VA_ARGS__)
#define WRN(...)      EINA_LOG_DOM_WARN(_emous_domain, __VA_ARGS__)
#define INF(...)      EINA_LOG_DOM_INFO(_emous_domain, __VA_ARGS__)
#define DBG(...)      EINA_LOG_DOM_DBG(_emous_domain, __VA_ARGS__)

void _emous_mm_init(void);
void _emous_mm_shutdown(void);

/*
 * Will add a mountpoint and emit the signal.
 * if type is known because of the initial parsing of the tree
 * this type is passed, if not a faked one is created.
 */
void _emous_mount_add(const char *type, const char *mount_point, const char *source);
/*
 * Deletes the mountpoint and emits the del event.
 * After event is done the Mount Point is freeed etc.
 */
void _emous_mount_del(const char *mount_point);

#endif