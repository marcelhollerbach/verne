#include "emous_priv.h"

#include "emous_device_class.eo.h"
#include "emous_device.eo.h"

int EMOUS_MOUNT_REQUEST_ADD;
int EMOUS_MOUNT_REQUEST_DEL;

typedef struct {
   const char *displayname;
   Device_State state;
} Emous_Device_Data;


typedef struct {
   const char* name;
   Eina_List *devices;
} Emous_Device_Class_Data;

static void
_emous_device_class_name_set(Eo *obj EINA_UNUSED, Emous_Device_Class_Data *pd, const char *name)
{
  pd->name = eina_stringshare_add(name);
}

static const char*
_emous_device_class_name_get(Eo *obj EINA_UNUSED, Emous_Device_Class_Data *pd)
{
  return pd->name;
}

static void
_emous_device_state_set(Eo *obj, Emous_Device_Data *pd, Device_State value)
{
   pd->state = value;
   eo_do(obj,
         eo_event_callback_call(EMOUS_DEVICE_EVENT_STATE_CHANGED, NULL);
         );
}

static Device_State
_emous_device_state_get(Eo *obj EINA_UNUSED, Emous_Device_Data *pd)
{
   return pd->state;
}

static void
_emous_device_mount(Eo *obj, Emous_Device_Data *pd EINA_UNUSED, const char *params)
{
   eo_do(obj,
         eo_event_callback_call(EMOUS_DEVICE_EVENT_MOUNT_REQUEST, (void*) params);
         );
}

static void
_emous_device_umount(Eo *obj, Emous_Device_Data *pd EINA_UNUSED)
{
   eo_do(obj,
         eo_event_callback_call(EMOUS_DEVICE_EVENT_UMOUNT_REQUEST, NULL);
         );
}

static void
_emous_device_construct(Eo *obj EINA_UNUSED, Emous_Device_Data *pd, const char *name)
{
   pd->displayname = eina_stringshare_add(name);
}

static Emous_Device*
_emous_device_class_device_add(Eo *obj, Emous_Device_Class_Data *pd, const char *name)
{
   Emous_Device *result;
   //create new device
   result = eo_add(EMOUS_DEVICE_CLASS, NULL, emous_device_construct(name));
   //add list
   pd->devices = eina_list_append(pd->devices, result);
   //emit add signal
   eo_do(obj,
        eo_event_callback_call(EMOUS_DEVICE_CLASS_EVENT_DEVICE_ADD, result);
        );
   return result;
}

static void
_emous_device_class_construct(Eo *obj EINA_UNUSED, Emous_Device_Class_Data *pd, const char *name)
{
   pd->name = eina_stringshare_add(name);
}


#include "emous_device.eo.x"
#include "emous_device_class.eo.x"