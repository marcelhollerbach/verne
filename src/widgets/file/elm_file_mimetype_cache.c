#define NO_ENTRY ">FAIL"

#include "../elementary_ext_priv.h"

typedef struct {
    int size;
    const char *no_entry;
    Eina_Hash *mime_type;
} Elm_File_MimeType_Cache_Data;


static void
_free(void *data)
{
   eina_stringshare_del(data);
}

EOLIAN static Elm_File_MimeType_Cache*
_elm_file_mimetype_cache_cache_generate(Eo *obj EINA_UNUSED, void *npd EINA_UNUSED, int size)
{
    Eo *result;
    Elm_File_MimeType_Cache_Data *pd;

    result = efl_add(ELM_FILE_MIMETYPE_CACHE_CLASS, NULL);
    pd = efl_data_scope_get(result, ELM_FILE_MIMETYPE_CACHE_CLASS);

    pd->size = size;
    pd->mime_type = eina_hash_stringshared_new(_free);
    pd->no_entry = eina_stringshare_add(NO_ENTRY);

    return result;
}

static void
_elm_file_mimetype_cache_mimetype_set(Eo *obj EINA_UNUSED, Elm_File_MimeType_Cache_Data *pd, Elm_Icon *icon, const char *name)
{
    const char *result;

    if (!name)
      {
         evas_object_hide(icon);
         return;
      }

    if (!strcmp(elm_config_icon_theme_get(), "_Elementary_Icon_Theme"))
      {
         //just use normal standard set
         elm_icon_standard_set(icon, name);
         return;
      }

   result = eina_hash_find(pd->mime_type, name);

   if (!result)
     {
        const char *theme, *ic;

        theme = elm_config_icon_theme_get();
        ic = efreet_mime_type_icon_get(name, theme, pd->size);
        result = eina_stringshare_add(ic);

        if (!result)
          eina_hash_direct_add(pd->mime_type, name, pd->no_entry);
        else
          eina_hash_direct_add(pd->mime_type, name, result);
      }
   if (result && result != pd->no_entry)
     elm_image_file_set(icon, result, NULL);
}

#include "elm_file_mimetype_cache.eo.x"