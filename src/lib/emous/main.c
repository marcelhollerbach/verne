#include "emous_priv.h"

int ref = 0;
int _log_domain;

static Eina_Array *modules = NULL;

static Eina_Bool
_module_load_cb(Eina_Module *m, void *data EINA_UNUSED)
{
   //we take everything!
   INF("Loading module %s", eina_module_file_get(m));
   return EINA_TRUE;
}

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

   //loading modules
   modules = eina_module_list_get(modules, EMOUS_MODULE_PATH, EINA_FALSE, _module_load_cb, NULL);

   //load all found modules
   eina_module_list_load(modules);

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
   ref --;
   if (ref  != 0)
     return;
   eina_module_list_unload(modules);
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
