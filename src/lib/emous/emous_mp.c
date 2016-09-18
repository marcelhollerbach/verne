#include "emous_priv.h"

void
_emous_mount_add(const char *type, const char *mount_point, const char *source)
{
   Emous_Manager *manager;
   Emous_MountPoint *mp;

   ecore_thread_main_loop_begin();

   DBG("Adding %s %s %s", type, mount_point, source);

   mp = calloc(1, sizeof(Emous_MountPoint));

   mp->source = source;
   mp->mount_point = mount_point;
   mp->fs_type = type;

   manager = emous_manager_object_get(EMOUS_MANAGER_CLASS);
   efl_event_callback_call(manager, EMOUS_MANAGER_EVENT_MOUNT_ADD, mp);

   free(mp);

   ecore_thread_main_loop_end();
}

void
_emous_mount_del(const char *mount_point)
{
   Emous_Manager *manager;

   ecore_thread_main_loop_begin();

   manager = emous_manager_object_get(EMOUS_MANAGER_CLASS);

   efl_event_callback_call(manager, EMOUS_MANAGER_EVENT_DEVICE_DEL, (void*)mount_point);

   ecore_thread_main_loop_end();
}
