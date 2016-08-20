#ifndef EMOUS_PRIV_H
#define EMOUS_PRIV_H

#include "config.h"

#include <Eio.h>
#include <Ecore.h>
#include <Ecore_File.h>

#include "Emous.h"

#ifdef EAPI
# undef EAPI
#endif

#define EAPI

extern int _emous_domain;

#define CRIT(...)     EINA_LOG_DOM_CRIT(_emous_domain, __VA_ARGS__)
#define ERR(...)      EINA_LOG_DOM_ERR(_emous_domain, __VA_ARGS__)
#define WRN(...)      EINA_LOG_DOM_WARN(_emous_domain, __VA_ARGS__)
#define INF(...)      EINA_LOG_DOM_INFO(_emous_domain, __VA_ARGS__)
#define DBG(...)      EINA_LOG_DOM_DBG(_emous_domain, __VA_ARGS__)

#ifdef HAVE_MOUNT

void _emous_mm_init(void);
void _emous_mm_shutdown(void);

#endif

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