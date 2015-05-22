#ifndef EIO_FM_H
#define EIO_FM_H

#include <Eina.h>
#include <Eo.h>

typedef struct stat Efm_Stat;

#ifdef EFL_BETA_API_SUPPORT
# include "efm_file.eo.h"
#else
# include "efm_file.eo.legacy.h"
#endif

extern int MOUNT_REQUEST_ENDED;
extern int MOUNT_REQUEST_STARTED;

typedef struct _EFM_Monitor EFM_Monitor;

typedef void(*File_Cb)(void *data, EFM_Monitor *mon, Efm_File *file);
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
 * Efm archive stuff
 */

Eina_Bool efm_archive_file_supported(const char *fileending);

Eina_Bool efm_archive_file_extract(const char *file, const char *goal);

const char *efm_archive_lasterror_get(void);

#endif