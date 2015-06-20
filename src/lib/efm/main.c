#include "efm_priv.h"
#include <Efreet.h>
static int counter = 0;

int _efm_domain;

int
efm_init()
{
   eina_init();
   ecore_init();
   efreet_init();
   eio_init();
   if (counter > 0)
     goto inc;

   _efm_domain = eina_log_domain_register("efm", NULL);
   if (!_efm_domain)
     return 0;

   eina_log_domain_level_set("efm",EINA_LOG_LEVEL_DBG);

   if (!fm_monitor_init())
     {
        ERR("Failed to init resolve");
        counter --;
        return 0;
     }
   efm_file_init();
inc:
    counter ++;
    return counter;
}

void
efm_shutdown()
{
   counter --;

   if (counter > 0)
     return;

   fm_monitor_shutdown();

   efm_file_shutdown();

   eina_log_domain_unregister(_efm_domain);
   eio_shutdown();
   efreet_shutdown();
   ecore_shutdown();
   eina_shutdown();
}