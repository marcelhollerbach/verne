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

typedef struct {
  Eo *monitor;
} Efm_Monitor_Eio_Job;

#define MARK_POPULATED eo_key_data_set("__populated", ((void*)1));
#define UNMARK_POPULATED eo_key_data_del("__populated");
#define CHECK_POPULATED (eo_key_data_get("__populated") != NULL)

static inline Eina_Bool
_take_filter(Efm_Monitor *mon EINA_UNUSED, Efm_Monitor_Data *pd, Efm_File *file)
{
   Eina_Bool dir;
   const char *filename;

   if (pd->config.only_folder &&
       !eo_do_ret(file, dir, efm_file_is_type(EFM_FILE_TYPE_DIRECTORY)))
     return EINA_FALSE;

   if (!pd->config.hidden_files &&
       eo_do_ret(file, filename, efm_file_filename_get())[0] == '.')
     return EINA_FALSE;

   return EINA_TRUE;
}

static void
_add(Efm_Monitor *mon, const char *file)
{
   const char *path;
   Efm_File *ef;
   Efm_Monitor_Data *pd;
   path = eina_stringshare_add(file);

   eo_do(EFM_FILE_CLASS, ef = efm_file_generate(path));

   if (!ef)
     {
       ERR("Creation of %s failed, this is ... strange", file);
       return;
     }

   pd = eo_data_scope_get(mon, EFM_MONITOR_CLASS);

   eina_hash_add(pd->file_icons, path, ef);

   if (!_take_filter(mon, pd, ef))
     return;

   eo_do(mon, eo_event_callback_call(EFM_MONITOR_EVENT_FILE_ADD, ef));
   eo_do(ef, MARK_POPULATED);
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
   eo_do(f, UNMARK_POPULATED);
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

static void
_refresh_files(Efm_Monitor *mon, Efm_Monitor_Data *pd)
{
   Eina_Iterator *it;
   Efm_File *ef;

   it = eina_hash_iterator_data_new(pd->file_icons);

   EINA_ITERATOR_FOREACH(it, ef)
     {
        Eina_Bool populated;
        eo_do(ef, populated = CHECK_POPULATED ? EINA_TRUE : EINA_FALSE);
        //this is a hidden file
        if (_take_filter(mon, pd, ef) && !populated)
          {
             eo_do(ef, MARK_POPULATED);
             eo_do(mon, eo_event_callback_call(EFM_MONITOR_EVENT_FILE_ADD, ef));
          }
        else if (!_take_filter(mon, pd, ef) && populated)
          {
             eo_do(ef, UNMARK_POPULATED);
             eo_do(mon, eo_event_callback_call(EFM_MONITOR_EVENT_FILE_DEL, ef));
          }
     }
}

EOLIAN static void
_efm_monitor_config_hidden_files_set(Eo *obj, Efm_Monitor_Data *pd, Eina_Bool hidden)
{
   if (pd->config.hidden_files == hidden)
     return;
   pd->config.hidden_files = hidden;
   _refresh_files(obj, pd);
}

EOLIAN static Eina_Bool
_efm_monitor_config_hidden_files_get(Eo *obj EINA_UNUSED, Efm_Monitor_Data *pd)
{
   return pd->config.hidden_files;
}

EOLIAN static void
_efm_monitor_config_only_folder_set(Eo *obj, Efm_Monitor_Data *pd, Eina_Bool folder)
{
   if (pd->config.only_folder == folder)
     return;

   pd->config.only_folder = folder;

   _refresh_files(obj, pd);
}

EOLIAN static Eina_Bool
_efm_monitor_config_only_folder_get(Eo *obj EINA_UNUSED, Efm_Monitor_Data *pd)
{
   return pd->config.only_folder;
}

static void
_fm_action(void *data EINA_UNUSED, Efm_Monitor *mon, const char *file, Fm_Action action)
{
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

static Eina_Bool
_eio_filter_cb(void *data EINA_UNUSED, Eio_File *handler EINA_UNUSED, const char *file EINA_UNUSED)
{
   //We take everything, so the configuration can be easily changed later
   return EINA_TRUE;
}

static void
_eio_main_cb(void *data, Eio_File *handler EINA_UNUSED, const char *file)
{
  Efm_Monitor_Eio_Job *job;

  job = data;

  if (!job->monitor)
    return;

   _add(job->monitor, file);
}

static void
_eio_done_cb(void *data, Eio_File *handler EINA_UNUSED)
{
  Efm_Monitor_Eio_Job *job;
  Efm_Monitor_Data *pd;

   job = data;

   if (!job->monitor)
     return;

   pd = eo_data_scope_get(job->monitor, EFM_MONITOR_CLASS);

   pd->mon = eio_monitor_stringshared_add(pd->directory);
   fm_monitor_add(job->monitor, pd->mon, _fm_action);
   pd->file = NULL;

   eo_do(job->monitor, eo_wref_del(&job->monitor));
   free(job);
}

static void
_eio_error_cb(void *data, Eio_File *handler EINA_UNUSED, int error EINA_UNUSED)
{
   Efm_Monitor_Eio_Job *job;

   job = data;

   if (job->monitor)
     return;

   _error(job->monitor);

   eo_do(job->monitor, eo_wref_del(&job->monitor));
   free(job);
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

   {
     Efm_Monitor_Eio_Job *job;

     job = calloc(1, sizeof(Efm_Monitor_Eio_Job));
     eo_do(mon, eo_wref_add(&job->monitor));

     pd->file = eio_file_ls(pd->directory, _eio_filter_cb, _eio_main_cb,
                           _eio_done_cb, _eio_error_cb, job);
   }
   return mon;
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
