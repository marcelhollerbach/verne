#ifndef EIO_FM_H
#define EIO_FM_H

#include <Eina.h>

typedef struct _EFM_Monitor EFM_Monitor;
typedef struct _EFM_File EFM_File;

typedef void(*File_Cb)(void *data, EFM_Monitor *mon, EFM_File *file);
typedef void(*Err_Cb)(void *data, EFM_Monitor *mon);

/*
 * Inits the fm_monitor
 */
int efm_init();
/*
  shutdown the fm_monitor
 */
void efm_shutdown();

/*
  Start the service on the passed directory.
  The passed callbacks will be called
  */
EFM_Monitor* fm_monitor_start(const char *directory, File_Cb add_cb,
                             File_Cb del_cb, File_Cb mime_ready,
                             Err_Cb selfdel_cb, Err_Cb err_cb, void *data,
                             Eina_Bool hidden_files, Eina_Bool only_folder);

/*
  Stop the passed FM_Monitor
 */
void        fm_monitor_stop(EFM_Monitor *mon);

const char* fm_monitor_path_get(EFM_Monitor *mon);

/*
 * Efm files
 */

const char* efm_file_filename_get(EFM_File *f);

const char* efm_file_path_get(EFM_File *f);

const char* efm_file_fileending_get(EFM_File *f);

const char* efm_file_mimetype_get(EFM_File *f);

Eina_Bool efm_file_is_dir(EFM_File *f);

struct stat* efm_file_stat_get(EFM_File *f);

/*
 * Efm archive stuff
 */

Eina_Bool efm_archive_file_supported(const char *fileending);

Eina_Bool efm_archive_file_extract(const char *file, const char *goal);

const char *efm_archive_lasterror_get(void);

#endif