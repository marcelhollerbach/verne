#define EFL_BETA_API_SUPPORT
#define EFL_EO_API_SUPPORT

#define NO_ENTRY ">FAIL"

#include "../elementary_ext_priv.h"

typedef struct {
    int size;
    Eina_Hash *mime_type;
} Elm_File_MimeType_Cache_Data;


EOLIAN static Elm_File_MimeType_Cache*
_elm_file_mimetype_cache_cache_generate(Eo *obj EINA_UNUSED, void *npd EINA_UNUSED, int size)
{
    Eo *result;
    Elm_File_MimeType_Cache_Data *pd;

    result = eo_add(ELM_FILE_MIMETYPE_CACHE_CLASS, NULL);
    pd = eo_data_scope_get(result, ELM_FILE_MIMETYPE_CACHE_CLASS);

    pd->size = size;
    pd->mime_type = eina_hash_string_small_new(NULL);

    return result;
}

static const char*
_elm_file_mimetype_cache_mimetype_get(Eo *obj EINA_UNUSED, Elm_File_MimeType_Cache_Data *pd, const char *name)
{
    const char *result;

    result = eina_hash_find(pd->mime_type, name);

    if (!result)
      {
         const char *theme;

         theme = elm_obj_file_icon_util_icon_theme_get(ELM_FILE_ICON_CLASS);

         result = efreet_mime_type_icon_get(name, theme, pd->size);
         if (!result)
           eina_hash_direct_add(pd->mime_type, name, NO_ENTRY);
         else
           eina_hash_direct_add(pd->mime_type, name, result);
      }
    else if (!strcmp(result, NO_ENTRY))
      {
         return NULL;
      }

    return result;
}

#include "elm_file_mimetype_cache.eo.x"