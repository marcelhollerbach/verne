#include "udisk.h"

typedef struct {
   Emous_Device_State state; //the current state of this device
   const char *device; //the object path of the device
   const char *displayname; //the displayname of this object
   uint64_t size; //the size of this object
   const char *opath; //the object path where to find this object
   Eldbus_Object *obj; //the object of this device
   Eldbus_Proxy *proxy; //the proxy to use to communicate with the dbus object
   Eldbus_Signal_Handler *changed; // the signalhandler for a changed signal handler
   Eina_List *mountpoints; // the list of mountpoints where this device is mounted
} Emous_Device_UDisks_Data;

typedef struct {
  Emous_Device_Type t;
} Device_Entry;

static void
_device_state_change(Emous_Device *dev, Emous_Device_State state)
{
    Emous_Device_UDisks_Data *pd = eo_data_scope_get(dev, EMOUS_DEVICE_UDISKS_CLASS);
    Emous_Device_State_Changed ev;

    if (pd->state == state) return;

    ev.old = pd->state;
    ev.current = state;

    pd->state = state;
    eo_do(dev, eo_event_callback_call(EMOUS_DEVICE_EVENT_STATE_CHANGED, &ev));
}

static void
_device_mp_add(Emous_Device *dev, const char *mp)
{
   Emous_Device_UDisks_Data *pd = eo_data_scope_get(dev, EMOUS_DEVICE_UDISKS_CLASS);
   Eina_List *node;
   const char *mpi;

   //check for dups
   EINA_LIST_FOREACH(pd->mountpoints, node, mpi)
     {
        if (!strcmp(mp, mpi))
          return;
     }

   pd->mountpoints = eina_list_append(pd->mountpoints, mp);
   _device_state_change(dev, EMOUS_DEVICE_STATE_MOUNTED);
}

static void
_device_mp_del(Emous_Device *dev, const char *mp)
{
   Emous_Device_UDisks_Data *pd = eo_data_scope_get(dev, EMOUS_DEVICE_UDISKS_CLASS);
   Emous_Device_State state;
   Eina_List *node;
   const char *mpi;

   EINA_LIST_FOREACH(pd->mountpoints, node, mpi)
     {
        if (!strcmp(mp, mpi))
          {
             pd->mountpoints  = eina_list_remove_list(pd->mountpoints, node);
             goto end;
          }
     }
end:
   if (eina_list_count(pd->mountpoints) > 0)
     state = EMOUS_DEVICE_STATE_MOUNTED;
   else
     state = EMOUS_DEVICE_STATE_UMOUNTED;
}

static Eina_Bool
_dbus_helper_search_field(Eldbus_Message_Iter *it, const char *field, const char **char_return, Eina_List **tmp_list, uint64_t *tmp_numb, Eina_Bool *tmp_bool)
{
   Eldbus_Message_Iter *dict3;
   while (eldbus_message_iter_get_and_next(it, 'e', &dict3))
     {
        Eldbus_Message_Iter *var;
        char *key2, *type;

        if (!eldbus_message_iter_arguments_get(dict3, "sv", &key2, &var))
          continue;

        if (!!strcmp(key2, field))
          continue;
        type = eldbus_message_iter_signature_get(var);
        if (!strcmp(type, "s") || !strcmp(type, "o"))
          {
             char *val;
             if (!eldbus_message_iter_arguments_get(var, type, &val))
               {
                  free(type);
                  return EINA_FALSE;
               }
             if (val && val[0])
               {
                  free(type);
                  *char_return = eina_stringshare_add(val);
                  return EINA_TRUE;
               }
          }
        else if (!strcmp(type, "as"))
          {
             Eldbus_Message_Iter *inner_array;
             const char *tmp;
             if (!eldbus_message_iter_arguments_get(var, type, &inner_array))
               goto end;
             while(eldbus_message_iter_get_and_next(inner_array, 's', &tmp))
               *tmp_list = eina_list_append(*tmp_list, eina_stringshare_add(tmp));
             return EINA_TRUE;
          }
        else if (!strcmp(type, "t"))
          {
             if (!eldbus_message_iter_arguments_get(var, type, tmp_numb))
               {
                  free(type);
                  return EINA_FALSE;
               }

             free(type);
             return EINA_TRUE;
          }
        else if (!strcmp(type, "b"))
          {
             if (!eldbus_message_iter_arguments_get(var, type, tmp_bool))
               {
                  free(type);
                  return EINA_FALSE;
               }

             free(type);
             return EINA_TRUE;
          }
end:
        free(type);
     }
   return EINA_FALSE;
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
   Emous_Device *dev = data;
   Emous_Device_UDisks_Data *pd = eo_data_scope_get(dev, EMOUS_DEVICE_UDISKS_CLASS);
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

   ex_mountpoints = eina_list_clone(pd->mountpoints);

   //delete the now existing mounpoints out of the old list
   EINA_LIST_FOREACH(list, node, mp)
     {
        ex_mountpoints = eina_list_remove(ex_mountpoints, mp);
        _device_mp_add(dev, mp);
     }
   //the remaining ex_mountpoints are those which dissappeared
   EINA_LIST_FREE(ex_mountpoints, mp)
     {
        _device_mp_del(dev, mp);
     }
}

