#include "efm_priv.h"

typedef struct {
    Eina_Bool invalid;
} Efm_File_Data;

EOLIAN static Eina_Bool
_efm_file_is_invalid_get(Eo *obj EINA_UNUSED, Efm_File_Data *pd)
{
    return pd->invalid;
}

static void
_invalid_cb(void *data, const Eo_Event *event EINA_UNUSED)
{
   Efm_File_Data *pd = data;

   pd->invalid = EINA_TRUE;
}

EOLIAN static Efl_Object *
_efm_file_efl_object_constructor(Eo *obj, Efm_File_Data *pd)
{
   efl_event_callback_add(obj, EFM_FILE_EVENT_INVALID, _invalid_cb, pd);

   return efl_constructor(efl_super(obj, EFM_FILE_CLASS));
}


#include "efm_file.eo.x"