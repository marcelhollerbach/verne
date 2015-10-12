#include <Efreet.h>
#include "efm_priv.h"

typedef struct {

} Efm_Data;

typedef struct
{
  int counter;
  Eina_Hash *factory;
} Efm_Static_Data;

static Efm_Static_Data *sd;

int _efm_domain;


EOLIAN static int
_efm_init(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED)
{
   eina_init();
   ecore_init();
   efreet_init();
   eio_init();
   if (sd && sd->counter > 0)
     goto inc;
   _efm_domain = eina_log_domain_register("efm", NULL);
   if (!_efm_domain)
     return 0;

   sd = calloc(1, sizeof(Efm_Static_Data));
   if (!sd)
     return 0;

   sd->factory = eina_hash_string_small_new(NULL);
   if (!fm_monitor_init())
     {
        ERR("Failed to init resolve");
        free(sd);
        sd = NULL;
        return 0;
     }
   efm_file_init();
inc:
    sd->counter ++;
    return sd->counter;
}

EOLIAN static void
_efm_shutdown(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED)
{
   sd->counter --;

   if (sd->counter > 0)
     return;

   free(sd);
   sd = NULL;

   fm_monitor_shutdown();

   efm_file_shutdown();

   eina_log_domain_unregister(_efm_domain);
   eio_shutdown();
   efreet_shutdown();
   ecore_shutdown();
   eina_shutdown();
}

EOLIAN static Efm_File*
_efm_file_get(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED, const char *name)
{
   Efm_File *file;

   file = eina_hash_find(sd->factory, &name);

   if (file)
     return file;

   eo_do(EFM_FILE_CLASS, file = efm_file_generate(name));
   return file;
}

#include "efm.eo.x"