static void
_device_mounts_get(Emous_Device *dev)
{
   Emous_Device_UDisks_Data *pd;

   pd = eo_data_scope_get(dev, EMOUS_DEVICE_UDISKS_CLASS);

   eldbus_proxy_property_get(pd->proxy, "MountPoints", mountpoint_update_cb, dev);
}

static void
_prop_changed_cb(void *data, const Eldbus_Message *msg EINA_UNUSED)
{
  Emous_Device *dev = data;

  _device_mounts_get(dev);
}

static inline void
_device_parse(Emous_Device *dev, Emous_Device_UDisks_Data *pd, Eldbus_Message_Iter *partition, Eldbus_Message_Iter *block, Eldbus_Message_Iter *fs EINA_UNUSED)
{
   /* name out of partition, if this is not valid, take the idname out of block */

   _device_mounts_get(dev);

   _dbus_helper_search_field(block, "Drive", &pd->device, NULL, NULL, NULL);

   _dbus_helper_search_field(partition, "Size", NULL, NULL, &pd->size, NULL);

   _dbus_helper_search_field(block, "IdLabel", &pd->displayname, NULL, NULL, NULL);
   if (pd->displayname)
     return;
   _dbus_helper_search_field(block, "HintName", &pd->displayname, NULL, NULL, NULL);
   if (pd->displayname)
     return;
   _dbus_helper_search_field(partition, "Name", &pd->displayname, NULL, NULL, NULL);

   return;
}

static inline void
_drive_parse(Eldbus_Message_Iter *iter, const char *opath)
{
   Device_Entry *entry = calloc(1, sizeof(entry));

   Eina_Bool removable = EINA_TRUE;

   if (!_dbus_helper_search_field(iter, "Removable", NULL, NULL, NULL, &removable))
     ERR("Failed to fetch Removable");

   if (removable)
     {
        //lets say this is a usb device
       entry->t = EMOUS_DEVICE_TYPE_REMOVABLE;
     }
   else
     {
        //lets say its a hdd
        entry->t = EMOUS_DEVICE_TYPE_DISK;
     }

   eina_hash_add(drives, opath, entry);
}

Emous_Device*
_emous_device_new(Eldbus_Message_Iter *dict, const char **opath)
{
   Emous_Device *dev;
   Emous_Device_UDisks_Data *pd;

   Eldbus_Message_Iter *interfaces, *interface;
   Eldbus_Message_Iter *partition = NULL,
                       *block = NULL,
                       *fs = NULL,
                       *drive = NULL;

   //create a object to work with
   dev = eo_add(EMOUS_DEVICE_UDISKS_CLASS, type);
   pd = eo_data_scope_get(dev, EMOUS_DEVICE_UDISKS_CLASS);

   //get interfaces and objectpath
   if (!eldbus_message_iter_arguments_get(dict, "oa{sa{sv}}", &pd->opath, &interfaces))
     {
        ERR("Failed to get arguments!!");
     }

   *opath = pd->opath;

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
        else if (!strcmp (interface_name, "Drive"))
          drive = values;
     }



   //check if all interfaces are found
   if (partition && block && fs)
     {
        DBG("We found a usefull device %s", pd->opath);
        pd->obj = eldbus_object_get(con, UDISKS2_BUS, pd->opath);
        pd->proxy = eldbus_proxy_get(pd->obj, UDISKS2_INTERFACE_FILESYSTEM);

        if (!pd->proxy)
          {
             ERR("Failed to create proxy, this device will not work as expected!");
             eldbus_object_unref(pd->obj);
             eldbus_proxy_unref(pd->proxy);
             goto fail_dev;
          }
        //parse device
        _device_parse(dev, pd, partition, block, fs);

        pd->changed = eldbus_proxy_properties_changed_callback_add(pd->proxy, _prop_changed_cb, dev);
        return dev;
     }
   else if (drive)
     {
         //this will change something usefull for us
         _drive_parse(drive, pd->opath);
     }

