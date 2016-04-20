#define EFL_BETA_API_SUPPORT
#include <Eo.h>
#include <Ecore.h>

#include "efm_priv.h"

static Ecore_Thread *fs_query;
static Eina_List *query_stuff;
static Eina_Lock readlock;

typedef struct {
    const char *path;
    const char *mimetype;

    Eo *obj;
} Thread_Job;

typedef struct
{
    const char *filename;
    const char *path;
    const char *fileending;
    const char *mimetype;

    struct stat st;
    Efm_File_Stat stat;
    Eio_Monitor *file_mon;
} Efm_Fs_File_Data;


static void _mime_thread_fireup(void);
static Eina_Hash *watch_files;

static Ecore_Event_Handler *handler_mod;
static Ecore_Event_Handler *handler_del;
static Ecore_Event_Handler *handler_err;

static void
_fs_cb(void *dat EINA_UNUSED, Ecore_Thread *thread)
{
    Eina_List *copy;
    Thread_Job *data;

    while(!ecore_thread_check(thread) && query_stuff)
      {
        // take a local copy of the list
        eina_lock_take(&readlock);
        copy = query_stuff;
        query_stuff = NULL;
        eina_lock_release(&readlock);

        EINA_LIST_FREE(copy, data)
          {
             if (!data->obj)
               continue;

             const char *mime_type;

             mime_type = efreet_mime_globs_type_get(data->path);
             if (!mime_type) mime_type = efreet_mime_fallback_type_get(data->path);

             data->mimetype = mime_type;
             ecore_thread_feedback(thread, data);
          }
      }
}

static void
_notify_cb(void *data EINA_UNUSED, Ecore_Thread *et EINA_UNUSED, void *pass)
{
    Efm_File *file;
    Efm_Fs_File_Data *pd;
    Thread_Job *job;

    job = pass;

    // check if our object is still valid
    if (!job->obj)
      return;

    // we have a still valid object
    file = job->obj;
    pd = eo_data_scope_get(file, EFM_FS_FILE_CLASS);

    // set the mimetpye
    pd->mimetype = job->mimetype;

    // Remove weak reference
    eo_wref_del(job->obj, &job->obj);
    //unref stringshare
    eina_stringshare_del(job->path);
    // notify that this efm_file is ready for the world
    eo_event_callback_call(file, EFM_FILE_EVENT_FSQUERY_DONE, NULL);
    free(job);
}


static void
_end_cb(void *data EINA_UNUSED, Ecore_Thread *et EINA_UNUSED)
{
    fs_query = NULL;
    /* FIXME There are cases that the mimethread dies but there is still work*/
    if (query_stuff)
      _mime_thread_fireup();
}

static void
_cancel_cb(void *data EINA_UNUSED, Ecore_Thread *th  EINA_UNUSED)
{
    fs_query = NULL;
}

static void
_mime_thread_fireup(void)
{
   fs_query = ecore_thread_feedback_run(_fs_cb, _notify_cb,
                                        _end_cb, _cancel_cb,
                                        NULL, EINA_FALSE);
}

static void
_scheudle(Eo *obj, Efm_Fs_File_Data *pd)
{
    Thread_Job *job;

    job = calloc(1, sizeof(Thread_Job));

    eo_wref_add(obj, &job->obj);
    job->path = eina_stringshare_ref(pd->path);
    eina_lock_take(&readlock);
    query_stuff = eina_list_append(query_stuff, job);
    eina_lock_release(&readlock);
    // if there is a running thread the list will be took
    if (!fs_query)
      _mime_thread_fireup();
}

EOLIAN static const char *
_efm_fs_file_efm_file_filename_get(Eo *obj EINA_UNUSED, Efm_Fs_File_Data *pd)
{
    return pd->filename;
}

EOLIAN static const char *
_efm_fs_file_efm_file_path_get(Eo *obj EINA_UNUSED, Efm_Fs_File_Data *pd)
{
    return pd->path;
}

EOLIAN static const char *
_efm_fs_file_efm_file_fileending_get(Eo *obj EINA_UNUSED, Efm_Fs_File_Data *pd)
{
    return pd->fileending;
}

EOLIAN static const char *
_efm_fs_file_efm_file_mimetype_get(Eo *obj EINA_UNUSED, Efm_Fs_File_Data *pd)
{
    return pd->mimetype;
}

EOLIAN static Efm_File_Stat *
_efm_fs_file_efm_file_stat_get(Eo *obj EINA_UNUSED, Efm_Fs_File_Data *pd)
{
    return &pd->stat;
}

EOLIAN static Eina_Bool
_efm_fs_file_efm_file_is_type(Eo *obj EINA_UNUSED, Efm_Fs_File_Data *pd, Efm_File_Type type)
{
   if (type == EFM_FILE_TYPE_SOCKET && S_ISSOCK(pd->st.st_mode))
     return EINA_TRUE;
   else if (type == EFM_FILE_TYPE_FIFO && S_ISFIFO(pd->st.st_mode))
     return EINA_TRUE;
   else if (type == EFM_FILE_TYPE_DIRECTORY && S_ISDIR(pd->st.st_mode))
     return EINA_TRUE;
   else if (type == EFM_FILE_TYPE_SYM_LINK && S_ISLNK(pd->st.st_mode))
     return EINA_TRUE;
   else if (type == EFM_FILE_TYPE_REGULAR_FILE && S_ISREG(pd->st.st_mode))
     return EINA_TRUE;
   else if (type == EFM_FILE_TYPE_CHARACTER_DEVICE && S_ISCHR(pd->st.st_mode))
     return EINA_TRUE;
   else if (type == EFM_FILE_TYPE_BLOCK_DEVICE && S_ISBLK(pd->st.st_mode))
     return EINA_TRUE;
   return EINA_FALSE;
}

