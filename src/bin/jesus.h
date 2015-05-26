#ifndef JESUS_H
#define JESUS_H

#define EFL_BETA_API_SUPPORT
#define EFL_EO_API_SUPPORT

#include <Efm.h>
#include <Elementary.h>
#include <Elementary_Ext.h>

Evas_Object *titlebar_add(Evas_Object *parent);
void display_file_set(const char *path);
#endif
