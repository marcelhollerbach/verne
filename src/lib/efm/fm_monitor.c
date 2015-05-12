#include "efm_priv.h"

typedef struct
{
   Ecore_Event_Handler *fadd, *fdel,
                       *dadd, *ddel,
                       *err, *selfdel;
   Eina_Hash *open_monitors;
} Context;

static Context *ctx;

#define EVENT_ENTRY_GETTER \
   fmm = eina_hash_find(ctx->open_monitors, &ev->monitor); \
   if (!fmm) return EINA_TRUE;

static Eina_Bool
_filter_cb(const char *file, EFM_Monitor *fm)
{
   const char *filename = ecore_file_file_get(file);

   if ((!fm->hidden_files) && (filename[0] == '.'))
      return EINA_FALSE;

   if ((fm->only_folder) && !ecore_file_is_dir(file))
     return EINA_FALSE;

  return EINA_TRUE;
}

//=======================
//RUNS IN A THREAD

static void
_thread_cb(void *data, Ecore_Thread *et)
{
   EFM_Monitor *fm;
   EFM_File *fmf;
   Eina_List *it;
   const char *mime_type;

   fm = data;

   eina_lock_take(&fm->thread.lock);
   eina_lock_release(&fm->thread.lock);
   while (fm->thread.add)
     {
        eina_lock_take(&fm->thread.lock);
        it = fm->thread.add;
        fm->thread.add = NULL;
        eina_lock_release(&fm->thread.lock);
        EINA_LIST_FREE(it, fmf)
          {
             if (ecore_thread_check(et))
               break;
             mime_type = efreet_mime_special_type_get(fmf->path);
             if (!mime_type) mime_type = efreet_mime_globs_type_get(fmf->path);
             if (!mime_type) mime_type = efreet_mime_fallback_type_get(fmf->path);
             //mime type checking can take very long, check again before we are accessing memory
             if (ecore_thread_check(et))
               break;
             fmf->mime_type = mime_type;
             ecore_thread_feedback(et, fmf);
          }
        if (ecore_thread_check(et))
          break;
        //its possible that the thread is faster than the mainloop,
        //in this case the mime fetcher would go on/off/on/off all the time
        //we try to avoid that by sleeping this part of time
        if (!fm->thread.add)
          usleep(50);
     }
   fm->mime_thread = NULL;
}

//
//==========================

static void
_notify_cb(void *data, Ecore_Thread *et, void *pass)
{
   EFM_File *fmf = pass;
   EFM_Monitor *fmm = data;

   if (ecore_thread_check(et))
     return;

   if (fmm->deletion_mark)
     return;

   if (fmf->exists && fmf->use && fmm->mime_ready_cb)
     fmm->mime_ready_cb(fmm->data, fmm, fmf);
}

static void
_thread_end(void *data, Ecore_Thread *et)
{
   EFM_Monitor *fm;

   if (ecore_thread_check(et))
     return;

   fm = data;
   fm->mime_thread = NULL;
}

static void
_mime_type_scheudle(EFM_Monitor *fm, EFM_File *fmf)
{
   //do not scheudle a mime type fetch if we are in deletion
   if (fm->deletion_mark) return;
   //we are taking the lock if we are creating the thread etc.
   //we make sure with that the thread does not stop because add is not filled yet
   if (!fm->mime_thread)
     {
        eina_lock_new(&fm->thread.lock);
        eina_lock_take(&fm->thread.lock);
        //create the thread
        fm->mime_thread = ecore_thread_feedback_run(_thread_cb, _notify_cb, _thread_end, NULL, fm, EINA_FALSE);
     }
   else
     eina_lock_take(&fm->thread.lock);

   fm->thread.add = eina_list_append(fm->thread.add, fmf);

   eina_lock_release(&fm->thread.lock);
}

static EFM_File*
_ref_create(EFM_Monitor *fmm, const char *filename, Eina_Bool dir)
{
   const char *ref;
   EFM_File *fmm_file;
   int end;

   ref = eina_stringshare_add((filename));

   fmm_file = calloc(1, sizeof(EFM_File));

   //simple path
   fmm_file->path = ref;

   //filetype
   fmm_file->dir = dir;

   //should we publish it ?
   fmm_file->use = _filter_cb(ref, fmm);

   //does it really exists ?
   fmm_file->exists = ecore_file_exists(fmm_file->path);

   //if it exists save size and timestamp
   if (stat(fmm_file->path, &fmm_file->stat) < 0)
     {
        ERR("Failed to fetch informations \n");
     }

   //check how accessable it is
   if (access(fmm_file->path, W_OK) == 0)
    fmm_file->writeable = EINA_TRUE;
   else
    fmm_file->writeable = EINA_FALSE;

   //get the filename
   fmm_file->file = ecore_file_file_get(fmm_file->path);

   //parse the fileending
   end = strlen(fmm_file->path);
   do {
       if (fmm_file->path[end] == '.')
         {
            fmm_file->fileending = fmm_file->path + end + 1;
            break;
         }
       end --;
   } while(end > 0);

   eina_hash_add(fmm->file_icons, ref, fmm_file);

   return fmm_file;
}

