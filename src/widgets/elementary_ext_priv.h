#ifndef ELM_EXT_PRIV_H
#define ELM_EXT_PRIV_H

#include "config.h"

#define EVAS_OBJECT_BETA

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

#ifndef EAPI
# define EAPI
#endif

extern int log_domain;

#define FILESEP "file://"
#define FILESEP_LEN sizeof(FILESEP) - 1

#define CRI(...) EINA_LOG_DOM_CRIT(log_domain, __VA_ARGS__)
#define ERR(...) EINA_LOG_DOM_ERR(log_domain, __VA_ARGS__)
#define INF(...) EINA_LOG_DOM_INFO(log_domain, __VA_ARGS__)
#define DBG(...) EINA_LOG_DOM_DBG(log_domain, __VA_ARGS__)

#endif
