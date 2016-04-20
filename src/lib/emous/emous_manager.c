#include "emous_priv.h"

typedef struct {

} Emous_Manager_Data;

typedef struct {
   Eina_List *devices;
   Eina_List *device_types;
} Emous_Manager_Static_Data;

static Emous_Manager *manager;
static Emous_Manager_Static_Data *sd;

static Eina_Bool
_added_cb(void *data EINA_UNUSED, const Eo_Event *event)
{
    Emous_Device *device = event->info;

    // a device appeared
    if (!sd) return EINA_TRUE;

    sd->devices = eina_list_append(sd->devices, device);
    eo_event_callback_call(manager, EMOUS_MANAGER_EVENT_DEVICE_ADD, device);

    return EINA_TRUE;
}

static Eina_Bool
_deled_cb(void *data EINA_UNUSED, const Eo_Event *event)
{
    Emous_Device *device = event->info;

    if (!sd) return EINA_TRUE;

    sd->devices = eina_list_remove(sd->devices, device);
    eo_event_callback_call(manager, EMOUS_MANAGER_EVENT_DEVICE_DEL, device);

    return EINA_TRUE;
}

EOLIAN static Eina_List*
_emous_manager_device_types_get(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED)
{
    if (!sd)
      return NULL;

    return sd->device_types;
}

EOLIAN static void
_emous_manager_device_type_add(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED, const Eo_Class *type)
{
    Emous_Type *device_type;

    if (!sd) emous_manager_object_get(EMOUS_MANAGER_CLASS);

    device_type = emous_type_object_get(type);

    if (!device_type)
      {
         ERR("Error device type does not contain a object");
      }

    sd->device_types = eina_list_append(sd->device_types, type);

    eo_event_callback_add(device_type, EMOUS_TYPE_EVENT_DEVICE_ADDED, _added_cb, NULL);
    eo_event_callback_add(device_type, EMOUS_TYPE_EVENT_DEVICE_DELETED, _deled_cb, NULL);
}

EOLIAN static void
_emous_manager_device_type_del(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED, const Eo_Class *type)
{
    Emous_Type *device_type;
    if (!sd) // there cannot be a type
      return;

    device_type = emous_type_object_get(type);

    // check if this device type is valid
    if (!device_type)
      {
         ERR("Error device type does not contain a object");
      }

    // remove them from the known list
    sd->device_types = eina_list_remove(sd->device_types, type);

    // do not monitor them anymore
    eo_event_callback_del(device_type, EMOUS_TYPE_EVENT_DEVICE_ADDED, _added_cb, NULL);
    eo_event_callback_del(device_type, EMOUS_TYPE_EVENT_DEVICE_DELETED, _deled_cb, NULL);

}

EOLIAN static Emous_Manager*
_emous_manager_object_get(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED)
{
   if (manager)
     return manager;

   sd = calloc(1, sizeof(Emous_Manager_Static_Data));

   manager = eo_add(EMOUS_MANAGER_CLASS, NULL);

   return manager;
}

EOLIAN static Eina_List*
_emous_manager_devices_get(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED)
{
    if (!sd)
      return NULL;
    return sd->devices;
}

#include "emous_manager.eo.x"