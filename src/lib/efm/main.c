#include "efm_priv.h"
#include <Efreet.h>
static int counter = 0;

int _log_domain;

int
efm_init()
{
   eina_init();
   ecore_init();
   efreet_init();
   if (counter > 0)
     goto inc;

   _log_domain = eina_log_domain_register("efm", NULL);
   if (!_log_domain)
     return 0;

   eina_log_domain_level_set("efm",EINA_LOG_LEVEL_DBG);

   if (!fm_monitor_init())
     {
        ERR("Failed to init resolve");
        counter --;
        return 0;
     }

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

   eina_log_domain_unregister(_log_domain);

   efreet_shutdown();
   ecore_shutdown();
   eina_shutdown();
}