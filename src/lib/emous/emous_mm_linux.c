#include "emous_priv.h"
#include <libmount.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <poll.h>
#include <fcntl.h>

#define MONITOR_PATH "/proc/self/mounts"

static struct libmnt_table *mnt_table_old;
static Ecore_Thread *t;

void
mnt_event(void)
{
   int change;
   struct libmnt_table *current;
   struct libmnt_iter *itr;
   struct libmnt_fs *new, *old;
   struct libmnt_tabdiff *diff;

   current = mnt_new_table();
   if (!current)
     {
        ERR("Failed to alloc mnt table!");
        return;
     }

   if (mnt_table_parse_file(current, "/proc/self/mountinfo"))
     {
        ERR("Failed to parse /proc/self/mountinfo!!!");
        return;
     }

   itr = mnt_new_iter(MNT_ITER_BACKWARD);
   if (!itr)
     {
        ERR("Failed to alloc iterator");
        return;
     }

   diff = mnt_new_tabdiff();

   change = mnt_diff_tables(diff, mnt_table_old, current);

   if (change <= 0)
     {
        ERR("Failed to diff the tables");
        return;
     }

   while(!mnt_tabdiff_next_change(diff, itr, &old, &new, &change))
     {
        const char *src = NULL, *mnt_point, *type;
        const char *omnt_point = NULL;

        // null if the entry is gone
        if (new)
          src = mnt_fs_get_source(new);

        mnt_point = mnt_fs_get_target(new);
        type = mnt_fs_get_fstype(new);

        if (old)
          omnt_point = mnt_fs_get_target(old);

        switch(change)
          {
            case MNT_TABDIFF_MOUNT:
              _emous_mount_add(type, mnt_point, src);
              break;
            case MNT_TABDIFF_UMOUNT:
              _emous_mount_del(omnt_point);
              break;
            case MNT_TABDIFF_REMOUNT:
            case MNT_TABDIFF_MOVE:
              ERR("Remount and move event, what should we do ?!");
              break;
          }
     }
   mnt_free_iter(itr);
   mnt_free_tabdiff(diff);
   mnt_free_table(mnt_table_old);
   mnt_table_old = current;
}

void
_change_cb(void *data EINA_UNUSED, Ecore_Thread *th EINA_UNUSED, void *msg_data EINA_UNUSED)
{
   mnt_event();
}

void
_check_cb(void *data EINA_UNUSED, Ecore_Thread *thread)
{
    int mfd = open(MONITOR_PATH, O_RDONLY, 0);

    if (mfd < 0)
      {
        ERR("Failed to open "MONITOR_PATH);
        return;
      }

    fd_set rfds;
    struct timeval tv;
    int rv;

    FD_ZERO(&rfds);
    FD_SET(mfd, &rfds);
    tv.tv_sec = 2;
    tv.tv_usec = 0;

    while ((rv = select(mfd+1, NULL, NULL, &rfds, &tv)) >= 0) {
        if (FD_ISSET(mfd, &rfds))
          {
             ecore_thread_feedback(thread, NULL);
          }

        FD_ZERO(&rfds);
        FD_SET(mfd, &rfds);
        tv.tv_sec = 5;
        tv.tv_usec = 0;
        if (ecore_thread_check(thread))
          break;
    }
    if (!ecore_thread_check(thread))
      ERR("Polling thread died. but the thread is not cancled ... this is possible a bug!!!");
}

void
_emous_mm_init(void)
{
   // first run just a empty table
   mnt_table_old = mnt_new_table();

   /*
    * This will emit all events
    * for all _now_ mounted things
    */
   mnt_event();

   t = ecore_thread_feedback_run(_check_cb, _change_cb, NULL, NULL, NULL, EINA_TRUE);
}

void
_emous_mm_shutdown(void)
{
   mnt_free_table(mnt_table_old);
   ecore_thread_cancel(t);
}