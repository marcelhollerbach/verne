#include "efm_priv.h"

typedef struct
{
   Ecore_Event_Handler *fadd, *fdel,
                       *dadd, *ddel,
                       *err, *selfdel;
   Eina_Hash *open_monitors;
} Context;

typedef struct
{
  Efm_Monitor *mon;
  Fm_File_Action action;
} Monitor_Entry;

static Context *ctx;

#define EVENT_ENTRY_GETTER \
   me = eina_hash_find(ctx->open_monitors, &ev->monitor); \
   if (!me) return EINA_TRUE;

void
fm_monitor_add(Efm_Monitor *mon, Eio_Monitor *monitor, Fm_File_Action action)
{
   Monitor_Entry *entry;

   entry = calloc(1, sizeof(Monitor_Entry));

   entry->mon = mon;
   entry->action = action;

   eina_hash_add(ctx->open_monitors, &monitor, entry);
}

void
fm_monitor_del(Efm_Monitor *mon EINA_UNUSED, Eio_Monitor *monitor)
{
   eina_hash_del(ctx->open_monitors, &monitor, NULL);
}


static Eina_Bool
_file_add(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Eio_Monitor_Event *ev = event;
   Monitor_Entry *me;

   EVENT_ENTRY_GETTER

   me->action(NULL, me->mon, ev->filename, ADD);

   return EINA_FALSE;
}

static Eina_Bool
_file_del(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Eio_Monitor_Event *ev = event;
   Monitor_Entry *me;

   EVENT_ENTRY_GETTER

   me->action(NULL, me->mon, ev->filename, DEL);

   return EINA_FALSE;
}

static Eina_Bool
_dir_add(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Eio_Monitor_Event *ev = event;
   Monitor_Entry *me;

   EVENT_ENTRY_GETTER

   me->action(NULL, me->mon, ev->filename, ADD);

   return EINA_FALSE;
}

static Eina_Bool
_dir_del(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Eio_Monitor_Event *ev = event;
   Monitor_Entry *me;

   EVENT_ENTRY_GETTER

   me->action(NULL, me->mon, ev->filename, DEL);

   return EINA_FALSE;
}

static Eina_Bool
_mon_err(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Eio_Monitor_Event *ev = event;
   Monitor_Entry *me;

   EVENT_ENTRY_GETTER

   me->action(NULL, me->mon, ev->filename, ERROR);

   return EINA_FALSE;
}

static Eina_Bool
_mon_del(void *data EINA_UNUSED, int type EINA_UNUSED, void *event EINA_UNUSED)
{
   //we are doing nothing here ... a selfdel will result in a error call this are handeld there
   return EINA_FALSE;
}

static void
_free_monitor(void *data)
{
   free(data);
}

int
fm_monitor_init()
{
   ecore_init();
   efreet_mime_init();
   eio_init();

   ctx = calloc(1, sizeof(Context));

   ctx->open_monitors = eina_hash_pointer_new(_free_monitor);

   ctx->fadd = ecore_event_handler_add(EIO_MONITOR_FILE_CREATED, _file_add, ctx);
   EINA_SAFETY_ON_NULL_GOTO(ctx->fadd, err);
   ctx->fdel = ecore_event_handler_add(EIO_MONITOR_FILE_DELETED, _file_del, ctx);
   EINA_SAFETY_ON_NULL_GOTO(ctx->fdel, err);


   ctx->dadd = ecore_event_handler_add(EIO_MONITOR_DIRECTORY_CREATED, _dir_add, ctx);
   EINA_SAFETY_ON_NULL_GOTO(ctx->dadd,err);
   ctx->ddel = ecore_event_handler_add(EIO_MONITOR_DIRECTORY_DELETED, _dir_del, ctx);
   EINA_SAFETY_ON_NULL_GOTO(ctx->ddel,err);


   ctx->selfdel = ecore_event_handler_add(EIO_MONITOR_SELF_DELETED, _mon_del, ctx);
   EINA_SAFETY_ON_NULL_GOTO(ctx->selfdel,err);
   ctx->err = ecore_event_handler_add(EIO_MONITOR_ERROR, _mon_err, ctx);
   EINA_SAFETY_ON_NULL_GOTO(ctx->err,err);

   return 1;
err:
   fm_monitor_shutdown();

   return 0;
}

void
fm_monitor_shutdown()
{
   eina_hash_free(ctx->open_monitors);

   ecore_event_handler_del(ctx->fadd);
   ecore_event_handler_del(ctx->fdel);

   ecore_event_handler_del(ctx->dadd);
   ecore_event_handler_del(ctx->ddel);

   ecore_event_handler_del(ctx->err);
   ecore_event_handler_del(ctx->selfdel);

   free(ctx);

   ctx = NULL;
   ecore_shutdown();
   eio_shutdown();
   efreet_mime_shutdown();
}