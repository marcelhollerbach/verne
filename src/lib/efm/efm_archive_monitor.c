#define EFL_BETA_API_SUPPORT
#include <Eo.h>
#include <Ecore.h>

#include "efm_priv.h"

typedef struct {
    Efm_Archive_File *file;
} Efm_Archive_Monitor_Data;

EOLIAN static void
_efm_archive_monitor_generate(Eo *obj, Efm_Archive_Monitor_Data *pd, Efm_Archive_File *file, Efm_Filter *filter)
{
    const char *path;
    Efm_Fs_File *fs_file;

    pd->file = file;

    eo_do_ret(file, path, efm_archive_file_real_path_get());
    eo_do(EFM_CLASS, fs_file = efm_file_get(path));

    eo_do(obj, efm_fs_monitor_install(fs_file, filter));
}


EOLIAN static Efm_File*
_efm_archive_monitor_efm_monitor_file_get(Eo *obj EINA_UNUSED, Efm_Archive_Monitor_Data *pd)
{
    return pd->file;
}

#include "efm_archive_monitor.eo.x"