#include "udisk.h"

Eldbus_Connection *con;

static Eldbus_Signal_Handler *_iadd;
static Eldbus_Signal_Handler *_idel;
Eina_Hash *devices;
Eina_List *devices_list;

int __log_domain;

static void
_interface_add_cb(void *data EINA_UNUSED, const Eldbus_Message *msg)
{
   const char *interface;

   interface = eldbus_message_interface_get(msg);
   //parse device infos
   if (!strcmp(interface, ELDBUS_FDO_INTERFACE_OBJECT_MANAGER))
     {
        const char *opath;
        Eo *obj;

        obj = _emous_device_new(eldbus_message_iter_get(msg), &opath);
        if (!obj) return;
        eina_hash_add(devices, opath, obj);
        devices_list = eina_list_append(devices_list, obj);
        DBG("Added device %s %p", opath, obj);
        eo_do(type, eo_event_callback_call(EMOUS_TYPE_EVENT_DEVICE_ADDED, obj));
     }
}

static void
_interface_del_cb(void *data EINA_UNUSED, const Eldbus_Message *msg)
{
   const char *opath;
   Eldbus_Message_Iter *arr;
   Eo *obj;

   if (!eldbus_message_arguments_get(msg, "oas", &opath, &arr))
     {
        ERR("Failed to get information about a deleted device, the db is now broken :/");
        return;
     }

   obj = eina_hash_find(devices, opath);

   if (!obj) return;

   eina_hash_del(devices, opath, obj);
   devices_list = eina_list_remove(devices_list, obj);
   eo_do(type, eo_event_callback_call(EMOUS_TYPE_EVENT_DEVICE_DELETED, obj));
}

static void
_managed_obj_cb(void *data EINA_UNUSED, const Eldbus_Message *msg, Eldbus_Pending *pen EINA_UNUSED)
{
   const char *name, *txt;
   Eldbus_Message_Iter *arr1, *dict1;

   if (eldbus_message_error_get(msg, &name, &txt))
     {
        ERR("Error %s %s.", name, txt);
        return;
     }

   //parse all objects
   if (!eldbus_message_arguments_get(msg, "a{oa{sa{sv}}}", &arr1))
     {
        ERR("Error getting arguments.");
        return;
     }

   while (eldbus_message_iter_get_and_next(arr1, 'e', &dict1))
     {
        Eo *obj;
        const char *opath;

        obj = _emous_device_new(dict1, &opath);
        if (!obj) continue;
        eina_hash_add(devices, opath, obj);
        devices_list = eina_list_append(devices_list, obj);
        DBG("Added device %s %p", opath, obj);
        eo_do(type, eo_event_callback_call(EMOUS_TYPE_EVENT_DEVICE_ADDED, obj));
     }
}

static void
_name_start_cb(void *data EINA_UNUSED, const Eldbus_Message *msg,
               Eldbus_Pending *pending EINA_UNUSED)
{
   Eldbus_Object *obj;
   Eldbus_Proxy *proxy;
   const char *txt, *name;
   unsigned int flag;

   if (eldbus_message_error_get(msg, &txt, &name))
     {
        ERR("DBUS ERROR(%s, %s)", name, txt);
        return;
     }

   if (!eldbus_message_arguments_get(msg, "u", &flag) || !flag)
     {
        ERR("Starting failed! udisk2 backend will not be working!!");
        eldbus_connection_unref(con);
        con = NULL;
        return;
     }

   //fetch all existing objects
   obj = eldbus_object_get(con, UDISKS2_BUS, UDISKS2_PATH);
   eldbus_object_managed_objects_get(obj, _managed_obj_cb, NULL);

   //create proxy
   proxy = eldbus_proxy_get(obj, ELDBUS_FDO_INTERFACE_OBJECT_MANAGER);

   //subscribe to new arriving devices
   _iadd = eldbus_proxy_signal_handler_add(proxy, "InterfacesAdded",
                                  _interface_add_cb, NULL);
   _idel = eldbus_proxy_signal_handler_add(proxy, "InterfacesRemoved",
                                  _interface_del_cb, NULL);
}

static void
_free_cb(void *data)
{
    eo_del(data);
}

static Eina_Bool
_module_init(void)
{
   Eldbus_Pending *p;
   eina_init();

   __log_domain = eina_log_domain_register("emous_udisk", NULL);
   if (!__log_domain)
     return 0;

   devices = eina_hash_string_small_new(_free_cb);

   type = eo_add(EMOUS_TYPE_UDISKS_CLASS, NULL);
   eo_do(EMOUS_MANAGER_CLASS, emous_manager_device_type_add(EMOUS_TYPE_UDISKS_CLASS));

   //init dbus
   eldbus_init();

   con = eldbus_connection_get(ELDBUS_CONNECTION_TYPE_SYSTEM);
   EINA_SAFETY_ON_NULL_RETURN_VAL(con, EINA_FALSE);

   p = eldbus_name_start(con, UDISKS2_BUS, 0, _name_start_cb, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(p, EINA_FALSE);

   return EINA_TRUE;
}

static void
_module_shutdown(void)
{
   eina_log_domain_unregister(__log_domain);
   eina_shutdown();
}

EINA_MODULE_INIT(_module_init);
EINA_MODULE_SHUTDOWN(_module_shutdown);