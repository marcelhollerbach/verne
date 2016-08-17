#include "efm_priv.h"

typedef struct
{
   Efm_Fs_File *origin;
   Eio_Monitor *mon; //< The eio monitor which is started in the directory
   Eio_File *file; //< The eio file as long as the ls is running

   Eina_Hash *file_icons; // < Hash table of all listed FM_Monitor_Files
   Efm_Filter *filter;

} Efm_Fs_Monitor_Data;

typedef struct {
  Eo *monitor;
} Efm_Monitor_Eio_Job;

#define MARK_POPULATED(o) efl_key_data_set(o, "__populated", ((void*)1));
#define UNMARK_POPULATED(o) efl_key_data_set(o, "__populated", NULL);
#define CHECK_POPULATED(o) (efl_key_data_get(o, "__populated") != NULL)

static inline Eina_Bool
_take_filter(Efm_Monitor *mon EINA_UNUSED, Efm_Fs_Monitor_Data *pd, Efm_File *file)
{
   if (!pd->filter) return EINA_TRUE;

   return efm_filter_matches(pd->filter, file);
}

static void
_file_del(void *data, const Eo_Event *event)
{
   Efm_Fs_Monitor_Data *pd;

   pd = efl_data_scope_get(data, EFM_FS_MONITOR_CLASS);
   eina_hash_del_by_data(pd->file_icons, event->object);
}

static void
_add(Efm_Monitor *mon, const char *file)
{
   const char *path, *filename;
   Efm_File *ef;
   Efm_Fs_Monitor_Data *pd;
   pd = efl_data_scope_get(mon, EFM_FS_MONITOR_CLASS);

   filename = ecore_file_file_get(file);

   ef = efm_file_child_get(pd->origin, filename);

   if (!ef)
     {
       INF("Creation of %s failed, this is ... strange", file);
       return;
     }

   path = efm_file_path_get(ef);
   efl_event_callback_add(ef, EFM_FILE_EVENT_INVALID, _file_del, mon);

   //prevent a file from beeing populated twice
   if (eina_hash_find(pd->file_icons, path)) return;

   eina_hash_add(pd->file_icons, path, ef);

   if (!_take_filter(mon, pd, ef))
     {
        UNMARK_POPULATED(ef);
        return;
     }

   efl_event_callback_call(mon, EFM_MONITOR_EVENT_FILE_ADD, ef);
   MARK_POPULATED(ef);
}

void
_error(Efm_Monitor *efm)
{
   //tell everyone that this monitor is crashed now
   efl_event_callback_call(efm, EFM_MONITOR_EVENT_ERROR, NULL);
   //delete the monitor
   efl_del(efm);
}

static void
_refresh_files(Efm_Monitor *mon, Efm_Fs_Monitor_Data *pd)
{
   Eina_Iterator *it;
   Efm_File *ef;

   it = eina_hash_iterator_data_new(pd->file_icons);

   EINA_ITERATOR_FOREACH(it, ef)
     {
        Eina_Bool populated;
        populated = CHECK_POPULATED(ef);
        // this is a hidden file
        if (_take_filter(mon, pd, ef) && !populated)
          {
             MARK_POPULATED(ef);
             efl_event_callback_call(mon, EFM_MONITOR_EVENT_FILE_ADD, ef);
          }
        else if (!_take_filter(mon, pd, ef) && populated)
          {
             UNMARK_POPULATED(ef);
             efl_event_callback_call(mon, EFM_MONITOR_EVENT_FILE_HIDE, ef);
          }
     }
}

static void
_fm_action(void *data EINA_UNUSED, Efm_Monitor *mon, const char *file, Fm_Action action)
{
   if (action == ADD)
     {
       _add(mon, file);
     }
   else if (action == SELFDEL)
     {
       //delete the monitor
       efl_del(mon);
     }
   else
     {
        _error(mon);
     }
}

static void
_filter_changed_cb(void *data, const Eo_Event *event EINA_UNUSED)
{
   Efm_Fs_Monitor_Data *pd = efl_data_scope_get(data, EFM_FS_MONITOR_CLASS);

   _refresh_files(data, pd);
}

EOLIAN static void
_efm_fs_monitor_efm_monitor_filter_set(Eo *obj, Efm_Fs_Monitor_Data *pd, Efm_Filter *filter)
{

   if (filter != pd->filter)
     {
        if (pd->filter)
          {
             efl_event_callback_del(pd->filter, EFM_FILTER_EVENT_FILTER_CHANGED, _filter_changed_cb, obj);
             efl_wref_del(pd->filter, &pd->filter);
          }

        pd->filter = filter;
        if (pd->filter)
          {
             efl_event_callback_add(pd->filter, EFM_FILTER_EVENT_FILTER_CHANGED, _filter_changed_cb, obj);
             efl_wref_add(pd->filter, &pd->filter);
          }
        _refresh_files(obj, pd);
     }

}

