#include "emous_priv.h"

typedef struct {

} Emous_Data;

const char* BYTE_ENDINGS[] = {"B", "KiB", "MiB", "GiB", "TiB", "PiB", "EiB", NULL};

static int mount_ref = 0;

int _emous_domain;

static Eina_Array *modules = NULL;

static Eina_Bool
_module_load_cb(Eina_Module *m, void *data EINA_UNUSED)
{
   // we take everything!
   DBG("Loading module %s", eina_module_file_get(m));
   return EINA_TRUE;
}

static Eina_Bool
_init_lib(void)
{
   if (!eina_init())
     {
        printf("Eina init failed!\n");
        goto eina_err;
     }

   _emous_domain = eina_log_domain_register("emous", NULL);
   if (!_emous_domain)
     goto eina_err;

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
   return 0;
}

EAPI int
emous_init(void)
{
   if (mount_ref > 0)
     return ++mount_ref;

   // init the libs
   if (!_init_lib())
     return 0;

   if (!getenv("SKIP_MODULES"))
     {
        // loading modules
        modules = eina_module_list_get(modules, EMOUS_MODULE_PATH, EINA_FALSE, _module_load_cb, NULL);
        // load all found modules
        eina_module_list_load(modules);
        // init mountpoints
        // init this after loading modules
        // so every module is getting the initial mountpoints
        _emous_mm_init();
     }

   mount_ref = 1;
   return mount_ref;
}

static void
_shutdown_lib()
{
   eio_shutdown();
   ecore_shutdown();
   eina_log_domain_unregister(_emous_domain);
   eina_shutdown();
}

EAPI void
emous_shutdown()
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
   _shutdown_lib();
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

   name = emous_device_displayname_get(dev);

   if (name)
     return strdup(name);

   type = emous_device_type_get(dev);
   size = emous_device_size_get(dev);

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

   name = emous_util_size_convert(EMOUS_CLASS, EINA_FALSE, size);

   snprintf(buf, sizeof(buf), " %s %s ", name, typename );

   return strdup(buf);
}

#include "emous.eo.x"