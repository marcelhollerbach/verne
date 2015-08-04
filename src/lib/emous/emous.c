#include "emous_priv.h"

typedef struct {

} Emous_Data;

const char* BYTE_ENDINGS[] = {"B", "KiB", "MiB", "GiB", "TiB", "PiB", "EiB", NULL};

static int mount_ref = 0;
static int lib_ref = 0;

int _emous_domain;

static Eina_Array *modules = NULL;

static Eina_Bool
_module_load_cb(Eina_Module *m, void *data EINA_UNUSED)
{
   // we take everything!
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
   // init the libs
   eo_do(EMOUS_CLASS, emous_init_lib());
   // loading modules
   modules = eina_module_list_get(modules, EMOUS_MODULE_PATH, EINA_FALSE, _module_load_cb, NULL);
   // load all found modules
   eina_module_list_load(modules);
   // init mountpoints
   // init this after loading modules
   // so every module is getting the initial mountpoints
   _emous_mm_init();
   mount_ref = 1;
   return mount_ref;
}

EOLIAN static void
_emous_shutdown(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED)
{
   // dec refs
   mount_ref --;
   if (mount_ref  != 0)
     return;

   // unload the modules
   eina_module_list_unload(modules);
   // shutdown mountpointslistening
   _emous_mm_shutdown();
   // shutdown the libs
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

EOLIAN static char *
_emous_util_size_convert(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED, Eina_Bool exact, long bytes)
{
   float niceval = bytes;
   int i = 0;
   char buf[PATH_MAX];

   for (i = 0; BYTE_ENDINGS[i + 1]; i++)
     {
        if (((float)(niceval / 1024.0f)) < 1.0f)
          break;
        niceval /= (float) 1024.0f;
     }
   if (exact)
     snprintf(buf, sizeof(buf), "%'.2f %s", niceval, BYTE_ENDINGS[i]);
   else {
     snprintf(buf, sizeof(buf), "%'.0f %s", roundf(niceval), BYTE_ENDINGS[i]);
   }

   return strdup(buf);
}
EOLIAN static char *
_emous_util_device_name_get(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED, Emous_Device *dev)
{
   const char *name;
   char *typename;
   char buf[PATH_MAX];
   Emous_Device_Type type;
   long size;

   eo_do(dev, name = emous_device_displayname_get());

   if (name)
     return strdup(name);

   eo_do(dev, type = emous_device_type_get();
              size = emous_device_size_get();
              );

   switch(type){
      case EMOUS_DEVICE_TYPE_DISK:
        typename = "Disk";
      break;
      case EMOUS_DEVICE_TYPE_CD:
        typename = "CD";
      break;
      case EMOUS_DEVICE_TYPE_REMOVABLE:
        typename = "Flash Device";
      break;
      case EMOUS_DEVICE_TYPE_FLOPPY:
        typename = "Floppy";
      break;
      case EMOUS_DEVICE_TYPE_NETWORK:
        typename = "Network Storage";
      break;
      default:
        typename = "device";
      break;
   }

   eo_do(EMOUS_CLASS, name = emous_util_size_convert(EINA_FALSE, size));

   snprintf(buf, sizeof(buf), " %s %s ", name, typename );

   return strdup(buf);
}

#include "emous.eo.x"