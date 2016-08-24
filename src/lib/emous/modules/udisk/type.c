#include "udisk.h"

typedef struct{

} Emous_Type_Udisks_Data;

Emous_Type *type = NULL;

static Eina_List *
_emous_type_udisks_emous_type_devices_get(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED)
{
    return devices_list;
}

static Eina_Bool
_emous_type_udisks_emous_type_create(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED, char *params EINA_UNUSED, char *mountpoint EINA_UNUSED)
{
    CRIT("Not allowed!!");
    return EINA_FALSE;
}

static Eina_Bool
_emous_type_udisks_emous_type_creatable(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED)
{
    return EINA_FALSE;
}


static const char *
_emous_type_udisks_emous_type_name_get(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED)
{
    return "Udisks2";
}

static Emous_Type *
_emous_type_udisks_emous_type_object_get(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED)
{
    if (type) return type;

    CRIT("Illigal call...");
    return NULL;
}

#include "emous_type_udisks.eo.x"