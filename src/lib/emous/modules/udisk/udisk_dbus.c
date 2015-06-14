#include "udisk.h"

#define UDISKS2_BUS "org.freedesktop.UDisks2"
#define UDISKS2_PATH "/org/freedesktop/UDisks2"
#define UDISKS2_INTERFACE "org.freedesktop.UDisks2"
#define UDISKS2_INTERFACE_FILESYSTEM "org.freedesktop.UDisks2.Filesystem"

//udisk dbus stuff
static Eldbus_Connection *con;
static Eldbus_Proxy *proxy;
static Eldbus_Object *obj;

static Eldbus_Signal_Handler *_iadd;
static Eldbus_Signal_Handler *_idel;

static const char*
_dbus_helper_search_field(Eldbus_Message_Iter *it, const char *field)
{
   Eldbus_Message_Iter *dict3;
   while (eldbus_message_iter_get_and_next(it, 'e', &dict3))
     {
        Eldbus_Message_Iter *var;
        char *key2, *val, *type;

        if (!eldbus_message_iter_arguments_get(dict3, "sv", &key2, &var))
          continue;

        type = eldbus_message_iter_signature_get(var);
        if (type[0] == 's')
          {
             if (strcmp(key2, field))
               goto end;
             if (!eldbus_message_iter_arguments_get(var, type, &val))
               goto end;
             if (val && val[0])
               {
                  free(type);
                  return eina_stringshare_add(val);
               }
          }
end:
        free(type);
     }
   return NULL;
}

static Eina_Stringshare *
_util_fuckyouglib_convert(Eldbus_Message_Iter *fuckyouglib)
{
   Eldbus_Message_Iter *no_really;
   unsigned char c, buf[PATH_MAX] = {0};
   unsigned int x = 0;

   if (!eldbus_message_iter_arguments_get(fuckyouglib, "ay", &no_really)) return NULL;
   while (eldbus_message_iter_get_and_next(no_really, 'y', &c))
     {
        buf[x] = c;
        x++;
     }
   if (!buf[0]) return NULL;
   return eina_stringshare_add((char*)buf);
}

static void
mountpoint_update_cb(void *data EINA_UNUSED, const Eldbus_Message *msg, Eldbus_Pending *p EINA_UNUSED)
{
   Eldbus_Message_Iter *var, *mountpoints;
   Device *d = data;
   Eina_List *list = NULL, *node = NULL, *ex_mountpoints = NULL;
   const char *mp;

   if (!eldbus_message_arguments_get(msg, "v", &var))
     {
        ERR("Failed to receive mountpointsvariant");
        return;
     }

   if (!var)
     return;

   if (!eldbus_message_iter_arguments_get(var, "aay", &mountpoints))
     {
        printf("Failed to receive mountpoints");
        return;
      }

   if (!mountpoints)
     return;
   do
     {
        mp = _util_fuckyouglib_convert(mountpoints);

        if (!mp)
          continue;

        list = eina_list_append(list, mp);
     }
   while(eldbus_message_iter_next(mountpoints));

   if (!d->device)
     {
        d->tmp_list = list;
        return;
     }
   eo_do(d->device, ex_mountpoints = emous_device_mountpoints_get(););

   ex_mountpoints = eina_list_clone(ex_mountpoints);

   EINA_LIST_FOREACH(list, node, mp)
     {
        ex_mountpoints = eina_list_remove(ex_mountpoints, mp);
        eo_do(d->device, emous_device_mountpoint_add(mp));
     }
   //the remaining ex_mountpoints are those which dissappeared
   EINA_LIST_FREE(ex_mountpoints, mp)
     {
        eo_do(d->device, emous_device_moutpoint_del(mp));
     }

   if (d->device)
     {
        eo_do(d->device, list = emous_device_mountpoints_get());
        if (list)
          eo_do(d->device, emous_device_state_set(DEVICE_STATE_MOUNTED));
     }
}

