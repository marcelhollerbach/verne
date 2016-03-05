#define EFL_BETA_API_SUPPORT
#include "efm_priv.h"

typedef struct {
    Eina_Bool invalid;
} Efm_File_Data;

EOLIAN static Eina_Bool
_efm_file_is_invalid_get(Eo *obj EINA_UNUSED, Efm_File_Data *pd)
{
    return pd->invalid;
}

static Eina_Bool
_invalid_cb(void *data, const Eo_Event *event EINA_UNUSED)
{
   Efm_File_Data *pd = data;

   pd->invalid = EINA_TRUE;

   return EO_CALLBACK_CONTINUE;
}

EOLIAN static Eo_Base *
_efm_file_eo_base_constructor(Eo *obj, Efm_File_Data *pd)
{
   eo_event_callback_add(obj, EFM_FILE_EVENT_INVALID, _invalid_cb, pd);

   return eo_constructor(eo_super(obj, EFM_FILE_CLASS));
}


#include "efm_file.eo.x"