#ifndef ELM_FILE_DISPLAY_PRIV_H
#define ELM_FILE_DISPLAY_PRIV_H

#ifndef EFM_EO_NEED
# include <Efm.h>
#endif

#define EFL_BETA_API_SUPPORT
#define EFL_EO_API_SUPPORT

#include <Emous.h>

#ifdef EFM_EO_NEED
# include <Efm.h>
#endif

#include <Elementary.h>
#include <Ecore.h>
#include <Evas.h>
#include <Eio.h>
#include "../elementary_ext_priv.h"
#include "elm_file_config.h"

#define FILESEP "file://"
#define FILESEP_LEN sizeof(FILESEP) - 1

extern Elm_File_MimeType_Cache *cache;

// == calls which are just calling the cb of the view, but secure

Evas_Object* icon_create(Evas_Object *par, Efm_File *file);

/*
 * Sort helper function
 */
int sort_func(const void *data1, const void *data2);


#endif