static void
_device_mounts_get(Device *d)
{
   eldbus_proxy_property_get(d->proxy, "MountPoints", mountpoint_update_cb, d);
}

static Device*
_device_parse(const char *opath, Eldbus_Message_Iter *partition, Eldbus_Message_Iter *block, Eldbus_Message_Iter *fs EINA_UNUSED, Eldbus_Proxy *proxy)
{
   /* name out of partition, if this is not valid, take the idname out of block */
   Device *d;
   d = calloc(1, sizeof(Device));
   d->proxy = proxy;
   d->opath = eina_stringshare_add(opath);

   _device_mounts_get(d);

   d->displayname = _dbus_helper_search_field(block, "IdLabel");
   if (d->displayname)
     return d;
   d->displayname = _dbus_helper_search_field(block, "HintName");
   if (d->displayname)
     return d;
   d->displayname = _dbus_helper_search_field(partition, "Name");

   return d;
}

static void
_prop_changed_cb(void *data, const Eldbus_Message *msg EINA_UNUSED)
{
  Device *d = data;

  _device_mounts_get(d);
}

static void
_device_handle(Eldbus_Message_Iter *dict)
{
   const char *opath;
   Eldbus_Object *obj;
   Eldbus_Proxy *proxy;
   Device *d;
   Eldbus_Message_Iter *interfaces, *interface;
   Eldbus_Message_Iter *partition = NULL,
                       *block = NULL,
                       *fs = NULL;
   //get interfaces and objectpath
   if (!eldbus_message_iter_arguments_get(dict, "oa{sa{sv}}", &opath, &interfaces))
     {
        ERR("Failed to get arguments!!");
     }

   //iterate all interfaces
   while (eldbus_message_iter_get_and_next(interfaces, 'e', &interface))
     {
        const char *interface_name;
        Eldbus_Message_Iter *values;

        /* We are caching those 3 iters and process them later,
           if one is missing we can drop this device! */

        if (!eldbus_message_iter_arguments_get(interface, "sa{sv}", &interface_name, &values))
          {
             ERR("Failed to get interface arguements!");
             continue;
          }
        //shortn name
        interface_name += sizeof(UDISKS2_INTERFACE);
        if (!strcmp(interface_name, "Partition"))
          partition = values;
        else if (!strcmp(interface_name, "Block"))
          block = values;
        else if (!strcmp (interface_name, "Filesystem"))
          fs = values;
     }

   //check if all interfaces are found
   if (!(partition && block && fs))
     {
        DBG("Dropping device %s, needed interfaces not found!", opath);
        return;
     }

   DBG("Adding device %s", opath);

   obj = eldbus_object_get(con, UDISKS2_BUS, opath);
   proxy = eldbus_proxy_get(obj, UDISKS2_INTERFACE_FILESYSTEM);

   if (!proxy)
     {
        ERR("Failed to create proxy, this device will not work as expected!");
        return;
     }

   //parse device
   d = _device_parse(opath, partition, block, fs, proxy);

   if (!d)
     {
        ERR("Parsing of valid device failed!");
        return;
     }

   d->changed = eldbus_proxy_properties_changed_callback_add(proxy, _prop_changed_cb, d);

   //add device
   _device_add(d);
}

static void
_interface_add_cb(void *data EINA_UNUSED, const Eldbus_Message *msg)
{
   const char *interface;

   interface = eldbus_message_interface_get(msg);
   //parse device infos
   if (!strcmp(interface, ELDBUS_FDO_INTERFACE_OBJECT_MANAGER))
     _device_handle(eldbus_message_iter_get(msg));
}

static void
_interface_del_cb(void *data EINA_UNUSED, const Eldbus_Message *msg)
{
   const char *opath;
   Eldbus_Message_Iter *arr;

   if (!eldbus_message_arguments_get(msg, "oas", &opath, &arr))
     {
        ERR("Failed to get information about a deleted device, the db is now broken :/");
        return;
     }

   _device_del(opath);

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
     _device_handle(dict1);
}

