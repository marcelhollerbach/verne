#ifndef EIO_FM_H
#define EIO_FM_H

#include <Eina.h>

typedef struct _FM_Monitor FM_Monitor;
typedef struct _FM_Monitor_File FM_Monitor_File;

typedef void(*File_Cb)(void *data, FM_Monitor *mon, FM_Monitor_File *file);
typedef void(*Err_Cb)(void *data, FM_Monitor *mon);

/*
 * Inits the fm_monitor
 */
int fm_monitor_init();
/*
  shutdown the fm_monitor
 */
void fm_monitor_shutdown();

/*
  Start the service on the passed directory.
  The passed callbacks will be called
  */
FM_Monitor* fm_monitor_start(const char *directory, File_Cb add_cb,
                             File_Cb del_cb, File_Cb mime_ready,
                             Err_Cb selfdel_cb, Err_Cb err_cb, void *data,
                             Eina_Bool hidden_files, Eina_Bool only_folder);

/*
  Stop the passed FM_Monitor
 */
void        fm_monitor_stop(FM_Monitor *mon);

const char* efm_file_filename_get(FM_Monitor_File *f);

const char* efm_file_path_get(FM_Monitor_File *f);

const char* efm_file_fileending_get(FM_Monitor_File *f);

const char* efm_file_mimetype_get(FM_Monitor_File *f);

Eina_Bool efm_file_is_dir(FM_Monitor_File *f);

struct stat* efm_file_stat_get(FM_Monitor_File *f);

const char* fm_monitor_path_get(FM_Monitor *mon);

Eina_Bool eio_fm_archive_file_supported(const char *fileending);

Eina_Bool eio_fm_archive_file_extract(const char *file, const char *goal);

const char *eio_fm_archive_lasterror_get(void);

#endif