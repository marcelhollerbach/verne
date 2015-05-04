#include "emous_priv.h"

typedef struct {
   int i_should_not_be_empty;
} Emous_Manager_Data;

typedef struct {
   Eina_Hash *hash;
} Emous_Manager_Static_Data;

typedef struct {
   const char *displayname;
   Device_State state;
} Emous_Device_Data;

typedef struct {
   const char* name;
   Eina_List *keywords;
   Eina_List *devices;
} Emous_Device_Class_Data;

static Emous_Manager_Static_Data *sd;
static Emous_Manager *manager;

#define M_SIGNAL(sig) \
eo_do(manager, eo_event_callback_call(sig, NULL));

int
mount_init()
{
   sd = calloc(1, sizeof(Emous_Manager_Static_Data));
   sd->hash = eina_hash_stringshared_new(NULL);

   manager = eo_add(EMOUS_MANAGER_CLASS, NULL);

   return 1;
}

static Emous_Manager*
_emous_manager_object_get(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED)
{
   return manager;
}


void
mount_shutdown()
{
   Eina_Iterator *it;
   void *data;

   it = eina_hash_iterator_data_new(sd->hash);

   EINA_ITERATOR_FOREACH(it, data)
      eo_del(data);

   eina_iterator_free(it);

   eina_hash_free(sd->hash);
   eo_del(manager);
   free(sd);
}

//==========================
// emous_device_class implement
//==========================

static Eina_Bool
_device_events_proxy(void *data EINA_UNUSED, Eo *obj, const Eo_Event_Description *desc EINA_UNUSED, void *event EINA_UNUSED)
{
   Device_State s;

   eo_do(obj, s = emous_device_state_get());
   if (s == _DEVICE_STATE_MOUNTED)
     {
        M_SIGNAL(EMOUS_MANAGER_EVENT_MOUNT_ADD);
     }
   else if (s == _DEVICE_STATE_UMOUNTREQ || s == _DEVICE_STATE_MOUNTREQ)
     {
        M_SIGNAL(EMOUS_MANAGER_EVENT_MOUNT_REQUEST_ADD);
     }
   else
     {
        M_SIGNAL(EMOUS_MANAGER_EVENT_MOUNT_DEL);
     }

   if (((Device_State) event) == _DEVICE_STATE_MOUNTREQ ||
       ((Device_State) event) == _DEVICE_STATE_UMOUNTREQ)
     {
        M_SIGNAL(EMOUS_MANAGER_EVENT_MOUNT_REQUEST_END);
     }
   return EINA_TRUE;
}

static void
_emous_device_class_type_add(Eo *obj EINA_UNUSED, Emous_Device_Class_Data *pd, const char *type)
{
   pd->keywords = eina_list_append(pd->keywords, eina_stringshare_add(type));
}

static Eina_Bool
_emous_device_class_type_test(Eo *obj EINA_UNUSED, Emous_Device_Class_Data *pd, const char *type)
{
   const char *words;
   Eina_List *lst;
   const char *s;

   s = eina_stringshare_add(type);

   EINA_LIST_FOREACH(pd->keywords, lst, words)
     {
        if (words == s)
          return EINA_TRUE;
     }

   return EINA_FALSE;
}

static void
_emous_device_class_mountpoint_removed(Eo *obj, Emous_Device_Class_Data *pd EINA_UNUSED, const char *mountpoint)
{
   eo_do(obj,
        eo_event_callback_call(EMOUS_DEVICE_CLASS_EVENT_MOUNTPOINT_CHECK_DEL, (void*)mountpoint);
        );
}

static void
_emous_device_class_mountpoint_added(Eo *obj, Emous_Device_Class_Data *pd EINA_UNUSED, const char *mountpoint, const char *source)
{
   Emous_Mountpoint *emp;

   emp = calloc(1, sizeof(Emous_Mountpoint));

   emp->mountpoint = mountpoint;
   emp->source = source;

   eo_do(obj,
        eo_event_callback_call(EMOUS_DEVICE_CLASS_EVENT_MOUNTPOINT_CHECK_ADD, emp);
        );

   free(emp);
}

static void
_emous_device_class_eo_base_destructor(Eo *obj, Emous_Device_Class_Data *pd)
{
   Eina_List *lst;
   void *dev;

   EINA_LIST_FOREACH(pd->devices, lst, dev)
     eo_del(dev);

   if (!eina_hash_del(sd->hash, pd->name, obj))
     ERR("Failed to remove a device_class from internal table, this is (near 100%%) a bug");

   {
      //TODO free types
   }

   eina_stringshare_del(pd->name);

   eo_do_super(obj, EMOUS_DEVICE_CLASS_CLASS, eo_destructor());
}

static const char*
_emous_device_class_name_get(Eo *obj EINA_UNUSED, Emous_Device_Class_Data *pd)
{
  return pd->name;
}

static Eina_Bool
_device_del(void *data, Eo *obj, const Eo_Event_Description *desc EINA_UNUSED, void *event EINA_UNUSED)
{
   Emous_Device_Class *cl = data;
   Emous_Device_Class_Data *cd;

   cd = eo_data_scope_get(cl, EMOUS_DEVICE_CLASS_CLASS);

   cd->devices = eina_list_remove(cd->devices, obj);

   eo_do(obj,
        eo_event_callback_call(EMOUS_DEVICE_CLASS_EVENT_DEVICE_DEL, obj);
        );

   return EINA_TRUE;
}

