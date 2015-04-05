#ifndef ELM_EXT_H
#define ELM_EXT_H

#include "elm_file_display.h"
#include "elm_tab_pane.eo.h"

static inline void
elm_ext_init()
{

   Elm_Theme *th = elm_theme_default_get();

   elm_theme_extension_add(th, THEME_PATH"file_icon.edc.edj");
   elm_theme_extension_add(th, THEME_PATH"tab.edc.edj");
   elm_theme_extension_add(th, THEME_PATH"file_display.edc.edj");

   elm_theme_flush(th);
}

void log_init(void);
void log_shutdown(void);

#endif