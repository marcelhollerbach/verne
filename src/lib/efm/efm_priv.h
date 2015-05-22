#ifndef EIO_FM_PRIV_H
#define EIO_FM_PRIV_H

#include "Efm.h"

#include <Eio.h>
#include <Ecore.h>
#include <Ecore_File.h>
#include <Efreet_Mime.h>

struct _EFM_Mount_Fs{
   const char *name;
   const char *script_path;
   const char *description;
};

typedef enum {
    ADD,DEL,ERROR,
} Fm_Action;

typedef void (*Fm_File_Action)(void *data, Efm_Monitor *mon, const char *file, Fm_Action add);

extern int _log_domain;

#define CRIT(...)     EINA_LOG_DOM_CRIT(_log_domain, __VA_ARGS__)
#define ERR(...)      EINA_LOG_DOM_ERR(_log_domain, __VA_ARGS__)
#define WRN(...)      EINA_LOG_DOM_WARN(_log_domain, __VA_ARGS__)
#define INF(...)      EINA_LOG_DOM_INFO(_log_domain, __VA_ARGS__)
#define DBG(...)      EINA_LOG_DOM_DBG(_log_domain, __VA_ARGS__)

int fm_monitor_init();
void fm_monitor_add(Efm_Monitor *mon, Eio_Monitor *monitor, Fm_File_Action action);
void fm_monitor_del(Efm_Monitor *mon, Eio_Monitor *monitor);
void fm_monitor_shutdown();
#endif