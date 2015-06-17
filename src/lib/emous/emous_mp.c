#include "emous_priv.h"

void
_emous_mount_add(const char *type, const char *mount_point, const char *source)
{
   Emous_Manager *manager;
   Emous_MountPoint *mp;

   DBG("Adding %s %s %s", type, mount_point, source);

   mp = calloc(1, sizeof(Emous_MountPoint));

   mp->source = source;
   mp->mount_point = mount_point;
   mp->fs_type = type;

   eo_do(EMOUS_MANAGER_CLASS, manager = emous_manager_object_get());
   eo_do(manager, eo_event_callback_call(EMOUS_MANAGER_EVENT_MOUNT_ADD, mp));//FIXME

   free(mp);
}

void
_emous_mount_del(const char *mount_point)
{
   Emous_Manager *manager;

   eo_do(EMOUS_MANAGER_CLASS, manager = emous_manager_object_get());

   eo_do(manager, eo_event_callback_call(EMOUS_MANAGER_EVENT_DEVICE_DEL, (void*)mount_point));
}