static Emous_Device*
_emous_device_class_device_add(Eo *obj, Emous_Device_Class_Data *pd, const char *name)
{
   Emous_Device *result;
   //create new device
   result = eo_add(EMOUS_DEVICE_CLASS, obj, emous_device_construct(name));

   //add proxy redirects
   eo_do(result,  eo_event_callback_add(EO_BASE_EVENT_DEL, _device_del, obj);
                  eo_event_callback_add(EMOUS_DEVICE_EVENT_STATE_CHANGED, _device_events_proxy, NULL));
   //add list
   pd->devices = eina_list_append(pd->devices, result);

   return result;
}

static void
_emous_device_class_construct(Eo *obj EINA_UNUSED, Emous_Device_Class_Data *pd, const char *name)
{
   pd->name = eina_stringshare_add(name);
}

//==========================
// emous_device implement
//==========================

static void
_emous_device_populate(Eo *obj, Emous_Device_Data *pd)
{
   Emous_Device_Class *c;
   eo_do(obj, c = eo_parent_get());

   eo_do(c,
        eo_event_callback_call(EMOUS_DEVICE_CLASS_EVENT_DEVICE_ADD, obj);
        );
}

static const char*
_emous_device_displayname_get(Eo *obj EINA_UNUSED, Emous_Device_Data *pd)
{
   return pd->displayname;
}

static void
_emous_device_eo_base_destructor(Eo *obj, Emous_Device_Data *pd EINA_UNUSED)
{
   eina_stringshare_del(pd->displayname);

   eo_do_super(obj, EMOUS_DEVICE_CLASS, eo_destructor());
}

static void
_emous_device_state_set(Eo *obj, Emous_Device_Data *pd, Device_State value)
{
   Device_State old = pd->state;

   pd->state = value;
   eo_do(obj,
         eo_event_callback_call(EMOUS_DEVICE_EVENT_STATE_CHANGED, (void*)old);
         );
}

static Device_State
_emous_device_state_get(Eo *obj EINA_UNUSED, Emous_Device_Data *pd)
{
   return pd->state;
}

static void
_emous_device_mount(Eo *obj, Emous_Device_Data *pd EINA_UNUSED)
{
   eo_do(obj,
         eo_event_callback_call(EMOUS_DEVICE_EVENT_MOUNT_REQUEST, NULL);
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
_emous_device_construct(Eo *obj EINA_UNUSED, Emous_Device_Data *pd, const char *displayname)
{
   pd->displayname = eina_stringshare_add(displayname);
}

//===============================
//emous_manager_device implement
//===============================

static Eina_Bool
_class_proxy_cb(void *data EINA_UNUSED, Eo *obj EINA_UNUSED, const Eo_Event_Description *desc, void *event EINA_UNUSED)
{
   Emous_Device *d = event;
   if (desc == EMOUS_DEVICE_CLASS_EVENT_DEVICE_ADD)
     {
        eo_do(manager, eo_event_callback_call(EMOUS_MANAGER_EVENT_DEVICE_ADD, d));
     }
   else if (desc == EMOUS_DEVICE_CLASS_EVENT_DEVICE_DEL)
     {
        eo_do(manager, eo_event_callback_call(EMOUS_MANAGER_EVENT_DEVICE_DEL, d));
     }
   return EINA_TRUE;
}

static Eina_List *
_emous_manager_device_classes_get(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED)
{
   Eina_Iterator *it;
   Eina_List *result = NULL;
   void *data;

   it = eina_hash_iterator_data_new(sd->hash);

   EINA_ITERATOR_FOREACH(it, data)
      result = eina_list_append(result, data);

   eina_iterator_free(it);

   return result;
}

static Emous_Device_Class *
_emous_manager_device_class_get(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED, const char *name)
{
   Eina_Iterator *it;
   Emous_Device_Class *d;

   it = eina_hash_iterator_data_new(sd->hash);

   EINA_ITERATOR_FOREACH(it, d)
     {
        eo_do(d,
                 if (emous_device_class_type_test(name))
                   return d;
              );
     }

   eina_iterator_free(it);

   return NULL;
}

static Emous_Device_Class*
_emous_manager_device_class_add(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED, const char *name)
{
   Emous_Device_Class *class = NULL;
   const char *sname;

   sname = eina_stringshare_add(name);
   class = eo_add(EMOUS_DEVICE_CLASS_CLASS, NULL, emous_device_class_construct(sname));

   eo_do(class,  eo_event_callback_add(EMOUS_DEVICE_CLASS_EVENT_DEVICE_ADD, _class_proxy_cb, obj);
                  eo_event_callback_add(EMOUS_DEVICE_CLASS_EVENT_DEVICE_DEL, _class_proxy_cb, obj);
                );

   eina_hash_add(sd->hash, sname, class);

   return class;
}

#include "emous_manager.eo.x"
#include "emous_device.eo.x"
#include "emous_device_class.eo.x"