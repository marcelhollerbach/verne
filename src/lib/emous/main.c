#include "emous_priv.h"

int ref = 0;
int _log_domain;

EAPI int
emous_init(void)
{
   ref ++;

   if (ref > 1)
     return ref;

   if (!eina_init())
     {
        printf("Eina init failed!\n");
        goto eina_err;
     }


   _log_domain = eina_log_domain_register("emous", NULL);
   if (!_log_domain)
     return 0;

   eina_log_domain_level_set("emous",EINA_LOG_LEVEL_DBG);

   if (!ecore_init())
     {
        ERR("Ecore init failed!");
        goto ecore_err;
     }
   if (!eio_init())
     {
        ERR("Eio init failed!");
        goto eio_err;
     }

   if (!mount_init())
     {
        ERR("Mount init failed!");
        goto mount_err;
     }

   _emous_mount_point_init();

   _emous_mm_init();

    return 1;
   mount_shutdown();
mount_err:
   eio_shutdown();
eio_err:
   ecore_shutdown();
ecore_err:
   eina_shutdown();
eina_err:
   ref = 0;
   return 0;
}

EAPI void
emous_shutdown(void)
{
   _emous_mount_point_shutdown();
   mount_shutdown();
   eio_shutdown();
   ecore_shutdown();
   eina_log_domain_unregister(_log_domain);
   eina_shutdown();
}

//The following init calls are just for the event cases!!!!!
EAPI int
emous_test_init(void)
{
   ref ++;

   if (ref > 1)
     return ref;

   if (!eina_init())
     {
        printf("Eina init failed!\n");
        goto eina_err;
     }


   _log_domain = eina_log_domain_register("emous", NULL);
   if (!_log_domain)
     return 0;

   eina_log_domain_level_set("emous",EINA_LOG_LEVEL_DBG);

   if (!ecore_init())
     {
        ERR("Ecore init failed!");
        goto ecore_err;
     }
   if (!eio_init())
     {
        ERR("Eio init failed!");
        goto eio_err;
     }

   if (!mount_init())
     {
        ERR("Mount init failed!");
        goto mount_err;
     }

   _emous_mount_point_init();

    return 1;
   mount_shutdown();
mount_err:
   eio_shutdown();
eio_err:
   ecore_shutdown();
ecore_err:
   eina_shutdown();
eina_err:
   ref = 0;
   return 0;
}

EAPI int
emous_test_init2(void)
{
   _emous_mm_init();
   return 1;
}