static void
_name_start_cb(void *data EINA_UNUSED, const Eldbus_Message *msg,
               Eldbus_Pending *pending EINA_UNUSED)
{
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
_mount_cb(void *data, const Eldbus_Message *msg, Eldbus_Pending *pen EINA_UNUSED)
{
   const char *name, *txt;
   const char *point;
   Device *d = data;
   Eina_List *mnt_points;

   if (eldbus_message_error_get(msg, &txt, &name))
     {
        ERR("DBUS ERROR(%s, %s)", name, txt);
        goto end;
     }

   if (!eldbus_message_arguments_get(msg, "s", &point))
     {
        ERR("Failed to get the new mountpoint! Not sure if mounted or not :(");
        goto end;
     }

   eo_do(d->device, emous_device_mountpoint_add(point));
   DBG("Mount call was successfully at %s", point);
end:
   eo_do(d->device, mnt_points = emous_device_mountpoints_get());
   eo_do(d->device, emous_device_state_set(eina_list_count(mnt_points) > 0 ? DEVICE_STATE_MOUNTED : DEVICE_STATE_UMOUNTED));
}

Eina_Bool
udisk_mount(Device *d)
{
   Eldbus_Message *msg;
   Eldbus_Message_Iter *array, *main_iter;

   msg = eldbus_proxy_method_call_new(d->proxy, "Mount");
   main_iter = eldbus_message_iter_get(msg);

   if (!eldbus_message_iter_arguments_append(main_iter, "a{sv}", &array))
     ERR("Failed to construct arguments!");

   eldbus_message_iter_container_close(main_iter, array);

   return !!eldbus_proxy_send(d->proxy, msg, _mount_cb, d, -1);
}

static void
_umount_cb(void *data, const Eldbus_Message *msg, Eldbus_Pending *pen EINA_UNUSED)
{
   const char *name, *txt;
   Device *d = data;
   Eina_List *mnt_points;

   if (eldbus_message_error_get(msg, &txt, &name))
     {
        ERR("DBUS ERROR(%s, %s)", name, txt);
     }

   eo_do(d->device, mnt_points = emous_device_mountpoints_get());
   eo_do(d->device, emous_device_state_set(eina_list_count(mnt_points) > 0 ? DEVICE_STATE_MOUNTED : DEVICE_STATE_UMOUNTED));
}

Eina_Bool
udisk_umount(Device *d)
{
   Eldbus_Message *msg;
   Eldbus_Message_Iter *array, *main_iter;

   msg = eldbus_proxy_method_call_new(d->proxy, "Unmount");
   main_iter = eldbus_message_iter_get(msg);

   if (!eldbus_message_iter_arguments_append(main_iter, "a{sv}", &array))
     ERR("Failed to construct arguments!");

   eldbus_message_iter_container_close(main_iter, array);
   return !!eldbus_proxy_send(d->proxy, msg, _umount_cb, d, -1);
}

Eina_Bool
udisk_dbus_init()
{
   Eldbus_Pending *p;
   eldbus_init();

   con = eldbus_connection_get(ELDBUS_CONNECTION_TYPE_SYSTEM);
   EINA_SAFETY_ON_NULL_RETURN_VAL(con, EINA_FALSE);

   p = eldbus_name_start(con, UDISKS2_BUS, 0, _name_start_cb, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(p, EINA_FALSE);

   return EINA_TRUE;
}

void
udisk_dbus_shutdown()
{
   if (_iadd)
     eldbus_signal_handler_del(_iadd);

   if (_idel)
     eldbus_signal_handler_del(_idel);

   if (obj)
     eldbus_object_unref(obj);
   //proxy gets freed when we free the object
   //eldbus_proxy_unref(proxy);
   eldbus_connection_unref(con);

   eldbus_shutdown();
}