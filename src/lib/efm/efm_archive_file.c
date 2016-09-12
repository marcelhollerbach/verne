#include "efm_priv.h"

typedef struct {
    struct {
        const char *path; //the path where the archive is
        const char *find_path; //the path where you can find what is in the archive
    } archive;
    const char *file; //the file in the archive
    const char *real_path;//< the file which _really_ exists its archive.find_path+file
    const char *fake_path;
    const char *internal;
} Efm_Archive_File_Data;


EOLIAN static void
_efm_archive_file_generate(Eo *obj, Efm_Archive_File_Data *pd, const char *archive, const char *internal)
{
   char fakepath_buf[PATH_MAX];
   char realpath_buf[PATH_MAX];
   Eina_Bool root = EINA_FALSE;

   if (!internal ||
        (internal && !strcmp(internal, "/"))
      )
     root = EINA_TRUE;
   pd->archive.find_path = archive_access(archive, !internal);

   if (!pd->archive.find_path)
     {
        ERR("Failed to access %s\n", pd->archive.find_path);
        return;
     }

   snprintf(fakepath_buf, sizeof(fakepath_buf), "%s/%s", archive, internal);
   snprintf(realpath_buf, sizeof(realpath_buf), "%s/%s", pd->archive.find_path, internal);

   pd->fake_path = eina_stringshare_add(fakepath_buf);
   pd->real_path = eina_stringshare_add(realpath_buf);
   pd->internal = eina_stringshare_add(internal);


   efm_fs_file_generate(efl_super(obj, EFM_ARCHIVE_FILE_CLASS), eina_file_path_sanitize(pd->real_path));
}

EOLIAN static const char *
_efm_archive_file_efm_file_path_get(Eo *obj EINA_UNUSED, Efm_Archive_File_Data *pd)
{
   return pd->real_path;
}

EOLIAN static void
_efm_archive_file_efl_object_destructor(Eo *obj, Efm_Archive_File_Data *pd)
{
   if (!pd->archive.find_path) archive_unref(pd->archive.find_path);
   eina_stringshare_del(pd->fake_path);
   eina_stringshare_del(pd->real_path);
   eina_stringshare_del(pd->internal);
   efl_destructor(efl_super(obj, EFM_ARCHIVE_FILE_CLASS));
}

EOLIAN static Efl_Object *
_efm_archive_file_efl_object_finalize(Eo *obj, Efm_Archive_File_Data *pd)
{
    if (!pd->archive.find_path) return NULL;

    return efl_finalize(efl_super(obj, EFM_ARCHIVE_FILE_CLASS));
}

EOLIAN static const char *
_efm_archive_file_real_path_get(Eo *obj, Efm_Archive_File_Data *pd EINA_UNUSED)
{
    return efm_file_path_get(efl_super(obj, EFM_ARCHIVE_FILE_CLASS));
}

EOLIAN static void *
_efm_archive_file_efm_file_monitor(Eo *obj EINA_UNUSED, Efm_Archive_File_Data *pd EINA_UNUSED, void *filter)
{
   return efl_add(EFM_ARCHIVE_MONITOR_CLASS, NULL, efm_archive_monitor_generate(efl_added, obj, filter));
}

EOLIAN static Efm_File *
_efm_archive_file_efm_file_child_get(Eo *obj EINA_UNUSED, Efm_Archive_File_Data *pd, const char *name)
{
   char buf[PATH_MAX];
   Efm_File *file;

   //TODO check if I am a dir

   snprintf(buf, sizeof(buf), "%s/%s", pd->internal, name);
   file = efm_archive_get(EFM_CLASS, pd->archive.path, buf);
   return file;
}

#include "efm_archive_file.eo.x"