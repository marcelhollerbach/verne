#ifndef EIO_FM_PRIV_H
#define EIO_FM_PRIV_H

#include "Efm.h"

#include <Eio.h>
#include <Ecore.h>
#include <Ecore_File.h>
#include <Efreet_Mime.h>

struct _EFM_Monitor
{
   const char *directory; //< The directory this monitor listens to
   Eio_Monitor *mon; //< The eio monitor which is started in the directory
   Eio_File *file; //< The eio file as long as the ls is running

   Eina_Hash *file_icons; // < Hash table of all listed FM_Monitor_Files
   File_Cb add_cb, del_cb, mime_ready_cb; // < The callbacks to call when something happen
   Err_Cb selfdel_cb, err_cb; //^
   void *data;
   Eina_Bool only_folder, hidden_files; //< the configuration which files to use
   Ecore_Thread *mime_thread; //< the thread which is querying the mime types
   Ecore_Idler *thread_idler; //< idler to start the thread
   struct {
      Eina_List *add; //< the list which icons to query
      Eina_Lock lock;
   } thread;

   Eina_Bool deletion_mark;
};

struct _EFM_File{
  Eina_Bool exists; //< true if the file exists at the add event
  Eina_Bool writeable; //< true if the file is writeabke
  Eina_Bool dir; //< true if the file is a directory
  Eina_Bool use; //< true if the filter returned true for the file

  struct stat stat;

  const char *file; //< file name of the path
  const char *path; //< the complete path

  const char *fileending; //< the file ending of the file

  const char *mime_type;
};

struct _EFM_Mount_Fs{
   const char *name;
   const char *script_path;
   const char *description;
};

extern int _log_domain;

#define CRIT(...)     EINA_LOG_DOM_CRIT(_log_domain, __VA_ARGS__)
#define ERR(...)      EINA_LOG_DOM_ERR(_log_domain, __VA_ARGS__)
#define WRN(...)      EINA_LOG_DOM_WARN(_log_domain, __VA_ARGS__)
#define INF(...)      EINA_LOG_DOM_INFO(_log_domain, __VA_ARGS__)
#define DBG(...)      EINA_LOG_DOM_DBG(_log_domain, __VA_ARGS__)

int fm_monitor_init();
void fm_monitor_shutdown();
#endif