fail_dev:

   eo_del(dev);

   return NULL;
}

static void
_mount_cb(void *data, const Eldbus_Message *msg, Eldbus_Pending *pen EINA_UNUSED)
{
   const char *name, *txt;
   const char *point;
   Emous_Device *dev = data;

   if (eldbus_message_error_get(msg, &txt, &name))
     {
        ERR("DBUS ERROR(%s, %s)", name, txt);
        return;
     }

   if (!eldbus_message_arguments_get(msg, "s", &point))
     {
        ERR("Failed to get the new mountpoint! Not sure if mounted or not :(");
        return;
     }

   _device_mounts_get(dev);
   DBG("Mount call was successfully at %s", point);
}

EOLIAN static Eina_Bool
_emous_device_udisks_emous_device_mount(Eo *obj, Emous_Device_UDisks_Data *pd)
{
   Eldbus_Message *msg;
   Eldbus_Message_Iter *array, *main_iter;

   msg = eldbus_proxy_method_call_new(pd->proxy, "Mount");
   main_iter = eldbus_message_iter_get(msg);

   if (!eldbus_message_iter_arguments_append(main_iter, "a{sv}", &array))
     ERR("Failed to construct arguments!");

   eldbus_message_iter_container_close(main_iter, array);

   if (!!eldbus_proxy_send(pd->proxy, msg, _mount_cb, obj, -1))
     {
        _device_state_change(obj, EMOUS_DEVICE_STATE_MOUNT_REQ);
        return EINA_TRUE;
     }
   else
     return EINA_FALSE;
}

static void
_umount_cb(void *data, const Eldbus_Message *msg, Eldbus_Pending *pen EINA_UNUSED)
{
   const char *name, *txt;
   Emous_Device *dev = data;

   if (eldbus_message_error_get(msg, &txt, &name))
     {
        ERR("DBUS ERROR(%s, %s)", name, txt);
     }

   //no idea which mp udisks will unmount ... check all of them!
   _device_mounts_get(dev);
}

EOLIAN static Eina_Bool
_emous_device_udisks_emous_device_umount(Eo *obj, Emous_Device_UDisks_Data *pd)
{
   Eldbus_Message *msg;
   Eldbus_Message_Iter *array, *main_iter;

   msg = eldbus_proxy_method_call_new(pd->proxy, "Unmount");
   main_iter = eldbus_message_iter_get(msg);

   if (!eldbus_message_iter_arguments_append(main_iter, "a{sv}", &array))
     ERR("Failed to construct arguments!");

   eldbus_message_iter_container_close(main_iter, array);
   if (!!eldbus_proxy_send(pd->proxy, msg, _umount_cb, obj, -1))
     {
        _device_state_change(obj, EMOUS_DEVICE_STATE_UNMOUNT_REQ);
        return EINA_TRUE;
     }
   else
     return EINA_FALSE;
}

EOLIAN static Emous_Device_State
_emous_device_udisks_emous_device_state_get(Eo *obj EINA_UNUSED, Emous_Device_UDisks_Data *pd)
{
    return pd->state;
}

EOLIAN static const char *
_emous_device_udisks_emous_device_displayname_get(Eo *obj EINA_UNUSED, Emous_Device_UDisks_Data *pd)
{
    return pd->displayname;
}

EOLIAN static Eina_List *
_emous_device_udisks_emous_device_mountpoints_get(Eo *obj EINA_UNUSED, Emous_Device_UDisks_Data *pd)
{
    return pd->mountpoints;
}

EOLIAN static long
_emous_device_udisks_emous_device_size_get(Eo *obj EINA_UNUSED, Emous_Device_UDisks_Data *pd)
{
    return pd->size;
}

EOLIAN static Emous_Device_Type
_emous_device_udisks_emous_device_type_get(Eo *obj EINA_UNUSED, Emous_Device_UDisks_Data *pd)
{
   Device_Entry *entry;

   entry = eina_hash_find(drives, pd->device);

   if (!entry) return EMOUS_DEVICE_TYPE_UNKOWN;

   return entry->t;
}


#include "emous_device_udisks.eo.x"