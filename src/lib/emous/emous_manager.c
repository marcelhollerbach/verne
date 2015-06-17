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
_added_cb(void *data EINA_UNUSED, Eo *obj EINA_UNUSED, const Eo_Event_Description2 *desc EINA_UNUSED, void *event EINA_UNUSED)
{
    Emous_Device *device = event;

    //a device appeared
    if (!sd) return EINA_TRUE;

    sd->devices = eina_list_append(sd->devices, device);
    eo_do(manager, eo_event_callback_call(EMOUS_MANAGER_EVENT_DEVICE_ADD, device));

    return EINA_TRUE;
}

static Eina_Bool
_deled_cb(void *data EINA_UNUSED, Eo *obj EINA_UNUSED, const Eo_Event_Description2 *desc EINA_UNUSED, void *event EINA_UNUSED)
{
    Emous_Device *device = event;

    if (!sd) return EINA_TRUE;

    sd->devices = eina_list_remove(sd->devices, device);
    eo_do(manager, eo_event_callback_call(EMOUS_MANAGER_EVENT_DEVICE_DEL, device));

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

    if (!sd) eo_do(EMOUS_MANAGER_CLASS, emous_manager_object_get());

    eo_do(type, device_type = emous_type_object_get());

    if (!device_type)
      {
         ERR("Error device type does not contain a object");
      }

    sd->device_types = eina_list_append(sd->device_types, type);

    eo_do(device_type,
            eo_event_callback_add(EMOUS_TYPE_EVENT_DEVICE_ADDED, _added_cb, NULL);
            eo_event_callback_add(EMOUS_TYPE_EVENT_DEVICE_DELETED, _deled_cb, NULL);
            );
}

EOLIAN static void
_emous_manager_device_type_del(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED, const Eo_Class *type)
{
    Emous_Type *device_type;
    if (!sd) //there cannot be a type
      return;

    eo_do(type, device_type = emous_type_object_get());

    //check if this device type is valid
    if (!device_type)
      {
         ERR("Error device type does not contain a object");
      }

    //remove them from the known list
    sd->device_types = eina_list_remove(sd->device_types, type);

    //do not monitor them anymore
    eo_do(device_type, eo_event_callback_del(EMOUS_TYPE_EVENT_DEVICE_ADDED, _added_cb, NULL);
                eo_event_callback_del(EMOUS_TYPE_EVENT_DEVICE_DELETED, _deled_cb, NULL);
                );
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