#include <Efreet.h>
#include "efm_priv.h"

typedef struct {

} Efm_Data;

typedef struct
{
  int counter;
  Eina_Hash *factory;
} Efm_Static_Data;

static Efm_Static_Data *sd;

int _efm_domain;

EOLIAN static int
_efm_init(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED)
{
   eina_init();
   ecore_init();
   efreet_init();
   eio_init();
   if (sd && sd->counter > 0)
     goto inc;
   _efm_domain = eina_log_domain_register("efm", NULL);
   if (!_efm_domain)
     return 0;

   sd = calloc(1, sizeof(Efm_Static_Data));
   if (!sd)
     return 0;

   sd->factory = eina_hash_string_superfast_new(NULL);
   if (!fm_monitor_init())
     {
        ERR("Failed to init resolve");
        free(sd);
        sd = NULL;
        return 0;
     }
   efm_file_init();

   archive_init();
inc:
    sd->counter ++;
    return sd->counter;
}

EOLIAN static void
_efm_shutdown(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED)
{
   sd->counter --;

   if (sd->counter > 0)
     return;

   free(sd);
   sd = NULL;

   archive_shutdown();

   fm_monitor_shutdown();

   efm_file_shutdown();

   eina_log_domain_unregister(_efm_domain);
   eio_shutdown();
   efreet_shutdown();
   ecore_shutdown();
   eina_shutdown();
}
static Eina_Bool _file_del(void *data, Eo *obj, const Eo_Event_Description2 *desc EINA_UNUSED, void *event_info EINA_UNUSED);

EO_CALLBACKS_ARRAY_DEFINE(factory_events, {EFM_FILE_EVENT_INVALID, _file_del},
                                          {EO_BASE_EVENT_DEL, _file_del});

static Eina_Bool
_file_del(void *data, Eo *obj, const Eo_Event_Description2 *desc EINA_UNUSED, void *event_info EINA_UNUSED)
{
   eina_hash_del_by_data(sd->factory, obj);
   eo_do(obj, eo_event_callback_array_del(factory_events(), sd->factory));
   return EO_CALLBACK_CONTINUE;
}

#define SEARCH_IF_FOUND_RETURN_INCED(PATH,FILE) \
   FILE = eina_hash_find(sd->factory, PATH); \
   if (FILE) { \
     eo_ref(FILE); \
     return FILE; \
   }\

EOLIAN static Efm_File*
_efm_file_get(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED, const char *path)
{
   Efm_File *file;

   SEARCH_IF_FOUND_RETURN_INCED(path, file)

   file = eo_add(EFM_FS_FILE_CLASS, NULL, efm_fs_file_generate(path));
   if (file)
     {
        eo_do(file, eo_event_callback_array_add(factory_events(), sd->factory));
        eina_hash_add(sd->factory, &path, file);
     }

   return file;
}

EOLIAN static Efm_File*
_efm_archive_get(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED, const char *archive_path, const char *path)
{
   char compose_path[PATH_MAX];
   Efm_File *file;

   snprintf(compose_path, sizeof(compose_path), "%s/%s", archive_path, path);

   SEARCH_IF_FOUND_RETURN_INCED(compose_path, file);

   file = eo_add(EFM_ARCHIVE_FILE_CLASS, NULL, efm_archive_file_generate(archive_path, path));

   if (file)
     {
        eo_do(file, eo_event_callback_array_add(factory_events(), sd->factory));
        eina_hash_add(sd->factory, &compose_path, file);
     }

   return file;
}

EOLIAN static Eina_Bool
_efm_archive_supported(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED, const char *fileending)
{
   return (fileending) ? archive_support(fileending) : EINA_FALSE;
}

#include "efm.eo.x"