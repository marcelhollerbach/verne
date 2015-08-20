#include <Efreet.h>
#include "efm_priv.h"

static int counter = 0;

int _efm_domain;

EAPI int
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

EAPI void
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