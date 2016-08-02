#define EFL_BETA_API_SUPPORT
#define EFL_EO_API_SUPPORT

#include <e.h>
#include "e_bg_widget.eo.h"
#include <Elementary_Ext.h>

typedef struct {
    struct {
        Ecore_Event_Handler *zone_resize, *zone_del, *zone_add;
    } handler;
    Eina_Hash *fms;
} Jesus_Data;

static Jesus_Data *sd;

E_API E_Module_Api e_modapi =
{
   21,
   "Jesus"
};

static Evas_Object*
_fm_find(E_Zone *zone)
{
   return eina_hash_find(sd->fms, &zone);
}

static void
_fm_resize(E_Zone *zone)
{
   Evas_Object *fm = _fm_find(zone);
   Eina_Rectangle size;

   if (!fm) return;

   e_zone_useful_geometry_get(zone, &size.x, &size.y, &size.w, &size.h);
   evas_object_geometry_set(fm, size.x, size.y, size.w, size.h);
}

static void
_fm_add(E_Zone *zone)
{
   Evas_Object *fm;
   Efm_File *file;
   const char *desk;

   if (_fm_find(zone)) return;

   fm = eo_add(E_BG_WIDGET_CLASS, e_comp->elm);

   desk = efreet_desktop_dir_get();
   file = efm_file_get(EFM_CLASS, desk);

   elm_file_selector_file_set(fm, file);

   eina_hash_add(sd->fms, &zone, fm);

   _fm_resize(zone);
   evas_object_show(fm);
}

static void
_fm_del(E_Zone *zone)
{
   eina_hash_del(sd->fms, &zone, NULL);
}

static Eina_Bool
_zone_resize(void *data EINA_UNUSED, int type EINA_UNUSED, void *event EINA_UNUSED)
{
   E_Event_Zone_Move_Resize *ev = event;

   _fm_resize(ev->zone);

   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_zone_del(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   E_Event_Zone_Del *ev = event;

   _fm_del(ev->zone);

   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_zone_add(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   E_Event_Zone_Add *ev = event;

   _fm_add(ev->zone);

   return ECORE_CALLBACK_PASS_ON;
}


static void
_fm_free(void *data)
{
   eo_del(data);
}

E_API void *
e_modapi_init(E_Module *m)
{
   Eina_List *l;
   E_Zone *zone;

   elm_need_ethumb();
   elm_need_efreet();

   sd = calloc(1, sizeof(Jesus_Data));

   elm_ext_init();

   sd->fms = eina_hash_pointer_new(_fm_free);

   sd->handler.zone_add =
     ecore_event_handler_add(E_EVENT_ZONE_ADD,
                             _zone_add, NULL);
   sd->handler.zone_resize =
     ecore_event_handler_add(E_EVENT_ZONE_MOVE_RESIZE,
                             _zone_resize, NULL);
   sd->handler.zone_del =
     ecore_event_handler_add(E_EVENT_ZONE_DEL,
                             _zone_del, NULL);
   /* Hook into zones */
   EINA_LIST_FOREACH(e_comp->zones, l, zone)
     {
        if (_fm_find(zone)) continue;//WTF
        _fm_add(zone);
     }
   return m;
}


E_API void *
e_modapi_shutdown(E_Module *m)
{
   ecore_event_handler_del(sd->handler.zone_add);
   ecore_event_handler_del(sd->handler.zone_resize);
   ecore_event_handler_del(sd->handler.zone_del);
   eina_hash_free(sd->fms);
   free(sd);
   return m;
}