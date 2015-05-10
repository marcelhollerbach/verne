#include "udisk.h"

//log domain
int _log_domain;

//emous objects
static Emous_Device_Class *c;

//internal device map
static Eina_Hash *devices;

static Eina_Bool
_mount_request_cb(void *data EINA_UNUSED, Eo *obj, const Eo_Event_Description *desc EINA_UNUSED, void *event EINA_UNUSED)
{
   Device *d;

   d = eina_hash_find(devices, &obj);

   eo_do(d->device, emous_device_state_set(_DEVICE_STATE_MOUNTREQ));

   if (udisk_mount(d))
     {
        ERR("Mount failed!");
     }

   return EINA_TRUE;
}

static Eina_Bool
_umount_request_cb(void *data EINA_UNUSED, Eo *obj EINA_UNUSED, const Eo_Event_Description *desc EINA_UNUSED, void *event EINA_UNUSED)
{
   Device *d;

   d = eina_hash_find(devices, &obj);

   eo_do(d->device, emous_device_state_set(_DEVICE_STATE_UMOUNTREQ));

   if (!udisk_umount(d))
     {
        ERR("umount failed!");
     }

   return EINA_TRUE;
}

void
_device_add(Device *d)
{
   Emous_Device *dev;
   Eina_List *mountpoints;
   eo_do(c, dev = emous_device_class_device_add(d->displayname));
   
   mountpoints = d->tmp_list;

   {
      Eina_List *node;
      const char *path;

      EINA_LIST_FOREACH(mountpoints, node, path)
        {
           eo_do(dev, emous_device_mountpoint_add(path));
        }
   }
   if (mountpoints == NULL)
     eo_do(dev, emous_device_state_set(_DEVICE_STATE_UMOUNTED));
   else
     eo_do(dev, emous_device_state_set(_DEVICE_STATE_MOUNTED));
  
   //connect them
   d->device = dev;

   eina_hash_add(devices, &dev, d);

   eo_do(dev, eo_event_callback_add(EMOUS_DEVICE_EVENT_MOUNT_REQUEST, _mount_request_cb, NULL);
              eo_event_callback_add(EMOUS_DEVICE_EVENT_UMOUNT_REQUEST, _umount_request_cb, NULL);
              emous_device_populate();
               );

   DBG("Added device %s", d->displayname);
}

void
_device_del(const char *opath)
{
   //TODO
}

static Eina_Bool
_add_cb(void *data EINA_UNUSED, Eo *obj EINA_UNUSED, const Eo_Event_Description *desc EINA_UNUSED, void *event EINA_UNUSED)
{
   return EINA_TRUE;
}
static Eina_Bool
_del_cb(void *data EINA_UNUSED, Eo *obj EINA_UNUSED, const Eo_Event_Description *desc EINA_UNUSED, void *event EINA_UNUSED)
{
   return EINA_TRUE;
}

static Eina_Bool
_module_init(void)
{
   eina_init();
   emous_init();

   _log_domain = eina_log_domain_register("emous_udisk", NULL);
   if (!_log_domain)
     return 0;

   udisk_dbus_init();

   eo_do(EMOUS_MANAGER_CLASS, c = emous_manager_device_class_add("udisk"));
   EINA_SAFETY_ON_NULL_RETURN_VAL(c, EINA_FALSE);

   eo_do(c, eo_event_callback_add(EMOUS_DEVICE_CLASS_EVENT_MOUNTPOINT_CHECK_ADD, _add_cb, NULL);
            eo_event_callback_add(EMOUS_DEVICE_CLASS_EVENT_MOUNTPOINT_CHECK_DEL, _del_cb, NULL);
         );

   devices = eina_hash_pointer_new(NULL/*FIXME*/);
   EINA_SAFETY_ON_NULL_RETURN_VAL(devices, EINA_FALSE);

   return EINA_TRUE;
}

static void
_module_shutdown(void)
{
   eina_hash_free(devices);
   eo_del(c);

   udisk_dbus_shutdown();
   emous_shutdown();
   eina_shutdown();
}

EINA_MODULE_INIT(_module_init);
EINA_MODULE_SHUTDOWN(_module_shutdown);