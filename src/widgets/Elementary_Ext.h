#ifndef ELM_EXT_H
#define ELM_EXT_H

#include "elm_file_display.h"
#include "elm_tab_pane.eo.h"

extern int log_domain;

#define CRI(...) EINA_LOG_DOM_CRIT(log_domain, __VA_ARGS__)
#define ERR(...) EINA_LOG_DOM_ERR(log_domain, __VA_ARGS__)
#define INF(...) EINA_LOG_DOM_INF(log_domain, __VA_ARGS__)
#define DBG(...) EINA_LOG_DOM_DBG(log_domain, __VA_ARGS__)

void elm_ext_init();

#endif