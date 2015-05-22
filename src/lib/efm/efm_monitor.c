#define EFL_BETA_API_SUPPORT
#include <Eo.h>
#include <Ecore.h>
#include <Eio.h>

#include "efm_priv.h"

typedef struct
{
   const char *directory; //< The directory this monitor listens to
   Eio_Monitor *mon; //< The eio monitor which is started in the directory
   Eio_File *file; //< The eio file as long as the ls is running

   Eina_Hash *file_icons; // < Hash table of all listed FM_Monitor_Files

   struct {
      Eina_Bool only_folder;
      Eina_Bool hidden_files;
   } config;

} Efm_Monitor_Data;

static void
_add(Efm_Monitor *mon, const char *file)
{
   const char *path;
   Efm_File *ef;
   Efm_Monitor_Data *pd;
   Eina_Bool result;

   path = eina_stringshare_add(file);

   ef = eo_add(EFM_FILE_CLASS, mon, result = efm_file_obj_generate(path));

   if (!ef)
     return;

   if (!result)
     {
       eo_del(ef);
     }

   pd = eo_data_scope_get(mon, EFM_MONITOR_CLASS);

   eina_hash_add(pd->file_icons, path, ef);
   eo_do(mon, eo_event_callback_call(EFM_MONITOR_EVENT_FILE_ADD, ef));
}

static void
_del(Efm_Monitor *mon, const char *file)
{
   Efm_Monitor_Data *pd;
   Efm_File *f;
   const char *files;

   files = eina_stringshare_add(file);
   pd = eo_data_scope_get(mon, EFM_MONITOR_CLASS);
   f = eina_hash_find(pd->file_icons, files);

   eo_do(mon, eo_event_callback_call(EFM_MONITOR_EVENT_FILE_DEL, f));
   eina_hash_del(pd->file_icons, files, NULL);
}

void
_error(Efm_Monitor *efm)
{
   eo_do(efm, eo_event_callback_call(EFM_MONITOR_EVENT_ERROR, NULL));
}

EOLIAN static const char*
_efm_monitor_path_get(Eo *obj EINA_UNUSED, Efm_Monitor_Data *pd)
{
   return pd->directory;
}

EOLIAN static void
_efm_monitor_config_hidden_files_set(Eo *obj, Efm_Monitor_Data *pd, Eina_Bool hidden)
{

}

EOLIAN static Eina_Bool
_efm_monitor_config_hidden_files_get(Eo *obj EINA_UNUSED, Efm_Monitor_Data *pd)
{
   return pd->config.hidden_files;
}

EOLIAN static void
_efm_monitor_config_only_folder_set(Eo *obj, Efm_Monitor_Data *pd, Eina_Bool folder)
{

}

EOLIAN static Eina_Bool
_efm_monitor_config_only_folder_get(Eo *obj EINA_UNUSED, Efm_Monitor_Data *pd)
{
   return pd->config.only_folder;
}

static Eina_Bool
_eio_filter_cb(void *data EINA_UNUSED, Eio_File *handler EINA_UNUSED, const char *file EINA_UNUSED)
{
   //We take everything, so the configuration can be easily changed later
   return EINA_TRUE;
}

static void
_eio_main_cb(void *data, Eio_File *handler EINA_UNUSED, const char *file)
{
   _add(data, file);
}

static void
_fm_action(void *data EINA_UNUSED, Efm_Monitor *mon, const char *file, Fm_Action action)
{
   Efm_Monitor_Data *pd;
   pd = eo_data_scope_get(mon, EFM_MONITOR_CLASS);

   if (action == ADD)
     {
       _add(mon, file);
     }
   else if (action == DEL)
     {
       _del(mon, file);
     }
   else
     {
        _error(mon);
     }
}

static void
_eio_done_cb(void *data, Eio_File *handler EINA_UNUSED)
{
  Efm_Monitor_Data *pd;

  pd = eo_data_scope_get(data, EFM_MONITOR_CLASS);

  pd->mon = eio_monitor_stringshared_add(pd->directory);
  fm_monitor_add(data, pd->mon, _fm_action);
  pd->file = NULL;
  eo_unref(data);
}

static void
_eio_error_cb(void *data, Eio_File *handler EINA_UNUSED, int error EINA_UNUSED)
{
  _error(data);
  eo_unref(data);
}

void
_mon_hash_free(void *data)
{
   eo_del(data);
}

EOLIAN static Efm_Monitor *
_efm_monitor_start(Eo *obj EINA_UNUSED, void *data EINA_UNUSED, const char *directory, Eina_Bool hidden_files, Eina_Bool only_folder)
{
   Efm_Monitor *mon;
   Efm_Monitor_Data *pd;

   mon = eo_add(EFM_MONITOR_CLASS, NULL);

   pd = eo_data_scope_get(mon, EFM_MONITOR_CLASS);

   pd->config.only_folder = only_folder;
   pd->config.hidden_files = hidden_files;

   pd->directory = eina_stringshare_add(directory);

   pd->file_icons = eina_hash_stringshared_new(_mon_hash_free);

   eo_ref(mon);
   pd->file = eio_file_ls(pd->directory, _eio_filter_cb, _eio_main_cb,
                           _eio_done_cb, _eio_error_cb, mon);

   return mon;
}

EOLIAN static void
_efm_monitor_stop(Eo *obj, Efm_Monitor_Data *pd)
{
   eo_do(obj, eo_event_freeze());
   eo_del(obj);
}


static void
_efm_monitor_eo_base_destructor(Eo *obj EINA_UNUSED, Efm_Monitor_Data *pd)
{
   if (pd->file)
     eio_file_cancel(pd->file);

   fm_monitor_del(obj, pd->mon);
   eio_monitor_del(pd->mon);
   eina_stringshare_del(pd->directory);
   eina_hash_free(pd->file_icons);

   eo_do_super(obj, EFM_MONITOR_CLASS, eo_destructor());
}
#include "efm_monitor.eo.x"
