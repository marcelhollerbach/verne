#include "emous_priv.h"

typedef struct {

} Emous_Data;

static int mount_ref = 0;
static int lib_ref = 0;

int _emous_domain;

static Eina_Array *modules = NULL;

static Eina_Bool
_module_load_cb(Eina_Module *m, void *data EINA_UNUSED)
{
   //we take everything!
   DBG("Loading module %s", eina_module_file_get(m));
   return EINA_TRUE;
}

EOLIAN static int
_emous_init_lib(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED)
{
   lib_ref ++;

   if (lib_ref > 1)
     return lib_ref;

   if (!eina_init())
     {
        printf("Eina init failed!\n");
        goto eina_err;
     }

   _emous_domain = eina_log_domain_register("emous", NULL);
   if (!_emous_domain)
     return 0;

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

   return 1;
   eio_shutdown();
eio_err:
   ecore_shutdown();
ecore_err:
   eina_shutdown();
eina_err:
   lib_ref = 0;
   return 0;
}

EOLIAN static int
_emous_init(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED)
{
   if (mount_ref > 0)
     return ++mount_ref;
   //init the libs
   eo_do(EMOUS_CLASS, emous_init_lib());
   //loading modules
   modules = eina_module_list_get(modules, EMOUS_MODULE_PATH, EINA_FALSE, _module_load_cb, NULL);
   //load all found modules
   eina_module_list_load(modules);
   //init mountpoints
   //init this after loading modules
   //so every module is getting the initial mountpoints
   _emous_mm_init();
   mount_ref = 1;
   return mount_ref;
}

EOLIAN static void
_emous_shutdown(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED)
{
   //dec refs
   mount_ref --;
   if (mount_ref  != 0)
     return;

   //unload the modules
   eina_module_list_unload(modules);
   //shutdown mountpointslistening
   _emous_mm_shutdown();
   //shutdown the libs
   eo_do(EMOUS_CLASS, emous_shutdown_lib());

}
EOLIAN static void
_emous_shutdown_lib(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED)
{
   lib_ref --;
   if (lib_ref != 0)
     return;

   eio_shutdown();
   ecore_shutdown();
   eina_log_domain_unregister(_emous_domain);
   eina_shutdown();
}


#include "emous.eo.x"