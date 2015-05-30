#ifndef JESUS_H
#define JESUS_H

#define EFL_BETA_API_SUPPORT
#define EFL_EO_API_SUPPORT

#include <Efm.h>
#include <Elementary.h>
#include <Elementary_Ext.h>
#include "ui.h"


typedef struct {
    Eina_Hash *mime_type_open;

} Jesus_Config;

extern Jesus_Config *config;

//the general window
extern Evas_Object *win;

//the filepreview widget
extern Evas_Object *preview;

//the main layout of the window
extern Evas_Object *layout;

void config_init(void);
void config_flush(void);
void config_shutdown(void);

void titlebar_init(void);
void titlebar_path_set(const char *path);

void hooks_init(void);

void exec_run(const char *cmd, Efm_File *f);
#endif
