#include <regex.h>

#include "efm_priv.h"

#define POPULATE_CHANGE(obj) eo_event_callback_call(obj, EFM_FILTER_EVENT_FILTER_CHANGED, NULL)

typedef struct {
    Eina_Bool init;
    regex_t reg;
    const char *regexp;
} Filter;

typedef struct
{
   Eina_List *attribute[3];
   Eina_List *types;
   Eina_Bool whitelist;
} Efm_Filter_Data;

EOLIAN static void
_efm_filter_whitelist_set(Eo *obj, Efm_Filter_Data *pd, Eina_Bool whitelist)
{
   pd->whitelist = whitelist;
   POPULATE_CHANGE(obj);
}

EOLIAN static Eina_Bool
_efm_filter_whitelist_get(Eo *obj EINA_UNUSED, Efm_Filter_Data *pd)
{
   return pd->whitelist;
}

EOLIAN static void
_efm_filter_attribute_add(Eo *obj, Efm_Filter_Data *pd, Efm_Attribute attribute, char *reg)
{
   Filter *f;

   f = calloc(1, sizeof(Filter));

   f->regexp = eina_stringshare_add(reg);

   pd->attribute[attribute] = eina_list_append(pd->attribute[attribute], f);
   POPULATE_CHANGE(obj);
}

EOLIAN static void
_efm_filter_attribute_del(Eo *obj, Efm_Filter_Data *pd, Efm_Attribute attribute, char *req)
{
   Filter *f;
   Eina_List *node;

   EINA_LIST_FOREACH(pd->attribute[attribute], node, f)
     {
        if (!strcmp(f->regexp, req))
          {
             pd->attribute[attribute] = eina_list_remove(pd->attribute[attribute], f);

             if (f->init)
               regfree(&f->reg);
             free(f);
          }
     }
   POPULATE_CHANGE(obj);
}

EOLIAN static void
_efm_filter_type_add(Eo *obj, Efm_Filter_Data *pd, Efm_File_Type type)
{
   pd->types = eina_list_append(pd->types, (void*)(uintptr_t) type);
   POPULATE_CHANGE(obj);
}

EOLIAN static void
_efm_filter_type_del(Eo *obj, Efm_Filter_Data *pd, Efm_File_Type type)
{
   pd->types = eina_list_remove(pd->types, (void*)(uintptr_t) type);
   POPULATE_CHANGE(obj);
}

static Eina_Bool
_type_match(Efm_Filter_Data *pd, Efm_File *file)
{
   Efm_File_Type type;
   void *type_raw;
   Eina_List *node;
   EINA_LIST_FOREACH(pd->types, node, type_raw)
     {

         type = (uintptr_t)(void*) type_raw;

         if (!efm_file_is_type(file, type))
           return EINA_FALSE;
     }
   return EINA_TRUE;
}

static Eina_Bool
_attr_match(Efm_Filter_Data *pd, Efm_File *file)
{
   Eina_List *node;
   Filter *f;

   for (int i = 0; i < EFM_ATTRIBUTE_END; i++)
     {
        const char *checker;

        switch(i){
          case EFM_ATTRIBUTE_FILEENDING:
            checker = efm_file_fileending_get(file);
          break;
          case EFM_ATTRIBUTE_FILENAME:
            checker = efm_file_filename_get(file);
          break;
          case EFM_ATTRIBUTE_MIMETYPE:
            checker = efm_file_mimetype_get(file);
          break;
        }
        EINA_LIST_FOREACH(pd->attribute[i], node, f)
          {
             if (!f->init)
               {
                  if (regcomp(&f->reg, f->regexp, 0) != 0)
                    ERR("Failed to compile reg %s", f->regexp);
                  f->init = EINA_TRUE;
               }

             if (regexec(&f->reg, checker, 0, NULL, 0) == REG_NOMATCH)
               {
                  return EINA_FALSE;
               }
          }
     }
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efm_filter_matches(Eo *obj EINA_UNUSED, Efm_Filter_Data *pd, Efm_File *file)
{
   //check if the types matches
   if (_type_match(pd, file) == !pd->whitelist) {
     return EINA_FALSE;
   }
   //check if the regex matches one of the attributes
   if (_attr_match(pd, file) == !pd->whitelist) {
     return EINA_FALSE;
   }
   return EINA_TRUE;
}

EOLIAN static Eo_Base *
_efm_filter_eo_base_constructor(Eo *obj, Efm_Filter_Data *pd)
{
   pd->whitelist = EINA_TRUE;
   return eo_constructor(eo_super(obj, EFM_FILTER_CLASS));
}

EOLIAN static void
_efm_filter_eo_base_destructor(Eo *obj, Efm_Filter_Data *pd)
{
   for (int i = 0; i < EFM_ATTRIBUTE_END; i++)
     {
        Filter *f;

        EINA_LIST_FREE(pd->attribute[i], f)
          {
             if (f->init)
               regfree(&f->reg);
             free(f);
          }
     }
   eina_list_free(pd->types);
   eo_destructor(eo_super(obj, EFM_FILTER_CLASS));
}


#include "efm_filter.eo.x"
