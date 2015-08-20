#ifndef EIO_FM_PRIV_H
#define EIO_FM_PRIV_H

#include <Eio.h>
#include <Ecore.h>
#include <Ecore_File.h>
#include <Efreet_Mime.h>
#include "Efm.h"

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

typedef enum {
    ADD,ERROR,SELFDEL
} Fm_Action;

typedef void (*Fm_File_Action)(void *data, Efm_Monitor *mon, const char *file, Fm_Action add);

extern int _efm_domain;

#define CRIT(...)     EINA_LOG_DOM_CRIT(_efm_domain, __VA_ARGS__)
#define ERR(...)      EINA_LOG_DOM_ERR(_efm_domain, __VA_ARGS__)
#define WRN(...)      EINA_LOG_DOM_WARN(_efm_domain, __VA_ARGS__)
#define INF(...)      EINA_LOG_DOM_INFO(_efm_domain, __VA_ARGS__)
#define DBG(...)      EINA_LOG_DOM_DBG(_efm_domain, __VA_ARGS__)

int fm_monitor_init();
void fm_monitor_add(Efm_Monitor *mon, Eio_Monitor *monitor, Fm_File_Action action);
void fm_monitor_del(Efm_Monitor *mon, Eio_Monitor *monitor);
void fm_monitor_shutdown();

void efm_file_init(void);
void efm_file_shutdown(void);

#endif