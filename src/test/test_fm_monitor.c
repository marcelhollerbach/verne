#define EFL_BETA_API_SUPPORT
#define EFL_EO_API_SUPPORT

#include <Elementary.h>
#include <Efm.h>

static FM_Monitor *mon;

void
add_cb(void *data EINA_UNUSED, FM_Monitor *mon EINA_UNUSED, FM_Monitor_File *icon)
{
   printf("FILE PATH: %s\n", efm_file_path_get(icon));
   printf("FILE FILE %s\n", efm_file_filename_get(icon));
   printf("FILE FILEENDING %s\n", efm_file_fileending_get(icon));
}

void
del_cb(void *data EINA_UNUSED, FM_Monitor *mon EINA_UNUSED, FM_Monitor_File *icon)
{
   printf("DEL... %s\n", efm_file_path_get(icon));
}

void
err_cb(void *data EINA_UNUSED, FM_Monitor *mon EINA_UNUSED)
{
   printf("ERR...\n");
}

void
sdel_cb(void *data EINA_UNUSED, FM_Monitor *mon EINA_UNUSED)
{
   printf("SDEL...\n");
}

static Eina_Bool
func(void *data EINA_UNUSED)
{
   mon = fm_monitor_start("/home/marcel/", add_cb, del_cb, NULL,
         err_cb, sdel_cb, NULL, EINA_FALSE, EINA_FALSE);

   return EINA_FALSE;
}

static Eina_Bool
del(void *data EINA_UNUSED)
{
   fm_monitor_stop(mon);
   return EINA_FALSE;
}

static Eina_Bool
del2(void *data EINA_UNUSED)
{
   ecore_main_loop_quit();
   return EINA_FALSE;
}

int
main(int argc EINA_UNUSED, char **argv EINA_UNUSED)
{
   FM_Monitor *fm;
   fm_monitor_init();

   fm = fm_monitor_start("/home/marcel/", add_cb, del_cb, NULL,
         err_cb, sdel_cb, NULL, EINA_FALSE, EINA_FALSE);
   fm_monitor_stop(fm);

   ecore_timer_add(2.0, func, NULL);
   ecore_timer_add(5.0, del, NULL);
   ecore_timer_add(20.0, del2, NULL);

   ecore_main_loop_begin();
   fm_monitor_shutdown();
   return 1;
}