static void
_ref_unref(EFM_Monitor *fmm, const char *filename)
{
  eina_hash_del(fmm->file_icons, filename, NULL);
}

static EFM_File*
_ref_find(EFM_Monitor *fmm, const char *filename)
{
   const char *path;
   EFM_File *fmm_file;

   path = eina_stringshare_add(filename);
   fmm_file = eina_hash_find(fmm->file_icons, path);
   eina_stringshare_del(path);

   return fmm_file;
}

static Eina_Bool
_file_add(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Eio_Monitor_Event *ev = event;
   EFM_Monitor *fmm;
   EFM_File *file;

   EVENT_ENTRY_GETTER

   file = _ref_create(fmm, ev->filename, EINA_FALSE);
   _mime_type_scheudle(fmm, file);
   if (file->exists && file->use)
     fmm->add_cb(fmm->data, fmm, file);

   return EINA_FALSE;
}

static Eina_Bool
_file_del(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Eio_Monitor_Event *ev = event;
   EFM_Monitor *fmm;
   EFM_File *fmm_file;

   EVENT_ENTRY_GETTER

   fmm_file = _ref_find(fmm, ev->filename);

   if (!fmm_file)
     {
        ERR("File %s not found", ev->filename);
        return EINA_FALSE;
     }

   if (fmm_file->exists && fmm_file->use)
     fmm->del_cb(fmm->data, fmm, fmm_file);

   _ref_unref(fmm, fmm_file->path);
   return EINA_FALSE;
}

static Eina_Bool
_dir_add(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Eio_Monitor_Event *ev = event;
   EFM_Monitor *fmm;
   EFM_File *path;

   EVENT_ENTRY_GETTER

   path = _ref_create(fmm, ev->filename, EINA_TRUE);
   if (path->exists && path->use)
     fmm->add_cb(fmm->data, fmm, path);
   return EINA_FALSE;
}

static Eina_Bool
_dir_del(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Eio_Monitor_Event *ev = event;
   EFM_Monitor *fmm;
   EFM_File *fmm_file;

   EVENT_ENTRY_GETTER

   fmm_file = _ref_find(fmm, ev->filename);

   if (!fmm_file)
     {
        ERR("File %s not found", ev->filename);
        return EINA_FALSE;
     }

   if (fmm_file->exists && fmm_file->use)
     fmm->del_cb(fmm->data, fmm, fmm_file);

   _ref_unref(fmm, fmm_file->path);
   return EINA_FALSE;
}

static Eina_Bool
_mon_err(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Eio_Monitor_Event *ev = event;
   EFM_Monitor *fmm;

   EVENT_ENTRY_GETTER

   fmm->err_cb(fmm->data, fmm);
   
   fm_monitor_stop(fmm);

   return EINA_FALSE;
}

static Eina_Bool
_mon_del(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Eio_Monitor_Event *ev = event;
   EFM_Monitor *fmm;

   EVENT_ENTRY_GETTER

   fmm->selfdel_cb(fmm->data, fmm);

   //a self delete will end in a error, so stop is called there

   return EINA_FALSE;
}

int
fm_monitor_init()
{
   ecore_init();
   efreet_mime_init();
   eio_init();

   ctx = calloc(1, sizeof(Context));

   ctx->open_monitors = eina_hash_pointer_new(NULL);

   ctx->fadd = ecore_event_handler_add(EIO_MONITOR_FILE_CREATED, _file_add, ctx);
   EINA_SAFETY_ON_NULL_GOTO(ctx->fadd, err);
   ctx->fdel = ecore_event_handler_add(EIO_MONITOR_FILE_DELETED, _file_del, ctx);
   EINA_SAFETY_ON_NULL_GOTO(ctx->fdel, err);


   ctx->dadd = ecore_event_handler_add(EIO_MONITOR_DIRECTORY_CREATED, _dir_add, ctx);
   EINA_SAFETY_ON_NULL_GOTO(ctx->dadd,err);
   ctx->ddel = ecore_event_handler_add(EIO_MONITOR_DIRECTORY_DELETED, _dir_del, ctx);
   EINA_SAFETY_ON_NULL_GOTO(ctx->ddel,err);


   ctx->selfdel = ecore_event_handler_add(EIO_MONITOR_SELF_DELETED, _mon_del, ctx);
   EINA_SAFETY_ON_NULL_GOTO(ctx->selfdel,err);
   ctx->err = ecore_event_handler_add(EIO_MONITOR_ERROR, _mon_err, ctx);
   EINA_SAFETY_ON_NULL_GOTO(ctx->err,err);

   return 1;
err:
   fm_monitor_shutdown();

   return 0;
}

void
fm_monitor_shutdown()
{

   ecore_event_handler_del(ctx->fadd);
   ecore_event_handler_del(ctx->fdel);

   ecore_event_handler_del(ctx->dadd);
   ecore_event_handler_del(ctx->ddel);

   ecore_event_handler_del(ctx->err);
   ecore_event_handler_del(ctx->selfdel);

   free(ctx);

   ctx = NULL;
   ecore_shutdown();
   eio_shutdown();
   efreet_mime_shutdown();
}


