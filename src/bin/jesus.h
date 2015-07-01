#ifndef JESUS_H
#define JESUS_H

#define EFL_BETA_API_SUPPORT
#define EFL_EO_API_SUPPORT

#include <Efm.h>
#include <Eio.h>
#include <Elementary.h>
#include <Elementary_Ext.h>
#include "ui.h"

typedef enum
{
    MOVE, COPY
} Clipboard_Mode;

typedef struct {
    Eina_Hash *mime_type_open;
} Jesus_Config;

typedef void (*Cmd_Choosen)(Efm_File *file, const char *cmd);

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
void history_init(void);

void exec_ui_open_with(Efm_File *file, Cmd_Choosen choosen);
void exec_run(const char *cmd, Efm_File *f);

void clipboard_set(Clipboard_Mode m, Eina_List *list);
void clipboard_init(void);
Eina_Bool clipboard_something_in(void);
void clipboard_shutdown(void);
void clipboard_paste(const char *paste);

void fs_operations_init(void);
void fs_operations_move(Eina_List *files, const char *goal);
void fs_operations_copy(Eina_List *files, const char *goal);
void fs_operations_delete(Eina_List *files);

void preview_copy(void);
void preview_move(void);
void preview_remove(void);
void preview_paste(void);
#endif
