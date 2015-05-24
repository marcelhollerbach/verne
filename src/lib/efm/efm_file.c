#define EFL_BETA_API_SUPPORT
#include "efm_priv.h"
#include <Eo.h>
#include <Ecore.h>

static Ecore_Thread *fs_query;
static Eina_List *query_stuff;
static Eina_Lock readlock;

typedef struct {
    Eina_Lock lock;
    const char *path;
    const char *mimetype;

    Efm_File *file;
} Thread_Data;

typedef struct
{
    const char *filename;
    const char *path;
    const char *fileending;
    const char *mimetype;

    Thread_Data data;
    struct stat st;
    Efm_File_Stat stat;
} Efm_File_Data;

static void
_fs_cb(void *dat EINA_UNUSED, Ecore_Thread *thread)
{
    Eina_List *copy;
    Thread_Data *data;

    while(query_stuff)
      {
        //take a local copy of the list
        eina_lock_take(&readlock);
        copy = query_stuff;
        query_stuff = NULL;
        eina_lock_release(&readlock);

        EINA_LIST_FREE(copy, data)
          {
             const char *mime_type;

             mime_type = efreet_mime_special_type_get(data->path);
             if (!mime_type) mime_type = efreet_mime_globs_type_get(data->path);
             if (!mime_type) mime_type = efreet_mime_fallback_type_get(data->path);

             eina_lock_take(&data->lock);

             data->mimetype = mime_type;

             eina_lock_release(&data->lock);

             ecore_thread_feedback(thread, data);
          }
        if (!query_stuff)
          usleep(50);
      }
}

static void
_notify_cb(void *data EINA_UNUSED, Ecore_Thread *et EINA_UNUSED, void *pass)
{
    Efm_File *file;
    Efm_File_Data *pd;

    Thread_Data *thdata = pass;

    file = thdata->file;
    pd = eo_data_scope_get(file, EFM_FILE_CLASS);

    pd->mimetype = pd->data.mimetype;

    //we dont need it anymore
    if (eo_ref_get(file) > 1)
      {
        eo_unref(file);
        //notify that this efm_file is ready for the world
        eo_do(file, eo_event_callback_call(EFM_FILE_EVENT_FSQUERY_DONE, NULL));
      }
    else
      eo_unref(file);

}

static void
_end_cb(void *data EINA_UNUSED, Ecore_Thread *et EINA_UNUSED)
{
    fs_query = NULL;
}

static void
_cancel_cb(void *data EINA_UNUSED, Ecore_Thread *th  EINA_UNUSED)
{

}

static void
_scheudle(Thread_Data *data)
{
    eo_ref(data->file);

    eina_lock_take(&readlock);
    query_stuff = eina_list_append(query_stuff, data);
    eina_lock_release(&readlock);

    if (!fs_query)
      fs_query = ecore_thread_feedback_run(_fs_cb, _notify_cb,
                                           _end_cb, _cancel_cb,
                                           NULL, EINA_FALSE);

}

EOLIAN static const char *
_efm_file_filename_get(Eo *obj EINA_UNUSED, Efm_File_Data *pd)
{
    return pd->filename;
}

EOLIAN static const char *
_efm_file_path_get(Eo *obj EINA_UNUSED, Efm_File_Data *pd)
{
    return pd->path;
}

EOLIAN static const char *
_efm_file_fileending_get(Eo *obj EINA_UNUSED, Efm_File_Data *pd)
{
    return pd->fileending;
}

EOLIAN static const char *
_efm_file_mimetype_get(Eo *obj EINA_UNUSED, Efm_File_Data *pd)
{
    return pd->mimetype;
}

EOLIAN static Efm_File_Stat *
_efm_file_stat_get(Eo *obj EINA_UNUSED, Efm_File_Data *pd)
{
    return &pd->stat;
}
EOLIAN static Eina_Bool
_efm_file_is_type(Eo *obj, Efm_File_Data *pd, Efm_File_Type type)
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

EOLIAN static Eina_Bool
_efm_file_generate(Eo *obj, Efm_File_Data *pd, const char *filename)
{
    int end;

    //get the stat
    if (stat(filename, &pd->st) < 0)
      {
         return EINA_FALSE;
      }

    //parse stat to the eo struct
    pd->stat.uid = pd->st.st_uid;
    pd->stat.gid = pd->st.st_gid;
    pd->stat.size = pd->st.st_size;

    pd->stat.atime = pd->st.st_atim.tv_sec;
    pd->stat.ctime = pd->st.st_ctim.tv_sec;
    pd->stat.mtime = pd->st.st_mtim.tv_sec;

    //safe this name
    pd->path = eina_stringshare_add(filename);

    pd->data.path = pd->path;
    pd->data.file = obj;

    eina_lock_new(&pd->data.lock);
    //get the filename
    pd->filename = ecore_file_file_get(pd->path);

    //parse the fileending
    end = strlen(pd->path);
    do {
        if (pd->path[end] == '.')
          {
             pd->fileending = pd->path + end + 1;
             break;
          }
        end --;
    } while(end > 0);

    _scheudle(&pd->data);
    return EINA_TRUE;
}

void
efm_file_init(void)
{
    eina_lock_new(&readlock);
}

#include "efm_file.eo.x"