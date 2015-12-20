#define EFL_BETA_API_SUPPORT
#include <Eo.h>
#include <Ecore.h>

#include "efm_priv.h"

typedef struct {
    struct {
        const char *extract_path;
        const char *real_path;
    } archive;
    const char *inner_path;
    char *fake_path;
    char *real_path;
} Efm_Archive_Monitor_Data;

EOLIAN static void
_efm_archive_monitor_generate(Eo *obj, Efm_Archive_Monitor_Data *pd, const char *archive, const char *internal, Efm_Filter *filter)
{
    char buf[PATH_MAX];
    char buf2[PATH_MAX];

    pd->archive.real_path = archive;
    pd->archive.extract_path = archive_access(archive, 0);
    pd->inner_path = internal;

    if (!pd->archive.extract_path) return;

    snprintf(buf, sizeof(buf), "%s/%s", archive, internal);
    snprintf(buf2, sizeof(buf2), "%s/%s", pd->archive.extract_path, internal);

    pd->fake_path = strdup(eina_file_path_sanitize(buf));
    pd->real_path = strdup(eina_file_path_sanitize(buf2));

    printf("%s\n", pd->real_path);
    eo_do(obj, efm_fs_monitor_install(pd->real_path, filter));
}


EOLIAN static const char *
_efm_archive_monitor_efm_monitor_path_get(Eo *obj EINA_UNUSED, Efm_Archive_Monitor_Data *pd)
{
    return pd->fake_path;
}

EOLIAN static void
_efm_archive_monitor_eo_base_destructor(Eo *obj, Efm_Archive_Monitor_Data *pd)
{
    archive_unref(pd->archive.real_path);
    free(pd->fake_path);
    eo_do_super(obj, EFM_ARCHIVE_MONITOR_CLASS, eo_destructor());
}

EOLIAN static Eo_Base*
_efm_archive_monitor_eo_base_finalize(Eo *obj, Efm_Archive_Monitor_Data *pd)
{
   Eo_Base *ret;

   if (!pd->archive.extract_path) return NULL;

   eo_do_super(obj, EFM_ARCHIVE_MONITOR_CLASS, ret = eo_finalize());

   return ret;
}


#include "efm_archive_monitor.eo.x"