static void
_attributes_update(Eo *obj EINA_UNUSED, Efm_Fs_File_Data *pd)
{
    // parse stat to the eo struct
    pd->stat.uid = pd->st.st_uid;
    pd->stat.gid = pd->st.st_gid;
    pd->stat.size = pd->st.st_size;

    pd->stat.mode = pd->st.st_mode;

    pd->stat.atime = pd->st.st_atim.tv_sec;
    pd->stat.ctime = pd->st.st_ctim.tv_sec;
    pd->stat.mtime = pd->st.st_mtim.tv_sec;
}

EOLIAN static void
_efm_fs_file_generate(Eo *obj, Efm_Fs_File_Data *pd EINA_UNUSED, const char *filename)
{
    EINA_SAFETY_ON_NULL_RETURN(watch_files);

    // get the stat
    if (stat(filename, &pd->st) < 0)
      {
         ERR("Failed to access %s\n", filename);
         return;
      }

    _attributes_update(obj, pd);

    // safe this name
    pd->path = eina_stringshare_add(filename);

    // get the filename
    pd->fileending = pd->filename = ecore_file_file_get(pd->path);

    // parse the fileending
    do {
        //check if we are at the end
        if (pd->fileending[0] == '\0')
          {
             pd->fileending = NULL;
             break;
          }
        pd->fileending ++;
    } while(pd->fileending[0] != '.');

    if (pd->fileending) pd->fileending ++; //skip the .

    if (!S_ISREG(pd->st.st_mode))
      pd->mimetype = efreet_mime_special_type_get(pd->path);
    else
      _scheudle(obj, pd);

    pd->file_mon = eio_monitor_add(pd->path);
    eina_hash_add(watch_files, &pd->file_mon, obj);
}

EOLIAN static void
_efm_fs_file_eo_base_destructor(Eo *obj, Efm_Fs_File_Data *pd)
{
    DBG("Remove %p (%s)", obj, pd->path);

    eina_stringshare_del(pd->path);
    eio_monitor_del(pd->file_mon);
    //if this file lives longer than the lib XXX this should never happen
    if (watch_files)
      eina_hash_del(watch_files, &pd->file_mon, obj);
    eo_destructor(eo_super(obj, EFM_FS_FILE_CLASS));
}

EOLIAN static Eo_Base *
_efm_fs_file_eo_base_finalize(Eo *obj, Efm_Fs_File_Data *pd)
{
    Eo *finalize;

    if (!pd->filename) return NULL;

    finalize = eo_finalize(eo_super(obj, EFM_FS_FILE_CLASS));

    return finalize;
}

static Eina_Bool
_mod_cb(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
    Eio_Monitor_Event *ev = event;
    Efm_File *f;
    Efm_Fs_File_Data *pd;

    f = eina_hash_find(watch_files, &ev->monitor);

    if (!f) return EINA_TRUE;

    pd = eo_data_scope_get(f, EFM_FS_FILE_CLASS);

    if (stat(pd->path, &pd->st) < 0)
      {
         return EINA_TRUE;
      }

    _attributes_update(f, pd);

    DBG("File %p got modified", f);

    eo_event_callback_call(f, EFM_FILE_EVENT_CHANGED, NULL);

    return EINA_TRUE;
}

static Eina_Bool
_file_del_cb(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Eio_Monitor_Event *ev = event;
   Efm_File *f;

   f = eina_hash_find(watch_files, &ev->monitor);

   if (!f) return EINA_TRUE;

   DBG("File %s got deleted in storage", ev->filename);

   eo_event_callback_call(f, EFM_FILE_EVENT_INVALID, "File Deleted");

   return EINA_TRUE;
}

static Eina_Bool
_error_cb(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Eio_Monitor_Event *ev = event;
   Efm_File *f;

   f = eina_hash_find(watch_files, &ev->monitor);

   if (!f) return EINA_TRUE;

   DBG("Monitor (%s) error´ed out of the door", ev->filename);

   eo_event_callback_call(f, EFM_FILE_EVENT_INVALID, "File Monitor got error");

   return EINA_TRUE;
}

void
efm_file_init(void)
{
    eina_lock_new(&readlock);
    watch_files = eina_hash_pointer_new(NULL);
    handler_mod = ecore_event_handler_add(EIO_MONITOR_FILE_MODIFIED, _mod_cb, NULL);
    handler_del = ecore_event_handler_add(EIO_MONITOR_SELF_DELETED, _file_del_cb, NULL);
    handler_err = ecore_event_handler_add(EIO_MONITOR_ERROR, _error_cb, NULL);
}

void
efm_file_shutdown(void)
{
    ecore_event_handler_del(handler_mod);
    ecore_event_handler_del(handler_err);
    ecore_event_handler_del(handler_del);
    handler_mod = NULL;
    handler_err = NULL;
    handler_del = NULL;
    eina_lock_free(&readlock);
    eina_hash_free(watch_files);
    watch_files = NULL;
}

EOLIAN static void *
_efm_fs_file_efm_file_monitor(Eo *obj, Efm_Fs_File_Data *pd EINA_UNUSED, void *filter)
{
   return eo_add(EFM_FS_MONITOR_CLASS, NULL, efm_fs_monitor_install(__eo_self, obj, filter));
}

EOLIAN static Efm_File*
_efm_fs_file_efm_file_child_get(Eo *obj EINA_UNUSED, Efm_Fs_File_Data *pd, const char *name)
{
   char buf[PATH_MAX];
   Efm_File *file;

   if (!S_ISDIR(pd->st.st_mode)) return NULL;

   snprintf(buf, sizeof(buf), "%s/%s", pd->path, name);
   file = efm_file_get(EFM_CLASS, buf);

   return file;
}




#include "efm_fs_file.eo.x"