static Eina_Bool
_eio_filter_cb(void *data EINA_UNUSED, Eio_File *handler EINA_UNUSED, const char *file EINA_UNUSED)
{
   return EINA_TRUE;
}

static void
_eio_main_cb(void *data, Eio_File *handler EINA_UNUSED, const char *file)
{
   EFM_Monitor *fmm = data;
   EFM_File *path = NULL;

   if (eio_file_check(handler)) return;

   if (fmm->deletion_mark) return;

   path = _ref_create(fmm, file, ecore_file_is_dir(file));
   _mime_type_scheudle(fmm, path);
   if (path->exists && path->use)
     fmm->add_cb(fmm->data, fmm, path);
}

static void
_eio_done_cb(void *data, Eio_File *handler EINA_UNUSED)
{
   EFM_Monitor *fmm = data;
   /* now we are ready for new files etc. */
   eina_hash_add(ctx->open_monitors, &(fmm->mon), fmm);
   fmm->file = NULL;
 }

static void
_eio_error_cb(void *data EINA_UNUSED, Eio_File *handler, int error EINA_UNUSED)
{
   EFM_Monitor *fmm = data;

   if (eio_file_check(handler)) return;

   if (fmm->deletion_mark) return;

   fmm->err_cb(fmm->data, fmm);
   fm_monitor_stop(fmm);
}

void
_mon_hash_free(void *data)
{
   EFM_File *file = data;

   eina_stringshare_del(file->path);
   free(file);
}

EFM_Monitor*
fm_monitor_start(const char *directory, File_Cb add_cb,
                 File_Cb del_cb, File_Cb mime_ready_cb, Err_Cb selfdel_cb, Err_Cb err_cb,
                 void *data, Eina_Bool hidden_files, Eina_Bool only_folder)
{
   if (!ecore_file_exists(directory))
     {
        ERR("The path does not exist!");
        return NULL;
     }
   if (!ecore_file_is_dir(directory))
     {
        ERR("The path has to be a directory!");
        return NULL;
     }
   EFM_Monitor *fmm = calloc(1, sizeof(EFM_Monitor));

   fmm->only_folder = only_folder;
   fmm->hidden_files = hidden_files;

   fmm->directory = eina_stringshare_add(directory);

   fmm->mon = eio_monitor_stringshared_add(fmm->directory);
   fmm->file_icons = eina_hash_stringshared_new(_mon_hash_free);

   fmm->add_cb = add_cb;
   fmm->mime_ready_cb = mime_ready_cb;
   fmm->del_cb = del_cb;
   fmm->selfdel_cb = selfdel_cb;
   fmm->err_cb = err_cb;

   fmm->file = eio_file_ls(fmm->directory, _eio_filter_cb, _eio_main_cb,
                           _eio_done_cb, _eio_error_cb, fmm);

   fmm->data = data;
   return fmm;
}

static Eina_Bool
_deletion_idle(void *data)
{
  fm_monitor_stop(data);
  return EINA_FALSE;
}

void
fm_monitor_stop(EFM_Monitor *mon)
{
   Eina_Bool realdel = EINA_TRUE;
   //idle the delete
   if (mon->file)
     {
        //main_cb is called latelly we are deleting on dile
        eio_file_cancel(mon->file);
        mon->file = NULL;

        mon->deletion_mark = EINA_TRUE;
        ecore_idler_add(_deletion_idle, mon);
        realdel = EINA_FALSE;
     }
   if (mon->mime_thread)
     {
        //stop the thread
        ecore_thread_cancel(mon->mime_thread);
        mon->mime_thread = NULL;

        mon->deletion_mark = EINA_TRUE;
        ecore_idler_add(_deletion_idle, mon);
        realdel = EINA_FALSE;
     }
   if (!realdel)
     return;
   //delete if from the open monitors
   eina_hash_del(ctx->open_monitors, &(mon->mon), NULL);
   //close eio_monitor
   eio_monitor_del(mon->mon);
   //free all icons
   eina_hash_free(mon->file_icons);
   //mon->path is deleted by the free method of the hash
   free(mon);
}

const char*
fm_monitor_path_get(EFM_Monitor *mon)
{
   return mon->directory;
}

const char*
efm_file_filename_get(EFM_File *f)
{
   if (!f)
     return NULL;
   return f->file;
}

const char*
efm_file_path_get(EFM_File *f)
{
   if (!f)
     return NULL;
   return f->path;
}

const char*
efm_file_fileending_get(EFM_File *f)
{
   if (!f)
     return NULL;
   return f->fileending;
}

const char*
efm_file_mimetype_get(EFM_File *f)
{
   if (!f)
     return NULL;
   return f->mime_type;
}

Eina_Bool
efm_file_is_dir(EFM_File *f)
{
   if (!f)
     return EINA_FALSE;
   return f->dir;
}

struct stat*
efm_file_stat_get(EFM_File *f)
{
   if (!f)
     return NULL;
   return &f->stat;
}