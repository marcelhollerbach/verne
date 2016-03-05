#include <Emous.h>
#include "emous_device_debug.eo.h"
#include "emous_type_debug.eo.h"

typedef struct {

} Emous_Device_Debug_Data;


EOLIAN static Eina_Bool
_emous_device_debug_emous_device_mount(Eo *obj EINA_UNUSED, Emous_Device_Debug_Data *pd EINA_UNUSED)
{
   return EINA_FALSE;
}

EOLIAN static Eina_Bool
_emous_device_debug_emous_device_umount(Eo *obj EINA_UNUSED, Emous_Device_Debug_Data *pd EINA_UNUSED)
{
   return EINA_FALSE;
}

EOLIAN static Emous_Device_State
_emous_device_debug_emous_device_state_get(Eo *obj EINA_UNUSED, Emous_Device_Debug_Data *pd EINA_UNUSED)
{
    return EMOUS_DEVICE_STATE_MOUNTED;
}

EOLIAN static const char*
_emous_device_debug_emous_device_displayname_get(Eo *obj EINA_UNUSED, Emous_Device_Debug_Data *pd EINA_UNUSED)
{
    return "debug-device";
}

EOLIAN static Eina_List*
_emous_device_debug_emous_device_mountpoints_get(Eo *obj EINA_UNUSED, Emous_Device_Debug_Data *pd EINA_UNUSED)
{
    return NULL;
}

EOLIAN static long
_emous_device_debug_emous_device_size_get(Eo *obj EINA_UNUSED, Emous_Device_Debug_Data *pd EINA_UNUSED)
{
    return 1024;
}

#include "emous_device_debug.eo.x"

typedef struct {

} Emous_Type_Debug_Data;

static Eina_List *devices = NULL;
static Emous_Type_Debug *sd;

EOLIAN static Eina_List*
_emous_type_debug_emous_type_devices_get(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED)
{
    return devices;
}

EOLIAN static Eina_Bool
_emous_type_debug_emous_type_create(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED, char *params EINA_UNUSED, char *mountpoint EINA_UNUSED)
{
    return EINA_FALSE;
}

EOLIAN static Eina_Bool
_emous_type_debug_emous_type_creatable(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED)
{
    return EINA_FALSE;
}

EOLIAN static const char*
_emous_type_debug_emous_type_name_get(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED)
{
    return "DebugBackend";
}

EOLIAN static Emous_Type*
_emous_type_debug_emous_type_object_get(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED)
{
    if (sd)
      return sd;

    sd = eo_add(EMOUS_TYPE_DEBUG_CLASS, NULL);

    return sd;
}

#include "emous_type_debug.eo.x"

Eina_Bool
emous_debug_device_start(void)
{
   emous_manager_device_type_add(EMOUS_MANAGER_CLASS, EMOUS_TYPE_DEBUG_CLASS);
   // add this one
   Emous_Device_Debug *db = eo_add(EMOUS_DEVICE_DEBUG_CLASS, NULL);
   devices = eina_list_append(devices, db);
   eo_event_callback_call(sd, EMOUS_TYPE_EVENT_DEVICE_ADDED, db);
   // Add a second one
   db = eo_add(EMOUS_DEVICE_DEBUG_CLASS, NULL);
   devices = eina_list_append(devices, db);
   eo_event_callback_call(sd, EMOUS_TYPE_EVENT_DEVICE_ADDED, db);
   // remove the first one
   eo_event_callback_call(sd, EMOUS_TYPE_EVENT_DEVICE_DELETED, db);

   return EINA_TRUE;
}