EOLIAN static Efm_Filter *
_efm_fs_monitor_efm_monitor_filter_get(Eo *obj EINA_UNUSED, Efm_Fs_Monitor_Data *pd)
{
   return pd->filter;
}

static Eina_Bool
_eio_filter_cb(void *data EINA_UNUSED, Eio_File *handler EINA_UNUSED, const char *file EINA_UNUSED)
{
   // We take everything, so the configuration can be easily changed later
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
   Efm_Fs_Monitor_Data *pd;
   Efm_Monitor *mon;
   const char *dir;
   job = data;

   if (!job->monitor)
     return;

   pd = efl_data_scope_get(job->monitor, EFM_FS_MONITOR_CLASS);
   //free the eio file
   pd->file = NULL;

   //free the job of the monitor
   mon = job->monitor;
   efl_wref_del(job->monitor, &job->monitor);
   free(job);

   //call changed event
   efl_event_callback_call(mon, EFM_MONITOR_EVENT_LISTING_DONE, NULL);

   //start monitoring
   dir = efm_file_path_get(pd->origin);
   pd->mon = eio_monitor_stringshared_add(dir);
   fm_monitor_add(mon, pd->mon, _fm_action);

}

static void
_eio_error_cb(void *data, Eio_File *handler EINA_UNUSED, int error EINA_UNUSED)
{
   Efm_Monitor_Eio_Job *job;
   Efm_Fs_Monitor_Data *pd;

   job = data;

   if (!job->monitor)
     return;

   pd = efl_data_scope_get(job->monitor, EFM_FS_MONITOR_CLASS);
   //free the eio file
   pd->file = NULL;

   _error(job->monitor);

   efl_wref_del(job->monitor, &job->monitor);
   free(job);
}

static void
_del_cb(void *data)
{
   efl_unref(data);
}

EOLIAN static Efl_Object*
_efm_fs_monitor_efl_object_constructor(Eo *obj, Efm_Fs_Monitor_Data *pd)
{
   Efl_Object *construct;

   construct = efl_constructor(efl_super(obj, EFM_FS_MONITOR_CLASS));

   pd->file_icons = eina_hash_string_superfast_new(_del_cb);

   return construct;
}

EOLIAN static Efl_Object*
_efm_fs_monitor_efl_object_finalize(Eo *obj, Efm_Fs_Monitor_Data *pd)
{
   Efl_Object *base;
   Efm_Monitor_Eio_Job *job;
   const char *dir;

   base = efl_finalize(efl_super(obj, EFM_FS_MONITOR_CLASS));

   if (!pd->origin) {
     ERR("monitor does not have a set path");
     return NULL;
   }

   dir = efm_file_path_get(pd->origin);

   //prepare the listing
   job = calloc(1, sizeof(Efm_Monitor_Eio_Job));
   job->monitor = obj;

   efl_wref_add(obj, &job->monitor);

   //start the listing
   pd->file = eio_file_ls(dir, _eio_filter_cb, _eio_main_cb,
                           _eio_done_cb, _eio_error_cb, job);
   return base;
}

static void
_inv_file_cb(void *data, const Eo_Event *event EINA_UNUSED)
{
   efl_event_callback_call(data, EFM_MONITOR_EVENT_ERROR, NULL);
}

EOLIAN static void
_efm_fs_monitor_install(Eo *obj, Efm_Fs_Monitor_Data *pd, Efm_Fs_File *file, Efm_Filter *filter)
{
   pd->origin = file;
   efl_ref(pd->origin);
   efl_event_callback_add(pd->origin, EFM_FILE_EVENT_INVALID, _inv_file_cb, obj);
   efm_monitor_filter_set(obj, filter);
}

EOLIAN static Efm_File*
_efm_fs_monitor_efm_monitor_file_get(Eo *obj EINA_UNUSED, Efm_Fs_Monitor_Data *pd)
{
   return pd->origin;
}

EOLIAN static void
_efm_fs_monitor_efl_object_destructor(Eo *obj, Efm_Fs_Monitor_Data *pd)
{

   if (pd->file)
     eio_file_cancel(pd->file);

   if (pd->filter)
     {
        efl_event_callback_del(pd->filter, EFM_FILTER_EVENT_FILTER_CHANGED, _filter_changed_cb, obj);
        efl_wref_del(pd->filter, &pd->filter);
     }

   efl_unref(pd->origin);

   fm_monitor_del(obj, pd->mon);
   eio_monitor_del(pd->mon);

   eina_hash_free(pd->file_icons);

   efl_destructor(efl_super(obj, EFM_FS_MONITOR_CLASS));
}
#include "efm_fs_monitor.eo.x"
